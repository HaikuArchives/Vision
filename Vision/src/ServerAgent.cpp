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
#include <Directory.h>
#include <Entry.h>
#include <FilePanel.h>
#include <MessageRunner.h>
#include <Path.h>
#include <String.h>

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

#include "ClientAgent.h"
#include "ClientAgentLogger.h"
#include "ClientWindow.h"
#include "ChannelAgent.h"
#include "DCCConnect.h"
#include "ListAgent.h"
#include "MessageAgent.h"
#include "ServerAgent.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Vision.h"
#include "WindowList.h"

class failToLock { /* exception in Establish */ };

int32 ServerAgent::fServerSeed = 0;

ServerAgent::ServerAgent (
  const char *id_,
  BMessage &net,
  BRect frame_)

  : ClientAgent (
    id_,
    fServerSeed++,
    id_,
    net.FindString ("nick"),
    frame_),
    fLocalip (""),
    fMyNick (net.FindString ("nick")),
    fMyLag ((net.FindBool ("lagCheck")) ? "0.000" : S_SERVER_LAG_DISABLED),
    fLname (net.FindString ("realname")),
    fLident (net.FindString ("ident")),
    fLocalip_private (false),
    fGetLocalIP (false),
    fIsConnected (false),
    fIsConnecting (true),
    fReconnecting (false),
    fIsQuitting (false),
    fCheckingLag (false),
    fReacquiredNick (true),
    fEstablishHasLock (false),
    fRetry (0),
    fRetryLimit (47),
    fLagCheck (0),
    fLagCount (0),
    fNickAttempt (0),
    fServerSocket (-1),
    fParse_buffer (NULL),
    fParse_size (0),
    fEvents (vision_app->fEvents),
    fServerHostName (id_),
    fInitialMotd (true),
    fCmds (net.FindString ("autoexec")),
    fListAgent (NULL),
    fNetworkData (net),
    fServerIndex (-1),
    fNickIndex (1),
    fLogger (NULL)
{
  fLogger = new ClientAgentLogger(fId);
}

ServerAgent::~ServerAgent (void)
{
//  if (fSend_buffer)
//    delete [] fSend_buffer;
  if (fParse_buffer)
    delete [] fParse_buffer;
  if (fLagRunner)
    delete fLagRunner;

  if (!fEstablishHasLock && fEndPointLock)
    delete fEndPointLock;
    
  delete fLogger;
}

void
ServerAgent::AttachedToWindow(void)
{
  Init();
}

void
ServerAgent::AllAttached (void)
{
  fSMsgr = BMessenger (this);
  ClientAgent::AllAttached ();
}

// do nothing for now
void
ServerAgent::AddMenuItems(BPopUpMenu *)
{
}


void
ServerAgent::Init (void)
{
#ifdef NETSERVER_BUILD
  fEndPointLock = new BLocker();
#endif
  Display ("Vision ");
  Display (vision_app->VisionVersion(VERSION_VERSION).String(), C_MYNICK);
  Display (" built on ");
  Display (vision_app->VisionVersion(VERSION_DATE).String());
  Display ("\nThis agent goes by the name of Smith... err ");
  BString temp;
  temp << fId << " (sid: " << fSid << ")";
  Display (temp.String(), C_NICK);
  Display ("\nHave fun!\n");
  
  fLagRunner = new BMessageRunner (
    this,       // target ServerAgent
    new BMessage (M_LAG_CHECK),
    10000000,   // 10 seconds
    -1);        // forever
  
  BString name;

  vision_app->GetThreadName(THREAD_S, name);
  
  fLoginThread = spawn_thread (
    Establish,
    name.String(),
    B_NORMAL_PRIORITY,
    new BMessenger(this));

  resume_thread (fLoginThread);
}

int
ServerAgent::IRCDType (void)
{
  return fIrcdtype;
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
  int32 sleeptimer,
        loops;

  if ((msg->FindString  ("command", &cmd) != B_OK)
  ||  (msg->FindInt32   ("loops", &loops) != B_OK)
  ||  (msg->FindInt32   ("sleep", &sleeptimer) != B_OK)
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
  BString remoteIP;
  int32 serverSid;
  int32 serverSock (-1);
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
    const ServerData *serverData (NULL);
    int32 size;
    getMsg.FindData ("server", B_ANY_TYPE, reinterpret_cast<const void **>(&serverData), &size);
    connectId = serverData->serverName;
    connectPort << serverData->port;
    getMsg.FindString ("ident", &ident);
    getMsg.FindString ("name", &name);
    getMsg.FindString ("nick", &connectNick);
#ifdef NETSERVER_BUILD
    getMsg.FindPointer ("lock", reinterpret_cast<void **>(&endpointLock));
#endif
    serverSid = getMsg.FindInt32 ("sid");
    
    if (sMsgrE->SendMessage (M_GET_RECONNECT_STATUS, &reply) == B_OK)
    {
      int retrycount (reply.FindInt32 ("retries"));
      
      if (retrycount)
        snooze (1000000); // wait 1 second
    
      if (sMsgrE->SendMessage (M_INC_RECONNECT) != B_OK)
        throw failToLock();
      statString = S_SERVER_ATTEMPT1;
      if (retrycount != 0)
        statString += S_SERVER_ATTEMPT2;
      statString += S_SERVER_ATTEMPT3;
      statString << retrycount + 1;
      statString += S_SERVER_ATTEMPT4;
      statString << reply.FindInt32 ("max_retries");
      statString += ")\n";
      ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
      sMsgrE->SendMessage (&statMsg);
      
      BMessage data (M_DISPLAY);
      data.AddString ("data", statString.String());
      data.AddInt32 ("fore", C_ERROR);
      sMsgrE->SendMessage (&data);

    }
    else
      throw failToLock();
 
    statString = S_SERVER_ATTEMPT5;
    statString << connectId;
    statString += ":";
    statString << connectPort;
    statString += B_UTF8_ELLIPSIS "\n";
    ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
    sMsgrE->SendMessage (&statMsg);

    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
#ifdef BONE_BUILD
    if (inet_aton (connectId.String(), &remoteAddr.sin_addr) == 0)
#elif NETSERVER_BUILD
    if ((int)(remoteAddr.sin_addr.s_addr = inet_addr (connectId.String())) <= 0)
#endif
    {
       struct hostent *remoteInet (gethostbyname (connectId.String()));
       if (remoteInet)
         remoteAddr.sin_addr = *((in_addr *)remoteInet->h_addr_list[0]);
       else 
       {
         ClientAgent::PackDisplay (&statMsg, S_SERVER_CONN_ERROR1 "\n", C_ERROR);
         sMsgrE->SendMessage (&statMsg);
         sMsgrE->SendMessage (M_NOT_CONNECTING);
         sMsgrE->SendMessage (M_SERVER_DISCONNECT);
         throw failToLock();
       }
    }

    remoteAddr.sin_port = htons(atoi (connectPort.String()));
    remoteIP = inet_ntoa (remoteAddr.sin_addr);

    vision_app->AddIdent (remoteIP.String(), ident.String());      

    if ((serverSock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
         ClientAgent::PackDisplay (&statMsg, S_SERVER_CONN_ERROR1 "\n", C_ERROR);
         sMsgrE->SendMessage (&statMsg);
         sMsgrE->SendMessage (M_NOT_CONNECTING);
         sMsgrE->SendMessage (M_SERVER_DISCONNECT);
         throw failToLock();
    }
    
    // just see if he's still hanging around before
    // we got blocked for a minute
    ClientAgent::PackDisplay (&statMsg, S_SERVER_CONN_OPEN "\n", C_ERROR);
    sMsgrE->SendMessage (&statMsg);
    sMsgrE->SendMessage (M_LAG_CHANGED);

    if (connect (serverSock, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) >= 0)
    {
      BString ip ("");  
      struct sockaddr_in sockin;

      // store local ip address for future use (dcc, etc)
      int addrlength (sizeof (struct sockaddr_in));
      if (getsockname (serverSock,(struct sockaddr *)&sockin,&addrlength)) {
        ClientAgent::PackDisplay (&statMsg, S_SERVER_LOCALIP_ERROR "\n", C_ERROR);
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
        statString = S_SERVER_LOCALIP;
        statString += ip.String();
        statString += "\n";
        ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
        sMsgrE->SendMessage (&statMsg);
      }

      if (PrivateIPCheck (ip.String()))
      {
        ClientAgent::PackDisplay (&statMsg, "\n", C_ERROR);
        sMsgrE->SendMessage (&statMsg);  
      }
      
            ClientAgent::PackDisplay (&statMsg, S_SERVER_HANDSHAKE "\n", C_ERROR);
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

      ClientAgent::PackDisplay (&statMsg, S_SERVER_ESTABLISH "\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);
    }
    else // No endpoint->connect
    {
      ClientAgent::PackDisplay (&statMsg, S_SERVER_CONN_ERROR2 "\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);
      sMsgrE->SendMessage (M_NOT_CONNECTING);
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
    vision_app->RemoveIdent (remoteIP.String());
    return B_ERROR;
  }
  
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
      if ((length = recv (serverSock, indata, 1023, 0)) > 0)
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

          if (vision_app->fDebugRecv)
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
        
        if (vision_app->fDebugRecv)
        {
          // print interesting info          
          printf ("Negative from endpoint receive! (%ld)\n", length);
          printf ("eset : %s\nrset: %s\nwset: %s\n",
            FD_ISSET (serverSock, &eset) ? "true" : "false",
            FD_ISSET (serverSock, &rset) ? "true" : "false",
            FD_ISSET (serverSock, &wset) ? "true" : "false");
		}
		
        // tell the user all about it
        sMsgrE->SendMessage (M_NOT_CONNECTING);
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
  int32 length (0);
  if (!cData)
    return;
    
  BString data (cData);

  data.Append("\r\n");
  length = data.Length();

  memset(fSend_buffer, 0, sizeof(fSend_buffer));

  int32 dest_length (sizeof(fSend_buffer)), state (0);

  convert_from_utf8 (
    B_ISO_8859_1,
    data.String(), 
    &length,
    fSend_buffer,
    &dest_length,
    &state);
    
#ifdef NETSERVER_BUILD
  fEndPointLock->Lock();
#endif
  if (fServerSocket > 0 &&
#ifdef BONE_BUILD  
  (length = send (fServerSocket, fSend_buffer, dest_length, MSG_DONTWAIT) < 0)
#elif NETSERVER_BUILD
  (length = send (fServerSocket, fSend_buffer, dest_length, 0) < 0)
#endif
  || fServerSocket < 0)
  {
    // doh, we aren't even connected.
    if (!fReconnecting && !fIsConnecting)
      fMsgr.SendMessage (M_SERVER_DISCONNECT);
  }
#ifdef NETSERVER_BUILD
  fEndPointLock->Unlock();
#endif
  if (vision_app->fDebugSend)
  {
    data.RemoveAll ("\n");
    data.RemoveAll ("\r");
    printf("    SENT: (%ld:%03ld) \"%s\"\n", fSid, length, data.String());
  }
}

void
ServerAgent::ParseLine (const char *cData)
{
  BString data = FilterCrap (cData);
  int32 length (data.Length() + 1);

  if (fParse_size < length * 3UL)
  {
    if (fParse_buffer)
      delete [] fParse_buffer;
    fParse_buffer = new char [length * 3];
    fParse_size = length * 3;
  }

  int32 dest_length (fParse_size), state (0);

  convert_to_utf8 (
    B_ISO1_CONVERSION,
    data.String(), 
    &length,
    fParse_buffer,
    &dest_length,
    &state);
  
  if (vision_app->fNumBench)
  {
    vision_app->fBench1 = system_time();
    if (ParseEvents (fParse_buffer))
    {
      vision_app->fBench2 = system_time();
      BString bencht (GetWord (data.String(), 2));
      vision_app->BenchOut (bencht.String());
      return;
    }
  }
  else
  {
    if (ParseEvents (fParse_buffer))
      return;
  }
  

  data.Append("\n");
  Display (data.String(), 0);
}

ClientAgent *
ServerAgent::Client (const char *cName)
{
  ClientAgent *client (0);

  for (int32 i = 0; i < fClients.CountItems(); ++i)
  {
    ClientAgent *item ((ClientAgent *)fClients.ItemAt (i));
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

  for (int32 i = 0; i < fClients.CountItems(); ++i)
    if (!((ClientAgent *)fClients.ItemAt (i))->IsHidden())
      client = (ClientAgent *)fClients.ItemAt (i);

  return client;
}


void
ServerAgent::Broadcast (BMessage *msg)
{
  for (int32 i = 0; i < fClients.CountItems(); ++i)
  {
    ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));

    if (client != this)
      client->fMsgr.SendMessage (msg);
  }
  if (fListAgent)
    vision_app->pClientWin()->DispatchMessage(msg, (BView *)fListAgent);
}

void
ServerAgent::RepliedBroadcast (BMessage *)
{
//  TODO: implement this
//  BMessage cMsg (*msg);
//  BAutolock lock (this);
//
//  for (int32 i = 0; i < fClients.CountItems(); ++i)
//  {
//    ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));
//
//    if (client != this)
//    {
//      BMessenger fMsgr (client);
//      BMessage reply;
//      fMsgr.SendMessage (&cMsg, &reply);
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
  for (int32 i = 0; i < fClients.CountItems(); ++i)
  {
    ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));

    BMessage msg (M_DISPLAY);
    PackDisplay (&msg, buffer, fore, back, font);
    client->fMsgr.SendMessage (&msg);
  }

  return;
}

void
ServerAgent::PostActive (BMessage *msg)
{
  BAutolock activeLock (Window());
  ClientAgent *client (ActiveClient());

  if (client)
    client->fMsgr.SendMessage (msg);
  else
    fMsgr.SendMessage (msg);
}

void
ServerAgent::HandleReconnect (void)
{
  /*
   * Function purpose: Setup the environment and attempt a new connection
   * to the server 
   */
   
  if (fIsConnected)
  {
    // what's going on here?!
    printf (":ERROR: HandleReconnect() called when we're already connected! Whats up with that?!");
    return;
  }

  if (fRetry < fRetryLimit)
  {
    BString name;
    vision_app->GetThreadName(THREAD_S, name);
    // we are go for main engine start
    fReconnecting = true;
    fIsConnecting = true;
    fNickAttempt = 0;
    fEstablishHasLock = false;
#ifdef NETSERVER_BUILD
    fEndPointLock = new BLocker();
#endif
    fLoginThread = spawn_thread (
      Establish,
      name.String(),
      B_NORMAL_PRIORITY,
      new BMessenger(this));
    
    close (fServerSocket);  
    fServerSocket = -1;

    resume_thread (fLoginThread);
  }
  else
  {
    // we've hit our fRetry limit. throw in the towel
    fReconnecting = false;
    fRetry = 0;
    const char *soSorry;
    soSorry = S_SERVER_RETRY_LIMIT "\n";
    Display (soSorry, C_ERROR);
    ClientAgent *agent (ActiveClient());
    if (agent && (agent != this))
      agent->Display (soSorry, C_ERROR, C_BACKGROUND, F_SERVER);    
  }
}

const ServerData *
ServerAgent::GetNextServer ()
{
  type_code type;
  int32 count,
    size;
  fNetworkData.GetInfo ("server", &type, &count);
  uint32 state = (fReconnecting) ? 1 : 0;

  for (;;)
  {
    if (fServerIndex >= count)
    {
      fServerIndex = -1;
      state = 0;
    }
    
    const ServerData *server (NULL);
    for (; ++fServerIndex, fNetworkData.FindData ("server", B_RAW_TYPE, fServerIndex, 
      reinterpret_cast<const void **>(&server), &size) == B_OK;)
        if (server->state == state)
          return server;
  }
}

const char *
ServerAgent::GetNextNick ()
{
  type_code type;
  int32 count;
  fNetworkData.GetInfo ("nick", &type, &count);
  if (fNickIndex < count)
    return fNetworkData.FindString ("nick", fNickIndex++);
  else
  {
    fNickIndex = 0;
    return "";
  }
}

bool
ServerAgent::PrivateIPCheck (const char *ip)
{
  /*
   * Function purpose: Compare against fLocalip to see if it is a private address
   *                   if so, set fLocalip_private to true;
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
ServerAgent::AddResumeData (BMessage *msg)
{
  ResumeData *data;

  data = new ResumeData;

  data->expire = system_time() + 50000000LL;
  data->nick   = msg->FindString ("vision:nick");
  data->file   = msg->FindString ("vision:file");
  data->size   = msg->FindString ("vision:size");
  data->ip     = msg->FindString ("vision:ip");
  data->port   = msg->FindString ("vision:port");
  data->path   = msg->FindString ("path");
  data->pos    = msg->FindInt64  ("pos");
	
  //PRINT(("%s %s %s %s %s", data->nick.String(), data->file.String(), 
  //	data->size.String(), data->ip.String(), data->port.String()));
  fResumes.AddItem (data);

  BString buffer;

  buffer << "PRIVMSG "
    << data->nick
    << " :\1DCC RESUME "
    << data->file
    << " "
    << data->port
    << " "
    << data->pos
    << "\1";

  SendData (buffer.String());
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
        if (buffer)
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
        reply.AddData ("server", B_RAW_TYPE, GetNextServer(), sizeof (ServerData));
        reply.AddString  ("ident", fLident.String());
        reply.AddString  ("name", fLname.String());
        reply.AddString  ("nick", fMyNick.String());
#ifdef NETSERVER_BUILD
        reply.AddPointer ("lock", fEndPointLock);
#endif
        reply.AddInt32   ("sid", fSid);
        msg->SendReply (&reply); 
        fEstablishHasLock = true;
      }
      break;
    
    case M_SET_ENDPOINT:
      msg->FindInt32 ("socket", &fServerSocket);
      break;
      
    case M_NOT_CONNECTING:
      fIsConnecting = false;
      break;
     
    case M_CONNECTED:
      fIsConnected = true;
      break;
    
    case M_INC_RECONNECT:
      ++fRetry;
      break;
    
    case M_SET_IP:
      {
        static BString ip;
        msg->FindString("ip", &ip);
        fLocalip = ip.String();
        fLocalip_private = msg->FindBool("private");
        fGetLocalIP = fLocalip_private;
      }
      break;
    
    case M_GET_IP:
      {
        BMessage reply;
        reply.AddBool ("private", fLocalip_private);
        reply.AddString ("ip", fLocalip);
        msg->SendReply (&reply);
      }
      break;

    case M_GET_RECONNECT_STATUS:
      {
        BMessage reply (B_REPLY);
        reply.AddInt32 ("retries", fRetry);
        reply.AddInt32 ("max_retries", fRetryLimit);
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

    case M_DCC_ACCEPT:
      {
        bool cont (false);
        const char *nick,
   	               *size,
			       *ip,
			       *port;
        BPath path;
			
        msg->FindString("vision:nick", &nick);
        msg->FindString("vision:size", &size);
        msg->FindString("vision:ip", &ip);
        msg->FindString("vision:port", &port);
			
        if (msg->HasString ("path"))
          path.SetTo (msg->FindString ("path"));
        else
        {
          const char *file;
          entry_ref ref;

          msg->FindRef ("directory", &ref);
          msg->FindString("name", &file);
 
          BDirectory dir (&ref);
          path.SetTo (&dir, file);
        }

        if (msg->HasBool ("continue"))
          msg->FindBool ("continue", &cont);
        
        DCCReceive *view;
        view = new DCCReceive (
          nick,
          path.Path(),
          size,
          ip,
          port,
          fSMsgr,
          cont);
	
          BMessage aMsg (M_DCC_FILE_WIN);
          aMsg.AddPointer ("view", view);
          be_app->PostMessage (&aMsg);
      }
      break;
      
    case M_CHOSE_FILE: // DCC send
      {
        const char *nick;
        entry_ref ref;
        off_t size;
        msg->FindString ("nick", &nick);
        msg->FindRef ("refs", &ref); // get file
			
        BEntry entry (&ref);
        BPath path (&entry);
        // PRINT(("file path: %s\n", path.Path()));
        entry.GetSize (&size);

        BString ssize;
        ssize << size;

        // because of a bug in the be library
        // we have to get the sockname on this
        // socket, and not the one that DCCSend
        // binds.  calling getsockname on a
        // a binded socket will return the
        // LAN ip over the DUN one 
			
        DCCSend *view;

        view = new DCCSend (
                nick,
                path.Path(),
                ssize.String(),
                fSMsgr);
            BMessage msg (M_DCC_FILE_WIN);
            msg.AddPointer ("view", view);
            vision_app->PostMessage (&msg);

      }
      break;

    case M_ADD_RESUME_DATA:
      {
        AddResumeData (msg);
      }
      break;


    case B_CANCEL:
      if (msg->HasPointer ("source"))
      {
        BFilePanel *fPanel;
        msg->FindPointer ("source", reinterpret_cast<void **>(&fPanel));
        delete fPanel;
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
            fSid,
            fId.String(),
            fSMsgr,
            fMyNick.String(),
            "",
            true,
            false,
            theIP,
            thePort),
          fSid,
          theNick.String(),
          WIN_MESSAGE_TYPE,
          true);
          
          ClientAgent *client (vision_app->pClientWin()->pWindowList()->Agent (fSid, theNick.String()));
          fClients.AddItem (client);
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
              fSid,
              fId.String(),
              fSMsgr,
              fMyNick.String(),
              "",
              true,
              true,
              "",
              thePort != "" ? thePort.String() : ""),
            fSid,
            theId.String(),
            WIN_MESSAGE_TYPE,
            true);
         client = vision_app->pClientWin()->pWindowList()->Agent (fSid, theId.String());
         fClients.AddItem (client);
       }
     }
     break;

    case M_SLASH_RECONNECT:
      if (!fIsConnected && !fIsConnecting)
        fMsgr.SendMessage (M_SERVER_DISCONNECT);
      break;

    case M_SERVER_DISCONNECT:
      {
        // store current nick for reconnect use (might be an away nick, etc)
        if (fReacquiredNick)
        {
          fReconNick = fMyNick;
          fReacquiredNick = false;
        }
        // let the user know
        if (fIsConnected)
        {
          BString sAnnounce;
          sAnnounce += S_SERVER_DISCONNECT;
          sAnnounce += fServerName;
          sAnnounce += "\n";
          Display (sAnnounce.String(), C_ERROR);
          ClientAgent *agent (ActiveClient());
          if (agent && (agent != this))
            agent->Display (sAnnounce.String(), C_ERROR, C_BACKGROUND, F_SERVER);
        }
			
        fIsConnected = false;
       
        fMyLag = S_SERVER_DISCON_STATUS;
        fMsgr.SendMessage (M_LAG_CHANGED);
        fCheckingLag = false;
        fServerSocket = -1;
      
        // attempt a reconnect
        if (!fIsConnecting)
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
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, fId.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String(), true);
      }
      break;

    case M_LAG_CHECK:
      {
        if (fIsConnected && fNetworkData.FindBool ("lagCheck"))
        {
          BMessage lagSend (M_SERVER_SEND);
          AddSend (&lagSend, "VISION_LAG_CHECK");
          AddSend (&lagSend, endl);
          if (!fCheckingLag)
          {
            fLagCheck = system_time();
            fLagCount = 1;
            fCheckingLag = true;
          }
          else
          {
            if (fLagCount > 4)
            {
              // we've waited 50 seconds
              // connection problems?
              fMyLag = S_SERVER_CONN_PROBLEM;
              fMsgr.SendMessage (M_LAG_CHANGED);
            }
            else
            {
              // wait some more
              char lag[15] = "";
              sprintf (lag, "%ld0.000+", fLagCount);  // assuming a 10 second runner
              fMyLag = lag;
              ++fLagCount;
              fMsgr.SendMessage (M_LAG_CHANGED);
            }
          }
        }	
      }
      break;
      
    case M_LAG_CHANGED:
      {
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), true);
          
        BMessage newmsg (M_LAG_CHANGED);
        newmsg.AddString ("lag", fMyLag);
        Broadcast (&newmsg);        
      }
      break;

    case M_REJOIN_ALL:
      {
        for (int32 i = 0; i < fClients.CountItems(); ++i)
        {
          ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));
          
          if (dynamic_cast<ChannelAgent *>(client))
          {
            BMessage rejoinMsg (M_REJOIN);
            rejoinMsg.AddString ("nickname", fMyNick.String());
            client->fMsgr.SendMessage (&rejoinMsg);
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
            (client = new MessageAgent (
              *vision_app->pClientWin()->AgentRect(),
              theNick,
              fSid,
              fId.String(),
              fSMsgr,
              fMyNick.String(),
              "")),
            fSid,
            theNick,
            WIN_MESSAGE_TYPE,
            true);

          fClients.AddItem (client);
        }
        else
          client->fAgentWinItem->ActivateItem();
        
        if (msg->HasMessage ("msg"))
        {
          BMessage buffer;

          msg->FindMessage ("msg", &buffer);
          client->fMsgr.SendMessage (&buffer);
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
          fQuitMsg = quitstr;
        }

        fIsQuitting = true;

        if (fIsConnected && fServerSocket)
        {
          if (fQuitMsg.Length() == 0)
          {
            const char *expansions[1];
            BString version (vision_app->VisionVersion(VERSION_VERSION));
            expansions[0] = version.String();
            fQuitMsg << "QUIT :" << ExpandKeyed (vision_app->GetCommand (CMD_QUIT).String(), "V", expansions);
          }
 
          SendData (fQuitMsg.String());
        }

        Broadcast (new BMessage (M_CLIENT_QUIT));
		BMessenger listMsgr(fListAgent);
		listMsgr.SendMessage(M_CLIENT_QUIT);
		
        BMessage deathchant (M_OBITUARY);
        deathchant.AddPointer ("agent", this);
        deathchant.AddPointer ("item", fAgentWinItem);
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
    
        fClients.RemoveItem (deadagent);

        if (fIsQuitting && fClients.CountItems() <= 1)
          fSMsgr.SendMessage (M_CLIENT_QUIT);
      }
      break;
    
    case M_LIST_COMMAND:
      {
        if (fListAgent)
          break;
        vision_app->pClientWin()->pWindowList()->AddAgent (
          (fListAgent = new ListAgent (
            *vision_app->pClientWin()->AgentRect(),
            fServerHostName.String(), new BMessenger(this))),
          fSid,
          "Channels",
          WIN_LIST_TYPE,
          true);
        // kind of a hack since Agent() returns a pointer of type ClientAgent, of which
        // ListAgent is not a subclass...
        BMessenger listMsgr(fListAgent);
        listMsgr.SendMessage(M_LIST_COMMAND);
      }
      break;
    
    case M_LIST_SHUTDOWN:
      fListAgent = NULL;
      break;
    
    case M_REGISTER_LOGGER:
      {
        const char *logName;
        msg->FindString ("name", &logName);
        fLogger->RegisterLogger (logName);
      }
      break;
    
    case M_UNREGISTER_LOGGER:
      {
        const char *logName;
        msg->FindString ("name", &logName);
        fLogger->UnregisterLogger (logName);
      }
      break;
    
    case M_CLIENT_LOG:
      {
        const char *logName;
        const char *data;
        msg->FindString ("name", &logName);
        msg->FindString ("data", &data);
        fLogger->Log (logName, data);
      }
      break;

    default:
      ClientAgent::MessageReceived (msg);
  }
}
