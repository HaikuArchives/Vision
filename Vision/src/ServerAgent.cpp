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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 *                 John Robinson
 */

#include <UTF8.h>
#include <Autolock.h>
#include <MessageRunner.h>

#ifdef NETSERVER_BUILD 
#  include <netdb.h>
#endif

#ifdef BONE_BUILD
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
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
#include "ListAgent.h"

class failToLock { /* exception in Establish */ };

int32 ServerAgent::ServerSeed = 0;

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
    reacquiredNick (true),
    establishHasLock (false),
    retry (0),
    retryLimit (47),
    lagCheck (0),
    lagCount (0),
    serverSocket (-1),
    send_buffer (0),
    send_size (0),
    parse_buffer (0),
    parse_size (0),
    events (vision_app->events),
    serverHostName (id_),
    initialMotd (true),
    identd (identd_),
    cmds (cmds_),
    pListAgent (NULL)
{

}

ServerAgent::~ServerAgent (void)
{
  if (send_buffer)
    delete [] send_buffer;
  if (parse_buffer)
    delete [] parse_buffer;
  if (lagRunner)
    delete lagRunner;

  if (!establishHasLock && endPointLock)
    delete endPointLock;
}

void
ServerAgent::AttachedToWindow(void)
{
  Init();
}


void
ServerAgent::Init (void)
{
#ifdef NETSERVER_BUILD
  endPointLock = new BLocker();
#endif
  Display ("Vision ");
  Display (vision_app->VisionVersion(VERSION_VERSION).String(), C_MYNICK);
  Display (" built on ");
  Display (vision_app->VisionVersion(VERSION_DATE).String());
  Display ("\nThis agent goes by the name of Smith... err ");
  BString temp;
  temp << id << " (sid: " << sid << ")";
  Display (temp.String(), C_NICK);
  Display ("\nHave fun!\n");

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
ServerAgent::NewTimer (const char *, int32, int32)
{
// TODO: implement this once scripting is ready
  return B_OK;
}

int32
ServerAgent::Timer (void *arg)
{
  BMessage *msg (reinterpret_cast<BMessage *>(arg));
  ServerAgent *agent;
  const char *cmd;
  int32 sleep,
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
  BMessage getMsg;
#ifdef NETSERVER_BUILD
  BLocker *endpointLock (NULL);
#endif
  BLocker *establishLock (NULL);
  BString remoteIP;
  int32 serverSid;
  int32 serverSock (-1);
  bool identRegistered (false);
  if (sMsgrE->IsValid())
    sMsgrE->SendMessage (M_GET_ESTABLISH_DATA, &getMsg);
  else
  {
    printf (":ERROR: sMsgr not valid in Establish() -- bailing\n");
    delete sMsgrE;
    return B_ERROR;
  }

  BMessage statMsg (M_DISPLAY);
  BString statString;
  
  try {
    BMessage reply;
    BString connectId,
            connectPort,
            ident,
            name,
            connectNick;
    getMsg.FindString ("id", &connectId);
    getMsg.FindString ("port", &connectPort);
    getMsg.FindString ("ident", &ident);
    getMsg.FindString ("name", &name);
    getMsg.FindString ("nick", &connectNick);
#ifdef NETSERVER_BUILD
    getMsg.FindPointer ("lock", reinterpret_cast<void **>(&endpointLock));
#endif
    getMsg.FindPointer ("establish lock", reinterpret_cast<void **>(&establishLock));
    serverSid = getMsg.FindInt32 ("sid");
    bool useIdent (getMsg.FindBool ("identd"));
    
    BAutolock autoLock (establishLock);
    if (!autoLock.IsLocked())
       throw failToLock();
      
    if (sMsgrE->SendMessage (M_GET_RECONNECT_STATUS, &reply) == B_OK)
    {
      int retrycount (reply.FindInt32 ("retries"));
      
      if (retrycount)
        snooze (1000000); // wait 1 second
    
      if (sMsgrE->SendMessage (M_INC_RECONNECT) != B_OK)
        throw failToLock();
      statString = "[@] Attempting to ";
      if (retrycount != 0)
        statString += "re";
      statString += "connect (attempt ";
      statString << retrycount + 1;
      statString += " of ";
      statString << reply.FindInt32 ("max_retries");
      statString += ")\n";
      ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
      sMsgrE->SendMessage (&statMsg);
      
      BMessage data (M_DISPLAY_ALL);
      data.AddString ("data", statString.String());
      data.AddInt32 ("fore", C_ERROR);
      sMsgrE->SendMessage (&data);

    }
    else
      throw failToLock();
 
    statString = "[@] Attempting a connection to ";
    statString << connectId;
    statString += ":";
    statString << connectPort;
    statString += "...\n";
    ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
    sMsgrE->SendMessage (&statMsg);

    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
#ifdef BONE_BUILD
    if (inet_aton (connectId.String(), &remoteAddr.sin_addr) == 0)
#elif NETSERVER_BUILD
    if ((remoteAddr.sin_addr.s_addr = inet_addr (connectId.String())) == 0)
#endif
    {
       struct hostent *remoteInet (gethostbyname (connectId.String()));
       if (remoteInet)
         remoteAddr.sin_addr = *((in_addr *)remoteInet->h_addr_list[0]);
       else 
       {
         ClientAgent::PackDisplay (&statMsg, "[@] Could not create connection to address and port. Make sure your Internet connection is operational.\n", C_ERROR);
         sMsgrE->SendMessage (&statMsg);
         sMsgrE->SendMessage (M_NOT_CONNECTING);
         throw failToLock();
       }
    }

    remoteAddr.sin_port = htons(atoi (connectPort.String()));
    remoteIP = inet_ntoa (remoteAddr.sin_addr);
    

    if (useIdent)
    {
      identRegistered = true;
      vision_app->AddIdent (remoteIP.String(), ident.String());      
    }

    if ((serverSock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
         ClientAgent::PackDisplay (&statMsg, "[@] Could not create connection to address and port. Make sure your Internet connection is operational.\n", C_ERROR);
         sMsgrE->SendMessage (&statMsg);
         sMsgrE->SendMessage (M_NOT_CONNECTING);
         throw failToLock();
    }
    
    // just see if he's still hanging around before
    // we got blocked for a minute
    ClientAgent::PackDisplay (&statMsg, "[@] Connection open, waiting for reply from server\n", C_ERROR);
    sMsgrE->SendMessage (&statMsg);
    sMsgrE->SendMessage (M_LAG_CHANGED);

    if (connect (serverSock, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) >= 0)
    {
      BString ip ("");  
      struct sockaddr_in sockin;

      // store local ip address for future use (dcc, etc)
      int addrlength (sizeof (struct sockaddr_in));
      if (getsockname (serverSock,(struct sockaddr *)&sockin,&addrlength)) {
        ClientAgent::PackDisplay (&statMsg, "[@] Error getting Local IP\n", C_ERROR);
        sMsgrE->SendMessage (&statMsg);
        BMessage setIP (M_SET_IP);
        setIP.AddString("ip", "127.0.0.1");
        setIP.AddBool("private", PrivateIPCheck("127.0.0.1"));
        sMsgrE->SendMessage(&setIP);
      }
      else
      {
        BMessage setIP (M_SET_IP);
        ip = inet_ntoa (sockin.sin_addr);
        setIP.AddBool("private", PrivateIPCheck (ip.String()));
        setIP.AddString("ip", ip.String());
        sMsgrE->SendMessage(&setIP);
        statString = "[@] Local IP: ";
        statString += ip.String();
        statString += "\n";
        ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
        sMsgrE->SendMessage (&statMsg);
      }

      if (PrivateIPCheck (ip.String()))
      {
        ClientAgent::PackDisplay (&statMsg, "[@] (It looks like you are behind an Internet gateway. Vision will query the IRC server upon successful connection for your gateway's Internet address. This will be used for DCC communication.)\n", C_ERROR);
        sMsgrE->SendMessage (&statMsg);  
      }
      
            ClientAgent::PackDisplay (&statMsg, "[@] Handshaking\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);

      BString string;
      string = "USER ";
      string.Append (ident);
      string.Append (" localhost ");
      string.Append (connectId);
      string.Append (" :");
      string.Append (name);

      BMessage endpointMsg (M_SET_ENDPOINT);
      endpointMsg.AddInt32 ("socket", serverSock);
      if (sMsgrE->SendMessage (&endpointMsg, &reply) != B_OK)
        throw failToLock();
      BMessage dataSend (M_SERVER_SEND);
      dataSend.AddString ("data", string.String());
      if (sMsgrE->SendMessage (&dataSend) != B_OK)
        throw failToLock();
      string = "NICK ";
      string.Append (connectNick);
      dataSend.ReplaceString ("data", string.String());
      if (sMsgrE->SendMessage (&dataSend) != B_OK)
        throw failToLock();

      // resume normal business matters.

      ClientAgent::PackDisplay (&statMsg, "[@] Established\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);
    }
    else // No endpoint->connect
    {
      ClientAgent::PackDisplay (&statMsg, "[@] Could not establish a connection to the server. Sorry.\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);
      sMsgrE->SendMessage (M_SERVER_DISCONNECT);
      throw failToLock();
    }
  } catch (failToLock)
  {
#ifdef BONE_BUILD
    close (serverSock);
#elif NETSERVER_BUILD
    closesocket (serverSock);
    if (endpointLock)
      delete endpointLock;
#endif
    delete sMsgrE;
    if (identRegistered)
      vision_app->RemoveIdent (remoteIP.String());
    return B_ERROR;
  }  
  // Don't need this anymore
  struct fd_set eset, rset, wset;
#ifdef NETSERVER_BUILD
  struct timeval tv = {0, 0};
#endif
  FD_ZERO (&eset);
  FD_ZERO (&rset);
  FD_ZERO (&wset);

  BString buffer;
  
  FD_SET (serverSock, &eset);
  FD_SET (serverSock, &rset);
  FD_SET (serverSock, &wset);
  BLooper *looper;
  
  while (sMsgrE->Target (&looper) != NULL)
  {
    char indata[1024];
    int32 length (0);
    
    FD_SET (serverSock, &eset);
    FD_SET (serverSock, &rset);
    FD_SET (serverSock, &wset);
    memset (indata, 0, 1024);
#ifdef BONE_BUILD
    if (select (serverSock + 1, &rset, 0, &eset, NULL) > 0
#elif NETSERVER_BUILD
    if (select (serverSock + 1, &rset, 0, &eset, &tv) > 0
#endif
    &&  FD_ISSET (serverSock, &rset))
    {
#ifdef NETSERVER_BUILD
      endpointLock->Lock();
#endif
      if ((length = recv (serverSock, indata, 1024, 0)) > 0)
      {
#ifdef NETSERVER_BUILD
        endpointLock->Unlock();
#endif
        BString temp;
        int32 index;

        temp.SetTo (indata, strlen(indata));
        buffer += temp;

        while ((index = buffer.FindFirst ('\n')) != B_ERROR)
        {
          temp.SetTo (buffer, index);
          buffer.Remove (0, index + 1);
    
          temp.RemoveLast ("\r");

          if (vision_app->debugrecv)
          {
            printf ("RECEIVED: (%ld:%03ld) \"", serverSid, temp.Length());
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
#ifdef NETSERVER_BUILD
     else endpointLock->Unlock();
#endif
     if (FD_ISSET (serverSock, &eset)
     || (FD_ISSET (serverSock, &rset) && length == 0)
     || !FD_ISSET (serverSock, &wset)
     || length < 0)
     {
        // we got disconnected :(
        
        if (vision_app->debugrecv)
        {
          // print interesting info          
          printf ("Negative from endpoint receive! (%ld)\n", length);
          printf ("eset : %s\nrset: %s\nwset: %s\n",
            FD_ISSET (serverSock, &eset) ? "true" : "false",
            FD_ISSET (serverSock, &rset) ? "true" : "false",
            FD_ISSET (serverSock, &wset) ? "true" : "false");
		}
		
        // tell the user all about it
        sMsgrE->SendMessage (M_SERVER_DISCONNECT);
        break;
      }
    }
    
  // take a nap, so the ServerAgent can do things
#ifdef NETSERVER_BUILD
    snooze(20000);
#endif
  }
  vision_app->RemoveIdent (remoteIP.String());
// if the serverAgent has been destroyed, let Establish destroy the endpoint, otherwise
// HandleReconnect() will.
  if (!sMsgrE->IsValid())
  {
#ifdef BONE_BUILD
    close (serverSock);
#elif NETSERVER_BUILD
    closesocket (serverSock);
#endif
  }
#ifdef NETSERVER_BUILD
  delete endpointLock;
#endif
  delete sMsgrE;
  return B_OK;
}

void
ServerAgent::SendData (const char *cData)
{
  int32 length;
  if (!cData)
    return;
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
#ifdef NETSERVER_BUILD
  endPointLock->Lock();
#endif
  if (serverSocket > 0 && (length = send (serverSocket, send_buffer, strlen (send_buffer), 0) < 0)
  || serverSocket < 0)
  {
    // doh, we aren't even connected.
    if (!reconnecting && !isConnecting)
      msgr.SendMessage (M_SERVER_DISCONNECT);
  }
#ifdef NETSERVER_BUILD
  endPointLock->Unlock();
#endif
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
  
  if (vision_app->numBench)
  {
    vision_app->bench1 = system_time();
    if (ParseEvents (parse_buffer))
    {
      vision_app->bench2 = system_time();
      BString bencht (GetWord (data.String(), 2));
      vision_app->BenchOut (bencht.String());
      return;
    }
  }
  else
  {
    if (ParseEvents (parse_buffer))
      return;
  }
  

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
  if (pListAgent)
    vision_app->pClientWin()->DispatchMessage(msg, (BView *)pListAgent);
}

void
ServerAgent::RepliedBroadcast (BMessage *)
{
//  TODO: implement this
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
  const uint32 fore,
  const uint32 back,
  const uint32 font)
{
  for (int32 i = 0; i < clients.CountItems(); ++i)
  {
    ClientAgent *client ((ClientAgent *)clients.ItemAt (i));

    BMessage msg (M_DISPLAY);
    PackDisplay (&msg, buffer, fore, back, font);
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
   * Function purpose: Setup the environment and attempt a new connection
   * to the server 
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
    establishHasLock = false;
#ifdef NETSERVER_BUILD
    endPointLock = new BLocker();
#endif
    loginThread = spawn_thread (
      Establish,
      vision_app->GetThreadName(THREAD_S),
      B_NORMAL_PRIORITY,
      new BMessenger(this));
    
    close (serverSocket);  
    serverSocket = -1;

    resume_thread (loginThread);
  }
  else
  {
    // we've hit our retry limit. throw in the towel
    reconnecting = false;
    retry = 0;
    const char *soSorry;
    soSorry = "[@] Retry limit reached; giving up. Type /reconnect if you want to give it another go.\n";
    Display (soSorry, C_ERROR);
    DisplayAll (soSorry, C_ERROR, C_BACKGROUND, F_SERVER);    
  }
}


bool
ServerAgent::PrivateIPCheck (const char *ip)
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

   if (ip == NULL || ip == "")
   {
     // it is obviously a mistake we got called.
     // setup some sane values and print an assertion
     printf (":ERROR: PrivateIPCheck() called when there is no valid data to check!\n");
     return true;
   }
   
   if (ip == "127.0.0.1")
     return true;
   
   // catch 10.0.0.0 - 10.255.255.255 and 192.168.0.0 - 192.168.255.255
   if (  (strncmp (ip, "10.", 3) == 0)
      || (strncmp (ip, "192.168.", 8) == 0))
     return true;
   
   // catch 172.16.0.0 - 172.31.255.255
   if (strncmp (ip, "172.", 4) == 0)
   {
     // check to see if characters 5-6 are (or are between) 16 and 31
     {
       char temp172s[3];
       temp172s[0] = ip[4];
       temp172s[1] = ip[5];
       temp172s[2] = '\0';
       int temp172n (atoi (temp172s));
     
       if (temp172n >= 16 || temp172n <= 31)
         return true;
     }
     return false;
   }

  // if we got this far, its a public IP address
  return false;
   
}


void
ServerAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_PARSE_LINE:
      {
        const char *buffer (NULL);
        msg->FindString ("line", &buffer);
        ParseLine (buffer);
      }
      break;
    
    case M_STATE_CHANGE:
      {
        Broadcast(msg);
        ClientAgent::MessageReceived (msg);
      }
      break;
      
    case M_SEND_RAW:
      {
        const char *buffer;
        msg->FindString ("line", &buffer);
        SendData (buffer);
      }
      break;

    case M_DISPLAY_ALL:
      {
        BString data;
        msg->FindString  ("data", &data);
        DisplayAll (data.String(), msg->FindInt32 ("fore"), msg->FindInt32 ("back"), msg->FindInt32 ("font"));
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
#ifdef NETSERVER_BUILD
        reply.AddPointer ("lock", endPointLock);
#endif
		reply.AddPointer ("establish lock", &loginLock);
        reply.AddInt32   ("sid", sid);
        msg->SendReply (&reply);
        establishHasLock = true;
      }
      break;
    
    case M_SET_ENDPOINT:
      msg->FindInt32 ("socket", &serverSocket);
      break;
      
    case M_NOT_CONNECTING:
      isConnecting = false;
      break;
    
    case M_INC_RECONNECT:
      ++retry;
      break;
     
    case M_INIT_LAG:
      myLag = "0.000";
      if (!IsHidden())
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String(), false);
      break;
    
    case M_SET_IP:
      {
        static BString ip;
        msg->FindString("ip", &ip);
        localip = ip.String();
        localip_private = msg->FindBool("private");
      }
      break;
    
    case M_GET_IP:
      {
        BMessage reply;
        reply.AddBool ("private", localip_private);
        reply.AddString ("ip", localip);
        msg->SendReply (&reply);
      }
      break;

    case M_GET_RECONNECT_STATUS:
      {
        BMessage reply (B_REPLY);
        reply.AddInt32 ("retries", retry);
        reply.AddInt32 ("max_retries", retryLimit);
        msg->SendReply(&reply); 
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
        if (msg->IsSourceWaiting())
          msg->SendReply(B_REPLY);
      }
      break;
    
    case M_CHAT_ACCEPT:
      {
        int32 acceptDeny;
        BString theNick;
        const char *theIP, *thePort;
        msg->FindInt32("which", &acceptDeny);
        if (acceptDeny)
          return;
        msg->FindString("nick", &theNick);
        msg->FindString("ip", &theIP);
        msg->FindString("port", &thePort);
        
        theNick.Append (" [DCC]");
        vision_app->pClientWin()->pWindowList()->AddAgent (
          new MessageAgent (
            *vision_app->pClientWin()->AgentRect(),
            theNick.String(),
            sid,
            serverHostName.String(),
            sMsgr,
            myNick.String(),
            "",
            true,
            false,
            theIP,
            thePort),
          sid,
          theNick.String(),
          WIN_MESSAGE_TYPE,
          true);
          
          ClientAgent *client (vision_app->pClientWin()->pWindowList()->Agent (sid, theNick.String()));
          clients.AddItem (client);
      }
      break;	

    case M_CHAT_ACTION: // dcc chat
     {
       ClientAgent *client;
       const char *theNick;
       BString thePort;
       BString theId;

       msg->FindString ("nick", &theNick);
       msg->FindString ("port", &thePort);
       theId << theNick << " [DCC]";

       if ((client = Client (theId.String())) == 0)
       {
          vision_app->pClientWin()->pWindowList()->AddAgent (
            new MessageAgent (
              *vision_app->pClientWin()->AgentRect(),
              theId.String(),
              sid,
              serverHostName.String(),
              sMsgr,
              myNick.String(),
              "",
              true,
              true,
              "",
              thePort != "" ? thePort.String() : ""),
            sid,
            theId.String(),
            WIN_MESSAGE_TYPE,
            true);
         client = vision_app->pClientWin()->pWindowList()->Agent (sid, theId.String());
         clients.AddItem (client);
       }
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
        serverSocket = -1;

        // store current nick for reconnect use (might be an away nick, etc)
        if (reacquiredNick)
        {
          reconNick = myNick;
          reacquiredNick = false;
        }
        // let the user know
        if (isConnected)
        {
          BString sAnnounce;
          sAnnounce += "[@] Disconnected from ";
          sAnnounce += serverName;
          sAnnounce += "\n";
          Display (sAnnounce.String(), C_ERROR);
          DisplayAll (sAnnounce.String(), C_ERROR, C_BACKGROUND, F_SERVER);
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

        if (isConnected && serverSocket)
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
		BMessenger listMsgr(pListAgent);
		listMsgr.SendMessage(M_CLIENT_QUIT);
		
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
    
    case M_LIST_COMMAND:
      {
        if (pListAgent)
          break;
        vision_app->pClientWin()->pWindowList()->AddAgent (
          new ListAgent (
            *vision_app->pClientWin()->AgentRect(),
            serverHostName.String(), new BMessenger(this)),
          sid,
          "Channels",
          WIN_LIST_TYPE,
          true);
        // kind of a hack since Agent() returns a pointer of type ClientAgent, of which
        // ListAgent is not a subclass...
        pListAgent = reinterpret_cast<ListAgent *>(vision_app->pClientWin()->pWindowList()->Agent(sid, "Channels"));
        BMessenger listMsgr(pListAgent);
        listMsgr.SendMessage(M_LIST_COMMAND);
      }
      break;
    
    case M_LIST_SHUTDOWN:
      pListAgent = NULL;
      break;

    default:
      ClientAgent::MessageReceived (msg);
  }
}
