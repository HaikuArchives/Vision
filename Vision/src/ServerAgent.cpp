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
#include "NotifyList.h"
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
    fLoginThread (-1),
    fSenderThread (-1),
    fEvents (vision_app->fEvents),
    fServerHostName (id_),
    fInitialMotd (true),
    fCmds (net.FindString ("autoexec")),
    fListAgent (NULL),
    fNetworkData (net),
    fServerIndex (0),
    fNickIndex (0),
    fLogger (NULL)
{
  fLogger = new ClientAgentLogger(fId);
}

ServerAgent::~ServerAgent (void)
{
  if (fLagRunner)
    delete fLagRunner;
  
#ifdef NETSERVER_BUILD
  if (!fEstablishHasLock && fEndPointLock)
    delete fEndPointLock;
#endif

  while (fStartupChannels.CountItems() != 0)
    delete fStartupChannels.RemoveItem (0L);

  delete fLogger;

  delete_sem (fSendSyncSem);
  
  status_t result;
  wait_for_thread (fSenderThread, &result);

  while (fIgnoreNicks.CountItems() > 0)
    delete fIgnoreNicks.RemoveItem(0L);

  while (fNotifyNicks.CountItems() > 0)
    delete fNotifyNicks.RemoveItem(0L);
}

void
ServerAgent::AttachedToWindow(void)
{
  Init();
  ClientAgent::AttachedToWindow();
}

void
ServerAgent::AllAttached (void)
{
  fSMsgr = BMessenger (this);
  ClientAgent::AllAttached ();

  // build notify list here since we can't send a message to ourselves sooner

  type_code type;
  int32 attrCount (0);
  
  BMessage updatemsg (M_NOTIFYLIST_ADD);
  BString data;
  
  fNetworkData.GetInfo ("notify", &type, &attrCount);

  for (int32 i = 0; i < attrCount; i++)
  {
    data += fNetworkData.FindString("notify", i);
    data += " ";
  }
  if (attrCount > 0)
  {
    updatemsg.AddString("cmd", data.String());
    fSMsgr.SendMessage(&updatemsg);
  }
}

// do nothing for now
void
ServerAgent::AddMenuItems(BPopUpMenu *)
{
}


void
ServerAgent::Init (void)
{
  fPendingSends = new BList();
  fSendLock = new BLocker();
#ifdef NETSERVER_BUILD
  fEndPointLock = new BLocker();
#endif
  BString revString;
  Display ("Vision ");
  vision_app->VisionVersion (VERSION_VERSION, revString);
  Display (revString.String(), C_MYNICK);
  Display (" built on ");
  vision_app->VisionVersion (VERSION_DATE, revString);
  Display (revString.String());
  Display ("\nThis agent goes by the name of Smith... err ");
  BString temp;
  temp << fId << "\n";
  Display (temp.String(), C_NICK);
  Display ("Have fun!\n");
  
  fSendSyncSem = create_sem (0, "VisionSendSync");
  
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
  
  name = "t>";
  
  name += (rand() %2) ? "Tima" : "Kenichi";
  
  fSenderThread = spawn_thread (
    Sender,
    name.String(),
    B_NORMAL_PRIORITY,
    this);

  resume_thread (fLoginThread);
  resume_thread (fSenderThread);
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
ServerAgent::Sender (void *arg)
{
  ServerAgent *agent (reinterpret_cast<ServerAgent *>(arg));
  BMessenger msgr (agent);
  sem_id sendSyncLock (-1);
  BLocker *sendDataLock (NULL);
  BList *pendingSends (NULL);
  
  if (msgr.IsValid() && msgr.LockTarget())
  {
    sendSyncLock = agent->fSendSyncSem;
    sendDataLock = agent->fSendLock;
    pendingSends = agent->fPendingSends;
    // this is safe since we know we have the looper at this point
    agent->Window()->Unlock();
  }
  else
    return B_ERROR;
    
  BString *data (NULL);
  while (acquire_sem (sendSyncLock) == B_NO_ERROR)
  {
    sendDataLock->Lock();
    if (!pendingSends->IsEmpty())
    {
      data = (BString *)pendingSends->RemoveItem (0L);
      sendDataLock->Unlock();
      agent->AsyncSendData (data->String());
      delete data;
      data = NULL;
    }
    else
      sendDataLock->Unlock();
      
  }
  
  // sender takes possession of pending sends and sendDataLock structures
  // allows for self-contained cleanups
  while (pendingSends->CountItems() > 0)
    delete pendingSends->RemoveItem (0L);
  delete pendingSends;
  delete sendDataLock;

  return B_OK;
}

bool
ServerAgent::ServerThreadValid (thread_id tid)
{
  return (tid == fLoginThread);
}

int32
ServerAgent::Establish (void *arg)
{
  BMessenger *sMsgrE (reinterpret_cast<BMessenger *>(arg));
  BMessage getMsg;
  thread_id currentThread (find_thread(NULL));
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
        snooze (1000000 * retrycount * retrycount); // wait 1, 4, 9, 16 ... seconds
      
    
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
        if (vision_app->GetBool ("dccPrivateCheck"))
        {
          setIP.AddBool("private", PrivateIPCheck (ip.String()));
        }
        else
        {
          setIP.AddBool("private", false);
        }
        setIP.AddString("ip", ip.String());
        sMsgrE->SendMessage(&setIP);
        statString = S_SERVER_LOCALIP;
        statString += ip.String();
        statString += "\n";
        ClientAgent::PackDisplay (&statMsg, statString.String(), C_ERROR);
        sMsgrE->SendMessage (&statMsg);
      }

      if (vision_app->GetBool("dccPrivateCheck") && PrivateIPCheck (ip.String()))
      {
        ClientAgent::PackDisplay (&statMsg, S_SERVER_PROXY_MSG "\n", C_ERROR);
        sMsgrE->SendMessage (&statMsg);  
      }
      
      ClientAgent::PackDisplay (&statMsg, S_SERVER_HANDSHAKE "\n", C_ERROR);
      sMsgrE->SendMessage (&statMsg);

      BString string;
      BMessage dataSend (M_SERVER_SEND);
      dataSend.AddString ("data", "blah");

      BMessage endpointMsg (M_SET_ENDPOINT);
      endpointMsg.AddInt32 ("socket", serverSock);
      if (sMsgrE->SendMessage (&endpointMsg, &reply) != B_OK)
        throw failToLock();
        
      if (connectId.ICompare("64.156.75", 9) == 0)
      {
        string = "PASS 2legit2quit";
        dataSend.ReplaceString ("data", string.String());
        sMsgrE->SendMessage (&dataSend);
      }

      string = "USER ";
      string.Append (ident);
      string.Append (" localhost ");
      string.Append (connectId);
      string.Append (" :");
      string.Append (name);

      dataSend.ReplaceString ("data", string.String());
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

  FD_ZERO (&eset);
  FD_ZERO (&rset);
  FD_ZERO (&wset);

  BString buffer;
  
  FD_SET (serverSock, &eset);
  FD_SET (serverSock, &rset);
  FD_SET (serverSock, &wset);
  
  BMessage threadCheck (M_SERVER_THREAD_VALID),
           reply;
  threadCheck.AddInt32 ("thread", currentThread);
  while (sMsgrE->IsValid())
  {
    sMsgrE->SendMessage (&threadCheck, &reply);
    
    if (!reply.FindBool("valid"))
      break;
    
    char indata[1024];
    int32 length (0);
    
    FD_SET (serverSock, &eset);
    FD_SET (serverSock, &rset);
    FD_SET (serverSock, &wset);
    memset (indata, 0, 1024);
    if (select (serverSock + 1, &rset, 0, &eset, NULL) > 0
    &&  FD_ISSET (serverSock, &rset) && !FD_ISSET (serverSock, &eset))
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

int
ServerAgent::SortNotifyItems (const void *item1, const void *item2)
{
  const NotifyListItem *ptr1 (*((NotifyListItem * const *)item1));
  const NotifyListItem *ptr2 (*((NotifyListItem * const *)item2));
  
  if (!ptr1 || !ptr2)
    return 0;
  
  if (ptr1->GetState() && !ptr2->GetState())
    return -1;
  if (!ptr1->GetState() && ptr2->GetState())
    return 1;
  
  BString name (ptr1->Text());
    
  return name.ICompare(ptr2->Text());
}

void
ServerAgent::AsyncSendData (const char *cData)
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
    B_ISO1_CONVERSION,
    data.String(), 
    &length,
    fSend_buffer,
    &dest_length,
    &state);
    
#ifdef NETSERVER_BUILD
  fEndPointLock->Lock();
#endif
  if (fServerSocket <= 0)
  {
    if (!fReconnecting && !fIsConnecting)
      fMsgr.SendMessage (M_SERVER_DISCONNECT);
      // doh, we aren't even connected.
      return;
  }

  struct fd_set eset, wset;
  FD_ZERO (&wset);
  FD_ZERO (&eset);
  FD_SET (fServerSocket, &wset);
  FD_SET (fServerSocket, &eset);
  struct timeval tv = { 5 , 0 };
  // do a select to prevent the writer thread from deadlocking in the send call
  if (select(fServerSocket + 1, NULL, &wset, &eset, &tv) <= 0 || FD_ISSET(fServerSocket, &eset)
    || !FD_ISSET(fServerSocket, &wset))
  {
    if (!fReconnecting && !fIsConnecting)
      fMsgr.SendMessage (M_SERVER_DISCONNECT);
  }
  else
  {
    if ((length = send (fServerSocket, fSend_buffer, dest_length, 0) < 0))
    {
      if (!fReconnecting && !fIsConnecting)
        fMsgr.SendMessage (M_SERVER_DISCONNECT);
      // doh, we aren't even connected.
    }
  }

#ifdef NETSERVER_BUILD
  fEndPointLock->Unlock();
#endif
  if (vision_app->fDebugSend)
  {
    data.RemoveAll ("\n");
    data.RemoveAll ("\r");
    printf("    SENT: (%03ld) \"%s\"\n", length, data.String());
  }
}

void
ServerAgent::SendData (const char *cData)
{
  // this function simply queues up the data into the requests buffer, then releases
  // the sender thread to take care of it
  BString *data (new BString (cData));
  fSendLock->Lock();
  fPendingSends->AddItem (data);
  fSendLock->Unlock();
  release_sem (fSendSyncSem);
}

void
ServerAgent::ParseLine (const char *cData)
{
  BString data = FilterCrap (cData);
  int32 length (data.Length()),
        destLength (2048),
        state (0);
  
  memset (fParse_buffer, 0, sizeof (fParse_buffer));
  
  convert_to_utf8 (
    B_ISO1_CONVERSION,
    data.String(), 
    &length,
    fParse_buffer,
    &destLength,
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
  
  // empty out old send buffer to ensure no erroneous strings get sent
  fSendLock->Lock();
  while (fPendingSends->CountItems() > 0)
    delete fPendingSends->RemoveItem (0L);
  fSendLock->Unlock();
  
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
      fServerIndex = 0;
      state = 0;
    }
    
    const ServerData *server (NULL);
    for (; fNetworkData.FindData ("server", B_RAW_TYPE, fServerIndex, 
      reinterpret_cast<const void **>(&server), &size) == B_OK; fServerIndex++)
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
ServerAgent::ParseAutoexecChans (const BString &origLine)
{
  BString line (origLine);
  int32 chanIndex (0);
  if ((chanIndex = line.IFindFirst ("/JOIN")) != B_ERROR)
  {
    chanIndex += 6;
    line.Remove (0, chanIndex);
  }
  else if ((chanIndex = line.IFindFirst ("/J")) != B_ERROR)
  {
    chanIndex += 3;
    line.Remove (0, chanIndex);
  }
  else
    return;
  // parse out all autoexec channels to ensure we don't try to focus those
  // on join
  chanIndex = 0;
  BString *newChan (NULL);
  for (;;)
  {
    if ((chanIndex = line.FindFirst (',')) == B_ERROR)
      break;
    newChan = new BString();
    line.CopyInto (*newChan, 0, chanIndex);
    if ((*newChan)[0] != '#')
      newChan->Prepend("#");
    fStartupChannels.AddItem (newChan);
    line.Remove (0, chanIndex + 1);
  }
  newChan = new BString();
  // catch last channel (or only channel if no comma separations)
  if ((chanIndex = line.FindFirst (' ')) != B_ERROR)
    line.CopyInto (*newChan, 0, chanIndex);
  else
    *newChan = line;
  if ((*newChan)[0] != '#')
    newChan->Prepend("#");
  fStartupChannels.AddItem (newChan);
}

void
ServerAgent::RemoveAutoexecChan (const BString &chan)
{
  int32 chanCount (fStartupChannels.CountItems());
  for (int32 i = 0; i < chanCount; i++)
    if (((BString *)fStartupChannels.ItemAt (i))->ICompare(chan) == 0)
    {
      delete fStartupChannels.RemoveItem (i);
      return;
    }
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
    
    case M_SERVER_THREAD_VALID:
      {
        BMessage reply (B_REPLY);
        reply.AddBool ("valid", ServerThreadValid(msg->FindInt32("thread")));
        msg->SendReply(&reply);
      }
      break;
      
    case M_SEND_RAW:
      {
        const char *buffer;
        msg->FindString ("line", &buffer);
        if (buffer)
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
        msg->SendReply (&reply); 
        fEstablishHasLock = true;
      }
      break;
    
    case M_SET_ENDPOINT:
      msg->FindInt32 ("socket", &fServerSocket);
      break;
      
    case M_NOT_CONNECTING:
      fIsConnecting = false;
      fReconnecting = false;
      break;
     
    case M_CONNECTED:
      fIsConnected = true;
      fIsConnecting = false;
      fReconnecting = false;
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
        MessageAgent *newAgent (new MessageAgent (
            *vision_app->pClientWin()->AgentRect(),
            theNick.String(),
            fId.String(),
            fSMsgr,
            fMyNick.String(),
            "",
            true,
            false,
            theIP,
            thePort));
        vision_app->pClientWin()->pWindowList()->AddAgent (newAgent,
          theNick.String(),
          WIN_MESSAGE_TYPE,
          true);
          
          fClients.AddItem (newAgent);
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
          MessageAgent *newAgent (new MessageAgent (
              *vision_app->pClientWin()->AgentRect(),
              theId.String(),
              fId.String(),
              fSMsgr,
              fMyNick.String(),
              "",
              true,
              true,
              "",
              thePort != "" ? thePort.String() : ""));
          vision_app->pClientWin()->pWindowList()->AddAgent (newAgent,
            theId.String(),
            WIN_MESSAGE_TYPE,
            true);

         fClients.AddItem (newAgent);
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
          fIsConnected = false;
          BString sAnnounce;
          sAnnounce += S_SERVER_DISCONNECT;
          sAnnounce += fServerName;
          sAnnounce += "\n";
          Display (sAnnounce.String(), C_ERROR);
          ClientAgent *agent (ActiveClient());
          if (agent && (agent != this))
            agent->Display (sAnnounce.String(), C_ERROR, C_BACKGROUND, F_SERVER);
        }
       
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
        if (fIsConnected)
        {
          if (fNetworkData.FindBool ("lagCheck"))
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
          if (fNotifyNicks.CountItems() > 0)
          {
            BString cmd ("ISON ");
            for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
            {
              cmd += " ";
              cmd += ((NotifyListItem *)fNotifyNicks.ItemAt(i))->Text();
            }
            BMessage dataSend (M_SERVER_SEND);
            dataSend.AddString ("data", cmd.String());
            fSMsgr.SendMessage (&dataSend);
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
          MessageAgent *newAgent (new MessageAgent (
              *vision_app->pClientWin()->AgentRect(),
              theNick,
              fId.String(),
              fSMsgr,
              fMyNick.String(),
              ""));
          vision_app->pClientWin()->pWindowList()->AddAgent (
            newAgent,
            theNick,
            WIN_MESSAGE_TYPE,
            true);
            
          client = newAgent;

          fClients.AddItem (newAgent);
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
            BString version;
            vision_app->VisionVersion(VERSION_VERSION, version);
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
          "Channels",
          WIN_LIST_TYPE,
          true);
        // kind of a hack since Agent() returns a pointer of type ClientAgent, of which
        // ListAgent is not a subclass...
        BMessenger listMsgr(fListAgent);
        listMsgr.SendMessage(msg);
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
      
    case M_NOTIFYLIST_ADD:
      {
        BString cmd (msg->FindString("cmd"));
        BString curNick;
        int32 idx (-1);
        int32 i (0);

        // TODO: print notification message to user
        while ((idx = cmd.IFindFirst(" ")) > 0)
        {
          cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
          // remove leading space
          cmd.Remove(0, 1);
          for (i = 0; i < fNotifyNicks.CountItems(); i++)
            if (curNick.ICompare(((NotifyListItem *)fNotifyNicks.ItemAt(i))->Text()) == 0)
              break;
          // no dupes found, add it
          if (i == fNotifyNicks.CountItems())
          {
            vision_app->AddNotifyNick(fNetworkData.FindString("name"), curNick.String());
            NotifyListItem *item (new NotifyListItem (curNick.String(), false));
            fNotifyNicks.AddItem (item);
          }
          curNick = "";
        }
        // catch last one
        if (cmd.Length() > 0)
        {
          for (i = 0; i < fNotifyNicks.CountItems(); i++)
            if (cmd == ((NotifyListItem *)fNotifyNicks.ItemAt(i))->Text())
              break;
          // no dupes found, add it
          if (i == fNotifyNicks.CountItems())
          {
            NotifyListItem *item (new NotifyListItem (cmd.String(), false));
            vision_app->AddNotifyNick(fNetworkData.FindString("name"), cmd.String());
            fNotifyNicks.AddItem (item);
          }
        }
        
        fNotifyNicks.SortItems(SortNotifyItems);
        BMessage updMsg (M_NOTIFYLIST_UPDATE);
        updMsg.AddPointer ("list", &fNotifyNicks);
        updMsg.AddPointer ("source", this);
        Window()->PostMessage (&updMsg);
      }
      break;
    
    case M_NOTIFYLIST_REMOVE:
      {
        BString cmd (msg->FindString("cmd"));
        BString curNick;
        int32 idx (-1);

        // TODO: print notification message to user
        while ((idx = cmd.IFindFirst(" ")) > 0)
        {
          cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
          // remove leading space
          cmd.Remove(0, 1);
          vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), curNick.String());
          for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
            if (curNick.ICompare(((NotifyListItem *)fNotifyNicks.ItemAt(i))->Text()) == 0)
            {
              delete fNotifyNicks.RemoveItem(i);
            }
          curNick = "";
        }
        // catch last one
        if (cmd.Length() > 0)
        {
          vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), cmd.String());
          for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
            if (cmd.ICompare(((NotifyListItem *)fNotifyNicks.ItemAt(i))->Text()) == 0)
            {
              delete fNotifyNicks.RemoveItem(i);
            }
        }
        BMessage updMsg (M_NOTIFYLIST_UPDATE);
        updMsg.AddPointer ("list", &fNotifyNicks);
        updMsg.AddPointer ("source", this);
        Window()->PostMessage (&updMsg);
        
      }
      break;
    
    case M_NOTIFYLIST_UPDATE:
      {
        // force agent to update (used for winlist switches)
        BMessage newMsg (M_NOTIFYLIST_UPDATE);
        newMsg.AddPointer("list", &fNotifyNicks);
        newMsg.AddPointer("source", this);
        Window()->PostMessage(&newMsg);
      }
      break;

    case M_IGNORE_ADD:
      {
        BString cmd (msg->FindString("cmd"));
        for (int32 i = 0; i < fIgnoreNicks.CountItems(); i++)
        {
          if (cmd == *((BString *)fIgnoreNicks.ItemAt(i)))
            break;
        }
        fIgnoreNicks.AddItem (new BString(cmd));
      }
      break;
    
    default:
      ClientAgent::MessageReceived (msg);
  }
}

