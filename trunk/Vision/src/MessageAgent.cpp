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
 */

#include <PopUpMenu.h>
#include <MenuItem.h>
 
#include "MessageAgent.h"
#include "WindowList.h"
#include "ClientWindow.h"
#include "StatusView.h"
#include "Vision.h"
#include "VTextControl.h"

#include <unistd.h>
#include <stdlib.h>

#ifdef BONE_BUILD
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#elif NETSERVER_BUILD
#include <socket.h>
#include <netdb.h>
#endif

MessageAgent::MessageAgent (
  BRect &frame_,
  const char *id_,
  int32 sid_,
  const char *fServerName_,
  const BMessenger &fSMsgr_,
  const char *nick,
  const char *addyString,
  bool chat,
  bool initiate,
  const char *IP,
  const char *port)

  : ClientAgent (
    id_,
    sid_,
    fServerName_,
    nick,
    fSMsgr_,
    frame_),

  fChatAddy (addyString ? addyString : ""),
  fChatee (id_),
  fDIP (IP),
  fDPort (port),
  fDChat (chat),
  fDInitiate (initiate),
  fDConnected (false),
  fMySocket (0),
#ifdef NETSERVER_BUILD
  fAcceptSocket (0),
  fLocker (NULL)
#elif BONE_BUILD
  fAcceptSocket (0)
#endif
{
  Init();
#ifdef NETSERVER_BUILD
  if (fDChat)
    fLocker = new BLocker();
#endif
}

MessageAgent::~MessageAgent (void)
{
  fDConnected = false;
  if (fDChat)
  {
    if (fMySocket)
#ifdef BONE_BUILD
      close (fMySocket);
#elif NETSERVER_BUILD
      closesocket (fMySocket);
#endif
    if (fAcceptSocket)
#ifdef BONE_BUILD 
      close (fAcceptSocket);
#elif NETSERVER_BUILD
      closesocket (fAcceptSocket);
#endif
  }
}

void
MessageAgent::AllAttached (void)
{
  // initialize threads here since messenger will otherwise not be valid
  if (fDChat)
  {
    if (fDInitiate)
      fDataThread = spawn_thread(DCCIn, "DCC Chat(I)", B_NORMAL_PRIORITY, this);
    else
      fDataThread = spawn_thread(DCCOut, "DCC Chat(O)", B_NORMAL_PRIORITY, this);
    
    resume_thread (fDataThread);
  }
  ClientAgent::AllAttached();
}

void
MessageAgent::AddMenuItems (BPopUpMenu *pMenu)
{
  BMenuItem *item (NULL);
  item = new BMenuItem("Whois", new BMessage (M_MSG_WHOIS));
  item->SetTarget (this);
  if (Id().FindFirst (" [DCC]") >= 0)  // dont enable for dcc sessions
      item->SetEnabled (false);
  pMenu->AddItem (item);
  pMenu->AddSeparatorItem();
}

void
MessageAgent::Init (void)
{
// empty Init, call DCCServerSetup from input thread to avoid deadlocks
}

void
MessageAgent::DCCServerSetup(void)
{
  int32 myPort (atoi(vision_app->GetString ("dccMinPort")));
  int32 diff (atoi(vision_app->GetString ("dccMaxPort")) - myPort);
  if (diff > 0)
    myPort += rand() % diff;
    
  BString outNick (fChatee);
  outNick.RemoveFirst (" [DCC]");
  struct sockaddr_in sa;
  
  BMessage reply;
  fSMsgr.SendMessage (M_GET_IP, &reply);
  
  BString address;
  reply.FindString ("ip", &address);
    
  if (fDPort != "")
    myPort = atoi (fDPort.String());

  fMySocket = socket (AF_INET, SOCK_STREAM, 0);

  if (fMySocket < 0)
  {
    LockLooper();
    Display (S_DCC_SOCKET_ERROR);
    UnlockLooper();
    return;
  }

  sa.sin_family = AF_INET;

  sa.sin_addr.s_addr = INADDR_ANY;
  
  sa.sin_port = htons(myPort);
  
  if (bind (fMySocket, (struct sockaddr*)&sa, sizeof(sa)) == -1)
  {
      LockLooper();
      Display (S_DCC_BIND_ERROR);
      UnlockLooper();
      return;
  }
  
  BMessage sendMsg (M_SERVER_SEND);
  BString buffer;
  
  sa.sin_addr.s_addr = inet_addr (address.String());
  
  buffer << "PRIVMSG " << outNick << " :\1DCC CHAT chat ";
  buffer << htonl(sa.sin_addr.s_addr) << " ";
  buffer << myPort << "\1";
  sendMsg.AddString ("data", buffer.String());
  fSMsgr.SendMessage (&sendMsg);
  
  vision_app->AcquireDCCLock();	
  listen (fMySocket, 1);

  BString dataBuffer;
  struct in_addr addr;
		
  addr.s_addr = inet_addr (address.String());
  dataBuffer << S_DCC_CHAT_LISTEN 
    << address.String() << S_DCC_CHAT_PORT << myPort << "\n";
  LockLooper();
  Display (dataBuffer.String());
  UnlockLooper();
}

status_t
MessageAgent::DCCIn (void *arg)
{
  MessageAgent *agent ((MessageAgent *)arg);
  BMessenger fSMsgrE (agent->fSMsgr);
  BMessenger mMsgr (agent);
#ifdef NETSERVER_BUILD
  BLocker *myLocker (agent->fLocker);
#endif  
  agent->DCCServerSetup();
    
  int dccSocket (agent->fMySocket);
  int dccAcceptSocket (0);
  
  struct sockaddr_in remoteAddy;
  int theLen (sizeof (struct sockaddr_in));

  dccAcceptSocket = accept(dccSocket, (struct sockaddr*)&remoteAddy, &theLen);
  
  vision_app->ReleaseDCCLock();
  
  if (dccAcceptSocket < 0)
    return B_ERROR;
    
  agent->fAcceptSocket = dccAcceptSocket;
  agent->fDConnected = true;
  
  BMessage msg (M_DISPLAY);

  ClientAgent::PackDisplay (&msg, S_DCC_CHAT_CONNECTED);
  mMsgr.SendMessage (&msg);

  char tempBuffer[2];
  BString inputBuffer;
  int32 recvReturn (0);
  
  struct fd_set rset, eset;
#ifdef NETSERVER_BUILD
  struct timeval tv = { 0, 0 };
#endif
  FD_ZERO (&rset);
  FD_ZERO (&eset);
  FD_SET (dccAcceptSocket, &rset);
  FD_SET (dccAcceptSocket, &eset);
  
  while (mMsgr.IsValid() && agent->fDConnected)
  {
#ifdef NETSERVER_BUILD
    if (select (dccAcceptSocket + 1, &rset, NULL, &eset, &tv) > 0)
#elif BONE_BUILD
    if (select (dccAcceptSocket + 1, &rset, NULL, &eset, NULL) > 0)
#endif
    {
#ifdef NETSERVER_BUILD
      myLocker->Lock();
#endif
      if ((recvReturn = recv (dccAcceptSocket, tempBuffer, 1, 0)) == 0)
      {
        BMessage termMsg (M_DISPLAY);

        agent->fDConnected = false;
        ClientAgent::PackDisplay (&termMsg, S_DCC_CHAT_TERM);
        mMsgr.SendMessage (&termMsg);
        break;
      }
#ifdef NETSERVER_BUILD
      myLocker->Unlock();
#endif
      if (recvReturn > 0)
      {
        if (tempBuffer[0] == '\n')
        {
          inputBuffer.RemoveLast ("\r");
          inputBuffer = FilterCrap (inputBuffer.String(), false);
          BMessage dispMsg (M_CHANNEL_MSG);
          dispMsg.AddString ("msgz", inputBuffer.String());
          mMsgr.SendMessage (&dispMsg);
          inputBuffer = "";
        }
        else
          inputBuffer.Append(tempBuffer[0],1);
      }
    }
    else if (FD_ISSET (dccAcceptSocket, &eset))
    {
      BMessage termMsg (M_DISPLAY);
      agent->fDConnected = false;
      ClientAgent::PackDisplay (&termMsg, S_DCC_CHAT_TERM);
      mMsgr.SendMessage (&termMsg);
      break;
    }
#ifdef NETSERVER_BUILD
    else
      snooze (20000);
#endif
    FD_SET (dccAcceptSocket, &rset);
    FD_SET (dccAcceptSocket, &eset);
  }
	
#ifdef NETSERVER_BUILD
  myLocker->Unlock();
  delete myLocker;
#endif  
  return 0;
}

status_t
MessageAgent::DCCOut (void *arg)
{
  MessageAgent *agent ((MessageAgent *)arg);
  
  BMessenger mMsgr (agent);
#ifdef NETSERVER_BUILD
  BLocker *myLocker (agent->fLocker);
#endif
  struct sockaddr_in sa;
  int status;
  int dccAcceptSocket (0);
  char *endpoint;
  
  uint32 realIP = strtoul (agent->fDIP.String(), &endpoint, 10);

  if ((dccAcceptSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    BMessage msg (M_DISPLAY);

    ClientAgent::PackDisplay (&msg, S_DCC_SOCKET_ERROR);
    mMsgr.SendMessage (&msg);
    return false;
  }
  
  agent->fAcceptSocket = dccAcceptSocket;
	
  sa.sin_family = AF_INET;
  sa.sin_port = htons (atoi (agent->fDPort.String()));
  sa.sin_addr.s_addr = ntohl (realIP);
  memset (sa.sin_zero, 0, sizeof(sa.sin_zero));

  {
    BMessage msg (M_DISPLAY);
    BString buffer;
    struct in_addr addr;

    addr.s_addr = ntohl (realIP);

    buffer << S_DCC_CHAT_TRY
      << inet_ntoa (addr)
      << S_DCC_CHAT_PORT << agent->fDPort << "\n";
    
    ClientAgent::PackDisplay (&msg, buffer.String());
    mMsgr.SendMessage (&msg);
  }

  status = connect (dccAcceptSocket, (struct sockaddr *)&sa, sizeof(sa));
  if (status < 0)
  {
    BMessage msg (M_DISPLAY);

    ClientAgent::PackDisplay (&msg, S_DCC_CONN_ERROR);
    mMsgr.SendMessage (&msg);
#ifdef BONE_BUILD
    close (agent->fMySocket);
#elif NETSERVER_BUILD
    closesocket (agent->fMySocket);
#endif
    return false;
  }

  agent->fDConnected = true;

  BMessage msg (M_DISPLAY);
  ClientAgent::PackDisplay (&msg, S_DCC_CHAT_CONNECTED);
  mMsgr.SendMessage (&msg);

  char tempBuffer[2];
  BString inputBuffer;
  int32 recvReturn (0);

  struct fd_set rset, eset;
#ifdef NETSERVER_BUILD
  struct timeval tv = { 0, 0 };
#endif
  FD_ZERO (&rset);
  FD_ZERO (&eset);
  FD_SET (dccAcceptSocket, &rset);
  FD_SET (dccAcceptSocket, &eset);
  
  while (mMsgr.IsValid() && agent->fDConnected)
  {
#ifdef NETSERVER_BUILD
    if (select (dccAcceptSocket + 1, &rset, NULL, &eset, &tv) > 0)
#elif BONE_BUILD
    if (select (dccAcceptSocket + 1, &rset, NULL, &eset, NULL) > 0)
#endif
    {
#ifdef NETSERVER_BUILD
      myLocker->Lock();
#endif
      if ((recvReturn = recv (dccAcceptSocket, tempBuffer, 1, 0)) == 0)
      {
        BMessage termMsg (M_DISPLAY);

        agent->fDConnected = false;
        ClientAgent::PackDisplay (&termMsg, S_DCC_CHAT_TERM);
        mMsgr.SendMessage (&termMsg);
        break;
      }
#ifdef NETSERVER_BUILD
      myLocker->Unlock();
#endif
      if (recvReturn > 0)
      {
        if (tempBuffer[0] == '\n')
        {
          inputBuffer.RemoveLast ("\r");
          inputBuffer = FilterCrap (inputBuffer.String(), false);
          BMessage dispMsg (M_CHANNEL_MSG);
          dispMsg.AddString ("msgz", inputBuffer.String());
          mMsgr.SendMessage (&dispMsg);
          inputBuffer = "";
        }
        else
          inputBuffer.Append(tempBuffer[0],1);
      }
    }
    else if (FD_ISSET (dccAcceptSocket, &eset))
    {
      BMessage termMsg (M_DISPLAY);
      agent->fDConnected = false;
      ClientAgent::PackDisplay (&termMsg, S_DCC_CHAT_TERM);
      mMsgr.SendMessage (&termMsg);
      break;
    }
#ifdef NETSERVER_BUILD
    else
      snooze (20000);
#endif
    FD_SET (dccAcceptSocket, &rset);
    FD_SET (dccAcceptSocket, &eset);
  }
	
#ifdef NETSERVER_BUILD
  myLocker->Unlock();
  delete myLocker;
#endif  
  return 0;
}

void
MessageAgent::ChannelMessage (
  const char *msgz,
  const char *nick,
  const char *ident,
  const char *address)
{
//  fAgentWinItem->SetName (nick);

  ClientAgent::ChannelMessage (msgz, nick, ident, address);
}

void
MessageAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_CHANNEL_MSG:
      {
        const char *nick (NULL);
 
        if (msg->HasString ("nick"))
        {
          msg->FindString ("nick", &nick);
          BString outNick (nick);
          outNick.RemoveFirst (" [DCC]");
          if (fMyNick.ICompare (outNick) != 0 && !fDChat)
            fAgentWinItem->SetName (outNick.String());      
          msg->ReplaceString ("nick", outNick.String());
        }
        else
        {
          BString outNick (fChatee.String());
          outNick.RemoveFirst (" [DCC]");
          msg->AddString ("nick", outNick.String());
        }

        if (IsHidden())
          UpdateStatus (WIN_NICK_BIT);

        // Send the rest of processing up the chain
        ClientAgent::MessageReceived (msg);
      }
      break;
      
    case M_MSG_WHOIS:
      {
        BMessage dataSend (M_SERVER_SEND);

        AddSend (&dataSend, "WHOIS ");
        AddSend (&dataSend, fChatee.String());
        AddSend (&dataSend, " ");
        AddSend (&dataSend, fChatee.String());
        AddSend (&dataSend, endl);      
      }

    case M_CHANGE_NICK:
      {
        const char *oldNick, *newNick;

        msg->FindString ("oldnick", &oldNick);
        msg->FindString ("newnick", &newNick);

        if (fChatee.ICompare (oldNick) == 0)
        {
          const char *address;
          const char *ident;

          msg->FindString ("address", &address);
          msg->FindString ("ident", &ident);

          BString oldId (fId);
          fChatee = fId = newNick;

          if (fDChat)
            fId.Append(" [DCC]");
        
          fAgentWinItem->SetName (fId.String());

                 
          ClientAgent::MessageReceived (msg);
        }
      
        else if (fMyNick.ICompare (oldNick) == 0)
        {
          if (!IsHidden())
            vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);
          ClientAgent::MessageReceived (msg);
        }
      }
      break;

    case M_CLIENT_QUIT:
      {
        ClientAgent::MessageReceived(msg);
        BMessage deathchant (M_OBITUARY);
        deathchant.AddPointer ("agent", this);
        deathchant.AddPointer ("item", fAgentWinItem);
        vision_app->pClientWin()->PostMessage (&deathchant);
  
        deathchant.what = M_CLIENT_SHUTDOWN;
        fSMsgr.SendMessage (&deathchant);
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
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, fServerName.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String(), true);
      }        
      break;
    
    default:
      ClientAgent::MessageReceived (msg);
  }
}

void
MessageAgent::ActionMessage (const char *msg, const char *nick)
{
  if (!fDChat)
    ClientAgent::ActionMessage (msg, nick);
  else if (fDConnected)
  {
    BString outTemp;
    outTemp = "\1ACTION ";
    outTemp += msg;
    outTemp += "\1";
    outTemp += "\n";
#ifdef NETSERVER_BUILD
    fLocker->Lock();
#endif
    if (send(fAcceptSocket, outTemp.String(), outTemp.Length(), 0) < 0)
    {
      fDConnected = false;
      Display (S_DCC_CHAT_TERM);
#ifdef NETSERVER_BUILD
      fLocker->Unlock();
#endif
      return;
    }
#ifdef NETSERVER_BUILD
    fLocker->Unlock();
#endif
    outTemp.RemoveLast ("\n");
    ChannelMessage (outTemp.String(), nick);
  }
  
}
void
MessageAgent::Parser (const char *buffer)
{
  if (!fDChat)
  {
    BMessage dataSend (M_SERVER_SEND);

    AddSend (&dataSend, "PRIVMSG ");
    AddSend (&dataSend, fChatee);
    AddSend (&dataSend, " :");
    AddSend (&dataSend, buffer);
    AddSend (&dataSend, endl);
  }
  else if (fDConnected)
  {
    BString outTemp (buffer);

    outTemp << "\n";
#ifdef NETSERVER_BUILD
    fLocker->Lock();
#endif
    if (send(fAcceptSocket, outTemp.String(), outTemp.Length(), 0) < 0)
    {
      fDConnected = false;
      Display (S_DCC_CHAT_TERM);
#ifdef NETSERVER_BUILD
      fLocker->Unlock();
#endif
      return;
    }
#ifdef NETSERVER_BUILD
    fLocker->Unlock();
#endif
  }
  else
    return;

  Display ("<", C_MYNICK);
  Display (fMyNick.String(), C_NICKDISPLAY);
  Display ("> ", C_MYNICK);

  BString sBuffer (buffer);
  Display (sBuffer.String());
  

  Display ("\n");
}

void
MessageAgent::DroppedFile (BMessage *)
{
  // TODO: implement this when DCC's ready
}

void
MessageAgent::TabExpansion (void)
{
  int32 start,
        finish;

  fInput->TextView()->GetSelection (&start, &finish);

  if (fInput->TextView()->TextLength()
  &&  start == finish
  &&  start == fInput->TextView()->TextLength())
  {
    const char *fInputText (
                  fInput->TextView()->Text()
                  + fInput->TextView()->TextLength());
    const char *place (fInputText);


    while (place > fInput->TextView()->Text())
    {
      if (*(place - 1) == '\x20')
        break;
      --place;
    }

    BString insertion;

    if (!fId.ICompare (place, strlen (place)))
    {
      insertion = fId;
      insertion.RemoveLast(" [DCC]");
    }
    else if (!fMyNick.ICompare (place, strlen (place)))
      insertion = fMyNick;

    if (insertion.Length())
    {
      fInput->TextView()->Delete (
        place - fInput->TextView()->Text(),
        fInput->TextView()->TextLength());

      fInput->TextView()->Insert (insertion.String());
      fInput->TextView()->Select (
        fInput->TextView()->TextLength(),
        fInput->TextView()->TextLength());
    }
  }
}
