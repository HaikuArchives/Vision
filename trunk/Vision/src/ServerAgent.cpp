/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License.
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <guru@startrek.com>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 *                 John Robinson
 */

#ifdef GNOME_BUILD
#  include "gnome/NetEndpoint.h"
#  include "gnome/UTF8.h"
#  include "gnome/Autolock.h"
#elif BEOS_BUILD
#  include <NetEndpoint.h>
#  include <UTF8.h>
#  include <Autolock.h>
#  include <netdb.h>
#  include <MessageRunner.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "Vision.h"
#include "ClientWindow.h"
#include "MessageAgent.h"
#include "WindowList.h"
#include "StringManip.h"
#include "StatusView.h"

class failToLock { /* exception in Establish */ };

int32 ServerAgent::ServerSeed = 0;
BLocker ServerAgent::identLock;

ServerAgent::ServerAgent (
  const char *id_,
  const char *port,
  bool identd_,
  const char *cmds_,
  BRect frame_)

  : ClientAgent (
    id_,
    ServerSeed++,
    id_,
    vision_app->GetString ("nickname1"),
    frame_),
    localip (""),
    localip_private (false),
    lnick1 (vision_app->GetString ("nickname1")),
    lnick2 (vision_app->GetString ("nickname2")),
    lport (port),
    lname (vision_app->GetString ("realname")),
    lident (vision_app->GetString ("username")),
    nickAttempt (0),
    myNick (lnick1),
    myLag ("0.000"),
    isConnected (false),
    isConnecting (true),
    reconnecting (false),
    isQuitting (false),
    checkingLag (false),
    retry (0),
    retryLimit (47),
    lagCheck (0),
    lagCount (0),
    lEndpoint (0),
    send_buffer (0),
    send_size (0),
    parse_buffer (0),
    parse_size (0),
    events (vision_app->events),
    serverHostName (id_),
    initialMotd (true),
    identd (identd_),
    cmds (cmds_)
{

}

ServerAgent::~ServerAgent (void)
{
  if (send_buffer)  delete [] send_buffer;
  if (parse_buffer) delete [] parse_buffer;
}

void
ServerAgent::AttachedToWindow(void)
{
  Init();
}


void
ServerAgent::Init (void)
{
  myFont = *(vision_app->GetClientFont (F_SERVER));
  ctcpReqColor = vision_app->GetColor (C_CTCP_REQ);
  ctcpRpyColor = vision_app->GetColor (C_CTCP_RPY);
  whoisColor   = vision_app->GetColor (C_WHOIS);
  errorColor   = vision_app->GetColor (C_ERROR);
  quitColor    = vision_app->GetColor (C_QUIT);
  joinColor    = vision_app->GetColor (C_JOIN);
  noticeColor  = vision_app->GetColor (C_NOTICE);
  textColor    = vision_app->GetColor (C_SERVER);
  wallColor    = vision_app->GetColor (C_WALLOPS);

  Display ("Vision ", 0);
  Display (vision_app->VisionVersion(VERSION_VERSION).String(), &myNickColor);
  Display (" built on ", 0);
  Display (vision_app->VisionVersion(VERSION_DATE).String(), 0);
  Display ("\nThis agent goes by the name of Smith... err ", 0);
  BString temp;
  temp << id << " (sid: " << sid << ")";
  Display (temp.String(), &nickColor);
  Display ("\nHave fun!\n", 0);

  lagRunner = new BMessageRunner (
    this,       // target ServerAgent
    new BMessage (M_LAG_CHECK),
    10000000,   // 10 seconds
    -1);        // forever
  
  loginThread = spawn_thread (
    Establish,
    vision_app->GetThreadName(THREAD_S),
    B_NORMAL_PRIORITY,
    new BMessenger(this));

  resume_thread (loginThread);

}

int
ServerAgent::IRCDType (void)
{
  return ircdtype;
}


status_t
ServerAgent::NewTimer (const char *cmd, int32 sleep, int32 loops)
{

  return B_OK;
}

int32
ServerAgent::Timer (void *arg)
{
  BMessage *msg (reinterpret_cast<BMessage *>(arg));
  ServerAgent *agent;
  const char *cmd;
  int32 pass (0),
        sleep,
        loops;

  if ((msg->FindString  ("command", &cmd) != B_OK)
  ||  (msg->FindInt32   ("loops", &loops) != B_OK)
  ||  (msg->FindInt32   ("sleep", &sleep) != B_OK)
  ||  (msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
  {  
    printf (":ERROR: couldn't find valid data in BMsg to Timer() -- bailing\n");
    return B_ERROR;
  }
  
  
  return B_OK;
  
}

int32
ServerAgent::Establish (void *arg)
{
  BMessenger *sMsgrE (reinterpret_cast<BMessenger *>(arg));
  ServerAgent *server;
  thread_id establishThread (find_thread (NULL));
  BNetEndpoint *endPoint (NULL);
  BMessage getMsg;
  if (sMsgrE->IsValid())
    sMsgrE->SendMessage (M_GET_ESTABLISH_DATA, &getMsg);
  else
  {
    printf (":ERROR: sMsgr not valid in Establish() -- bailing\n");
    return B_ERROR;
  }

  if (getMsg.FindPointer ("server", reinterpret_cast<void **>(&server)) != B_OK)
  {
    printf (":ERROR: error getting valid pointer from M_GET_ESTABLISH_DATA in Establish() -- bailing\n");
    delete sMsgrE;
    return B_ERROR;
  }

  BMessage statMsg (M_DISPLAY);
  BString statString;
  
  try {
    if (!server->LockLooper())
      throw failToLock();
    
    BString connectId (server->id),
            connectPort (server->lport),
            ident (server->lident),
            name (server->lname),
            connectNick (server->myNick);
    bool useIdent (server->identd);
    server->UnlockLooper();
    
    if (!server->LockLooper())
      throw failToLock();
      
    if (server->reconnecting)
    {
      if (server->retry > 0)
      {
        server->UnlockLooper();
        snooze (1000000); // wait 1 second
        if (!server->LockLooper())
          throw failToLock();
      }
    
      server->retry++;
      statString = "[@] Attempting to reconnect (Retry ";
      statString << server->retry;
      statString += " of ";
      statString << server->retryLimit;
      statString += ")\n";
      server->PackDisplay (&statMsg, statString.String(), &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);
      server->DisplayAll (statString.String(), &(server->errorColor), &(server->serverFont));
    }
    server->UnlockLooper();
 
    if (!server->LockLooper())
      throw failToLock();
    statString = "[@] Attempting a connection to ";
    statString << connectId;
    statString += ":";
    statString << connectPort;
    statString += "...\n";
    server->PackDisplay (&statMsg, statString.String(), &(server->errorColor));
    sMsgrE->SendMessage (&statMsg);
    server->UnlockLooper();

    BNetAddress address;
 
    if (address.SetTo (connectId.String(), atoi (connectPort.String())) != B_NO_ERROR)
    {
      server->PackDisplay (&statMsg, "[@] The address and port seem to be invalid. Make sure your Internet connection is operational.\n", &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);
      sMsgrE->SendMessage (M_SERVER_DISCONNECT);
      throw failToLock();
    }
    endPoint = new BNetEndpoint;

    if (!endPoint || endPoint->InitCheck() != B_NO_ERROR)
    {
      if (!server->LockLooper())
        throw failToLock();
      server->PackDisplay (&statMsg, "[@] Could not create connection to address and port. Make sure your Internet connection is operational.\n", &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);
      server->isConnecting = false;
      server->UnlockLooper();

      throw failToLock();
    }

    // just see if he's still hanging around before
    // we got blocked for a minute
    if (!sMsgrE->IsValid())
      throw failToLock();
    

    server->PackDisplay (&statMsg, "[@] Connection open, waiting for reply from server\n", &(server->errorColor));
    sMsgrE->SendMessage (&statMsg);

    server->myLag = "0.000";
    sMsgrE->SendMessage (M_LAG_CHANGED);

    identLock.Lock();
    if (endPoint->Connect (address) == B_NO_ERROR)
    {
      struct sockaddr_in sockin;


      // store local ip address for future use (dcc, etc)
      int addrlength (sizeof (struct sockaddr_in));
      if (getsockname (endPoint->Socket(),(struct sockaddr *)&sockin,&addrlength)) {
        server->PackDisplay (&statMsg, "[@] Error getting Local IP\n", &(server->errorColor));
        sMsgrE->SendMessage (&statMsg);
        server->localip = "127.0.0.1";
      }
      else
      {
        server->localip = inet_ntoa (sockin.sin_addr);
        statString = "[@] Local IP: ";
        statString += server->localip;
        statString += "\n";
        server->PackDisplay (&statMsg, statString.String(), &(server->errorColor));
        sMsgrE->SendMessage (&statMsg);
      }

      server->PrivateIPCheck();
      if (server->localip_private == true)
      {
        server->PackDisplay (&statMsg, "[@] (It looks like you are behind an Internet gateway. Vision will query the IRC server upon successful connection for your gateway's Internet address. This will be used for DCC communication.)\n", &(server->errorColor));
        sMsgrE->SendMessage (&statMsg);  
      }

      // resume normal business matters.

      server->PackDisplay (&statMsg, "[@] Established\n", &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);

      if (useIdent)
      {
        if (sMsgrE->IsValid() && !server->LockLooper())
        {
          identLock.Unlock();
          throw failToLock();
        }
        server->PackDisplay (&statMsg, "[@] Spawning Ident daemon (10 sec timeout)\n", &(server->errorColor));
        sMsgrE->SendMessage (&statMsg);
        server->UnlockLooper();
        BNetEndpoint identPoint, *accepted;
        BNetAddress identAddress (sockin.sin_addr, 113);
        BNetBuffer buffer;
        char received[64];

        if (sMsgrE->IsValid()
        &&  identPoint.InitCheck()             == B_OK
        &&  identPoint.Bind (identAddress)     == B_OK
        &&  identPoint.Listen()                == B_OK
        && (accepted = identPoint.Accept (10000))    != 0   // 10 sec timeout
        &&  accepted->Receive (buffer, 64)     >= 0
        &&  buffer.RemoveString (received, 64) == B_OK)
        {
          int32 len;
  
          received[63] = 0;
          while ((len = strlen (received))
          &&     isspace (received[len - 1]))
            received[len - 1] = 0;

          BNetBuffer output;
          BString string;

          string.Append (received);
          string.Append (" : USERID : BeOS : ");
          string.Append (ident);
          string.Append ("\r\n");

          output.AppendString (string.String());
          accepted->Send (output);
          accepted->Close();
          delete accepted;

          server->PackDisplay (&statMsg, "[@] Replied to Ident request\n", &(server->errorColor));
          sMsgrE->SendMessage (&statMsg);
        }

        server->PackDisplay (&statMsg, "[@] Deactivated daemon\n", &(server->errorColor));
        sMsgrE->SendMessage (&statMsg);
      }

      server->PackDisplay (&statMsg, "[@] Handshaking\n", &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);

      BString string;
      string = "USER ";
      string.Append (ident);
      string.Append (" localhost ");
      string.Append (connectId);
      string.Append (" :");
      string.Append (name);

      if (sMsgrE->IsValid() && !server->LockLooper())
      {
        identLock.Unlock();
        throw failToLock();
      }
      server->lEndpoint = endPoint;
      server->SendData (string.String());

      string = "NICK ";
      string.Append (connectNick);
      server->SendData (string.String());
      server->UnlockLooper();

      identLock.Unlock();
    }
    else // No endpoint->connect
    {
      identLock.Unlock();

      server->PackDisplay (&statMsg, "[@] Could not establish a connection to the server. Sorry.\n", &(server->errorColor));
      sMsgrE->SendMessage (&statMsg);
      sMsgrE->SendMessage (M_SERVER_DISCONNECT);
      server->lEndpoint = 0;
      throw failToLock();
    }
  } catch (failToLock)
  {
    if (endPoint)
    {
      endPoint->Close();
      delete endPoint;
    }  
    delete sMsgrE;
    return B_ERROR;
  }  
  // Don't need this anymore
  struct fd_set eset, rset, wset;
  struct timeval tv = {0, 0};

  FD_ZERO (&eset);
  FD_ZERO (&rset);
  FD_ZERO (&wset);

  BString buffer;
 
  while (sMsgrE->IsValid() 
  && (establishThread == server->loginThread))
  {
    BNetBuffer inbuffer (1024);
    int32 length (0);

    FD_SET (endPoint->Socket(), &eset);
    FD_SET (endPoint->Socket(), &rset);
    FD_SET (endPoint->Socket(), &wset);
    if (select (endPoint->Socket() + 1, &rset, 0, &eset, &tv) > 0
    &&  FD_ISSET (endPoint->Socket(), &rset))
    {
      server->endPointLock.Lock();
      if ((length = endPoint->Receive (inbuffer, 1024)) > 0)
      {
        server->endPointLock.Unlock();
        BString temp;
        int32 index;

        temp.SetTo ((char *)inbuffer.Data(), inbuffer.Size());
        buffer += temp;

        while ((index = buffer.FindFirst ('\n')) != B_ERROR)
        {
          temp.SetTo (buffer, index);
          buffer.Remove (0, index + 1);
    
          temp.RemoveLast ("\r");

          if (vision_app->debugrecv)
          {
            printf ("RECEIVED: (%ld:%03ld) \"", server->sid, temp.Length());
            for (int32 i = 0; i < temp.Length(); ++i)
            {
              if (isprint (temp[i]))
                printf ("%c", temp[i]);
              else
                printf ("[0x%02x]", temp[i]);
            }
            printf ("\"\n");
          }


          // We ship it off this way because
          // we want this thread to loop relatively
          // quickly.  Let ServerWindow's main thread
          // handle the processing of incoming data!
         BMessage msg (M_PARSE_LINE);
         msg.AddString ("line", temp.String());
         sMsgrE->SendMessage (&msg);
       }
     }
     
     server->endPointLock.Unlock();
     
     if (FD_ISSET (endPoint->Socket(), &eset)
     || (FD_ISSET (endPoint->Socket(), &rset) && length == 0)
     || !FD_ISSET (endPoint->Socket(), &wset)
     || length < 0)
     {
        // we got disconnected :(
        
        if (vision_app->debugrecv)
        {
          // print interesting info          
          printf ("Negative from endpoint receive! (%ld)\n", length);
          printf ("(%d) %s\n", endPoint->Error(), endPoint->ErrorStr());

          printf ("eset : %s\nrset: %s\nwset: %s\n",
            FD_ISSET (endPoint->Socket(), &eset) ? "true" : "false",
            FD_ISSET (endPoint->Socket(), &rset) ? "true" : "false",
            FD_ISSET (endPoint->Socket(), &wset) ? "true" : "false");
		}
		
        // tell the user all about it
        sMsgrE->SendMessage (M_SERVER_DISCONNECT);
        break;
      }
    }
    
    // take a nap, so the ServerAgent can do things
    snooze (20000);
  }
  
  server->lEndpoint = 0;
  endPoint->Close();
  delete endPoint;

  delete sMsgrE;
  return B_OK;
}

void
ServerAgent::SendData (const char *cData)
{
  int32 length;
  BString data (cData);

  data.Append("\r\n");
  length = data.Length() + 1;

  // The most it could be is that every
  // stinking character is a utf8 character.
  // Which can be at most 3 bytes.  Hence
  // our multiplier of 3

  if (send_size < length * 3UL)
  {
    if (send_buffer)
      delete [] send_buffer;
    send_buffer = new char [length * 3];
    send_size = length * 3;
  }

  int32 dest_length (send_size), state (0);

  convert_from_utf8 (
    B_ISO1_CONVERSION,
    data.String(), 
    &length,
    send_buffer,
    &dest_length,
    &state);

  endPointLock.Lock();
  if ((lEndpoint != 0 && (length = lEndpoint->Send (send_buffer, strlen (send_buffer))) < 0)
  || lEndpoint == 0)
  {
    // doh, we aren't even connected.

    if (!reconnecting && !isConnecting)
      msgr.SendMessage (M_SERVER_DISCONNECT);
  }
  
  endPointLock.Unlock();

  if (vision_app->debugsend)
  {
    data.RemoveAll ("\n");
    data.RemoveAll ("\r");
    printf("    SENT: (%ld:%03ld) \"%s\"\n", sid, length, data.String());
  }
}

void
ServerAgent::ParseLine (const char *cData)
{
  BString data (FilterCrap (cData));

  int32 length (data.Length() + 1);

  if (parse_size < length * 3UL)
  {
    if (parse_buffer)
      delete [] parse_buffer;
    parse_buffer = new char [length * 3];
    parse_size = length * 3;
  }

  int32 dest_length (parse_size), state (0);

  convert_to_utf8 (
    B_ISO1_CONVERSION,
    data.String(), 
    &length,
    parse_buffer,
    &dest_length,
    &state);

  if (ParseEvents (parse_buffer))
    return;

  data.Append("\n");
  Display (data.String(), 0);
}

ClientAgent *
ServerAgent::Client (const char *cName)
{
  ClientAgent *client (0);

  for (int32 i = 0; i < clients.CountItems(); ++i)
  {
    ClientAgent *item ((ClientAgent *)clients.ItemAt (i));

    if (strcasecmp (cName, item->Id().String()) == 0)
    {
      client = item;
      break;
    }
  }

  return client;
}

ClientAgent *
ServerAgent::ActiveClient (void)
{
  ClientAgent *client (0);

  for (int32 i = 0; i < clients.CountItems(); ++i)
    if (!((ClientAgent *)clients.ItemAt (i))->IsHidden())
      client = (ClientAgent *)clients.ItemAt (i);

  return client;
}


void
ServerAgent::Broadcast (BMessage *msg)
{
  for (int32 i = 0; i < clients.CountItems(); ++i)
  {
    ClientAgent *client ((ClientAgent *)clients.ItemAt (i));

    if (client != this)
      client->msgr.SendMessage (msg);
  }
}

void
ServerAgent::RepliedBroadcast (BMessage *msg)
{
//  BMessage cMsg (*msg);
//  BAutolock lock (this);
//
//  for (int32 i = 0; i < clients.CountItems(); ++i)
//  {
//    ClientAgent *client ((ClientAgent *)clients.ItemAt (i));
//
//    if (client != this)
//    {
//      BMessenger msgr (client);
//      BMessage reply;
//      msgr.SendMessage (&cMsg, &reply);
//    }
//  }
}


void
ServerAgent::DisplayAll (
  const char *buffer,
  const rgb_color *color,
  const BFont *font)
{
  for (int32 i = 0; i < clients.CountItems(); ++i)
  {
    ClientAgent *client ((ClientAgent *)clients.ItemAt (i));

    BMessage msg (M_DISPLAY);
    PackDisplay (&msg, buffer, color, font);
    client->msgr.SendMessage (&msg);
  }

  return;
}

void
ServerAgent::PostActive (BMessage *msg)
{
  BAutolock lock (Window());
  ClientAgent *client (ActiveClient());

  if (client)
    client->msgr.SendMessage (msg);
  else
    msgr.SendMessage (msg);
}

BString
ServerAgent::FilterCrap (const char *data)
{
  BString outData ("");
  int32 theChars (strlen (data));
  bool ViewCodes (false);

  for (int32 i = 0; i < theChars; ++i)
  {
    if (data[i] > 1 && data[i] < 32)
    {
      // TODO Get these codes working
      if (data[i] == 3)
      {
        if (ViewCodes)
          outData << "[0x03]{";

        ++i;
        while (i < theChars
        &&   ((data[i] >= '0'
        &&     data[i] <= '9')
        ||     data[i] == ','))
        {
          if (ViewCodes)
          outData << data[i];

          ++i;
        }
        
        --i;
        
        if (ViewCodes)
          outData << "}";
      }
      else if (ViewCodes)
      {
        char buffer[16];
        sprintf (buffer, "[0x%02x]", data[i]);
        outData << buffer;
      }
    }
    else
      outData << data[i];
  }

  return outData;
}

void
ServerAgent::HandleReconnect (void)
{
  /*
   * Function purpose: Setup the environment and attempt a new connection to the server 
   */
   
  if (isConnected)
  {
    // what's going on here?!
    printf (":ERROR: HandleReconnect() called when we're already connected! Whats up with that?!");
    return;
  }

  if (retry < retryLimit)
  {
    // we are go for main engine start
    reconnecting = true;
    isConnecting = true;
    nickAttempt = 0;

    loginThread = spawn_thread (
      Establish,
      vision_app->GetThreadName(THREAD_S),
      B_NORMAL_PRIORITY,
      new BMessenger(this));
	
    resume_thread (loginThread);
  }
  else
  {
    // we've hit our retry limit. throw in the towel
    reconnecting = false;
    retry = 0;
    const char *soSorry;
    soSorry = "[@] Retry limit reached; giving up. Type /reconnect if you want to give it another go.\n";
    Display (soSorry, &errorColor);
    DisplayAll (soSorry, &errorColor, &serverFont);    
  }
}


void
ServerAgent::PrivateIPCheck (void)
{
  /*
   * Function purpose: Compare against localip to see if it is a private address
   *                   if so, set localip_private to true;
   *
   * Private ranges: 10.0.0.0    - 10.255.255.255
   *                 172.16.0.0  - 172.31.255.255
   *                 192.168.0.0 - 192.168.255.255
   *                 (as defined in RFC 1918)
   */

   if (localip == NULL || localip == "")
   {
     // it is obviously a mistake we got called.
     // setup some sane values and print an assertion
     localip = "127.0.0.1";
     localip_private = true;
     printf (":ERROR: PrivateIPCheck() called when there is no valid data to check!\n");
     return;
   }
   
   if (localip == "127.0.0.1")
   {
     localip_private = true;
     return;
   }
   
   // catch 10.0.0.0 - 10.255.255.255 and 192.168.0.0 - 192.168.255.255
   if (  (strncmp (localip, "10.", 3) == 0)
      || (strncmp (localip, "192.168.", 8) == 0))
   {
     localip_private = true;
     return;
   }
   
   // catch 172.16.0.0 - 172.31.255.255
   if (strncmp (localip, "172.", 4) == 0)
   {
     // check to see if characters 5-6 are (or are between) 16 and 31
     {
       char temp172s[3];
       temp172s[0] = localip[4];
       temp172s[1] = localip[5];
       temp172s[2] = '\0';
       int temp172n (atoi (temp172s));
     
       if (temp172n >= 16 || temp172n <= 31)
         localip_private = true;
     }
     return;
   }

  // if we got this far, its a public IP address
  localip_private = false;
   
}


void
ServerAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_PARSE_LINE:
      {
        const char *buffer;
        msg->FindString ("line", &buffer);
        ParseLine (buffer);
      }
      break;

    case M_GET_ESTABLISH_DATA:
      {
        BMessage reply (B_REPLY);
        reply.AddString  ("id", id.String());
        reply.AddString  ("port", lport.String());
        reply.AddString  ("ident", lident.String());
        reply.AddString  ("name", lname.String());
        reply.AddString  ("nick", myNick.String());
        reply.AddBool    ("identd", identd);
        reply.AddPointer ("server", this);
        msg->SendReply (&reply);
      }
      break;

    case M_SERVER_SEND:
      {
        BString buffer;
        int32 i;

        for (i = 0; msg->HasString ("data", i); ++i)
        {
          const char *str;

          msg->FindString ("data", i, &str);
          buffer << str;
        }

        SendData (buffer.String());
      }
      break;

    case M_SLASH_RECONNECT:
      if (!isConnected && !isConnecting)
        msgr.SendMessage (M_SERVER_DISCONNECT);
      break;

    case M_SERVER_DISCONNECT:
      {
        myLag = "CONNECTION PROBLEM";
        msgr.SendMessage (M_LAG_CHANGED);
        checkingLag = false;
        
        // let the user know
        if (isConnected)
        {
          BString sAnnounce;
          sAnnounce += "[@] Disconnected from ";
          sAnnounce += serverName;
          sAnnounce += "\n";
          Display (sAnnounce.String(), &errorColor);
          DisplayAll (sAnnounce.String(), &errorColor, &serverFont);
        }
			
        isConnected = false;
        isConnecting = false;		      
      
        // attempt a reconnect
        HandleReconnect();
      }
      break;
      
    case M_STATUS_ADDITEMS:
      {
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            0, ""),
          true);

        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            "Lag: ",
            "",
            STATUS_ALIGN_LEFT),
          true);

        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            0,
            "",
            STATUS_ALIGN_LEFT),
          true);

        // The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
        // We send true on the last SetItemValue().
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, serverHostName.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, myNick.String(), true);
      }
      break;

    case M_LAG_CHECK:
      {
        if (isConnected)
	    {
	      if (!checkingLag)
          {
            lagCheck = system_time();
            lagCount = 1;
            checkingLag = true;
            BMessage lagSend (M_SERVER_SEND);
            AddSend (&lagSend, "VISION_LAG_CHECK");
            AddSend (&lagSend, endl);
          }
          else
          {
            if (lagCount > 4)
            {
              // we've waited 50 seconds
              // connection problems?
              myLag = "CONNECTION PROBLEM";
              msgr.SendMessage (M_LAG_CHANGED);
            }
            else
            {
              // wait some more
              char lag[15] = "";
              sprintf (lag, "%ld0.000+", lagCount);  // assuming a 10 second runner
              myLag = lag;
              ++lagCount;
              msgr.SendMessage (M_LAG_CHANGED);
            }
          }	
        }
      }
      break;
      
    case M_LAG_CHANGED:
      {
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String(), true);
          
        BMessage newmsg (M_LAG_CHANGED);
        newmsg.AddString ("lag", myLag);
        Broadcast (&newmsg);        
      }
      break;

    case M_REJOIN_ALL:
      {
        for (int32 i = 0; i < clients.CountItems(); ++i)
        {
          ClientAgent *client ((ClientAgent *)clients.ItemAt (i));
          
          if (dynamic_cast<ChannelAgent *>(client))
          {
            BMessage rejoinMsg (M_REJOIN);
            rejoinMsg.AddString ("nickname", myNick.String());
            client->msgr.SendMessage (&rejoinMsg);
          }        
        }    
      }
      break;

    case M_OPEN_MSGAGENT:
      {
        ClientAgent *client;
        const char *theNick;

        msg->FindString ("nick", &theNick);

        if (!(client = Client (theNick)))
        {
          vision_app->pClientWin()->pWindowList()->AddAgent (
            new MessageAgent (
              *vision_app->pClientWin()->AgentRect(),
              theNick,
              sid,
              serverHostName.String(),
              sMsgr,
              myNick.String(),
              ""),
            sid,
            theNick,
            WIN_MESSAGE_TYPE,
            true);

          client = (vision_app->pClientWin()->pWindowList()->Agent (sid, theNick));
          clients.AddItem (client);
        }
        else
          client->agentWinItem->ActivateItem();
        
        if (msg->HasMessage ("msg"))
        {
          BMessage buffer;

          msg->FindMessage ("msg", &buffer);
          client->msgr.SendMessage (&buffer);
        }
      }
      break;

    case M_CLIENT_QUIT:
      {
        ClientAgent::MessageReceived(msg);
        bool shutingdown (false);

        if (msg->HasBool ("vision:shutdown"))
          msg->FindBool ("vision:shutdown", &shutingdown);

        if (msg->HasString ("vision:quit"))
        {
          const char *quitstr;
          msg->FindString ("vision:quit", &quitstr);
          quitMsg = quitstr;
        }

        isQuitting = true;
 
        if (isConnected && lEndpoint)
        {
          if (quitMsg.Length() == 0)
          {
            const char *expansions[1];
            BString version (vision_app->VisionVersion(VERSION_VERSION));
            expansions[0] = version.String();
            quitMsg << "QUIT :" << ExpandKeyed (vision_app->GetCommand (CMD_QUIT).String(), "V", expansions);
          }
 
          SendData (quitMsg.String());
        }

        Broadcast (new BMessage (M_CLIENT_QUIT));

        BMessage deathchant (M_OBITUARY);
        deathchant.AddPointer ("agent", this);
        deathchant.AddPointer ("item", agentWinItem);
        vision_app->pClientWin()->PostMessage (&deathchant);
      }
      break;

    case M_CLIENT_SHUTDOWN:
      {
        ClientAgent *deadagent;

        if (msg->FindPointer ("agent", reinterpret_cast<void **>(&deadagent)) != B_OK)
        {
          printf (":ERROR: error getting valid pointer from M_CLIENT_SHUTDOWN -- bailing\n");
          break;
        }
    
        clients.RemoveItem (deadagent);

        if (isQuitting && clients.CountItems() <= 1)
          sMsgr.SendMessage (M_CLIENT_QUIT);
      }
      break;

    default:
      ClientAgent::MessageReceived (msg);
  }
}
