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
 * Contributor(s): Rene Gollent
 *                 Wade Majors
 *                 Todd Lair
 */

#include <StatusBar.h>
#include <StringView.h>
#include <Path.h>
#include <Mime.h>
#include <Window.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "Vision.h"
#include "ServerAgent.h"
#include "DCCConnect.h"
#include "PlayButton.h"

DCCConnect::DCCConnect (
  const char *n,
  const char *fn,
  const char *sz,
  const char *i,
  const char *p,
  const BMessenger &c)
    : BView (
        BRect (0.0, 0.0, 275.0, 150.0),
        "dcc connect",
        B_FOLLOW_LEFT | B_FOLLOW_TOP,
        B_WILL_DRAW),
        fCaller (c),
        fNick (n),
        fFileName (fn),
        fSize (sz),
        fIp (i),
        fPort (p),
        fTotalTransferred (0),
        fFinalRateAverage (0),
        fTid (-1),
        fIsStopped (false)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));

  char trail[128];
  sprintf (trail, " / %.1fk", atol (fSize.String()) / 1024.0);

  fBar = new BStatusBar (
    BRect (10, 10, Bounds().right - 30, Bounds().bottom - 10),
    "progress",
    S_DCC_SPEED,
    trail);
  fBar->SetMaxValue (atol (fSize.String()));
  fBar->SetBarHeight (8.0);
  AddChild (fBar);

  fLabel = new BStringView (
    BRect (10, 10, Bounds().right - 10, Bounds().bottom - 10),
    "label",
    "",
    B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
  AddChild (fLabel);

  fStop = new StopButton (
    BPoint (fBar->Frame().right + 15, fBar->Frame().bottom - 18),
    new BMessage (M_DCC_STOP_BUTTON));
  
  AddChild (fStop);
}

DCCConnect::~DCCConnect (void)
{
}

void
DCCConnect::AttachedToWindow (void)
{
  fStop->SetTarget (this);
}

void
DCCConnect::AllAttached (void)
{
  fStop->MoveTo (
    fBar->Frame().right + 15, fBar->Frame().bottom - 18);
  fLabel->MoveTo (
    fLabel->Frame().left,
    fBar->Frame().bottom + 1);

  float width, height;
  fLabel->GetPreferredSize (&width, &height);
  fLabel->ResizeTo (fLabel->Frame().Width(), height);
  ResizeTo (fStop->Frame().right + 5, fLabel->Frame().bottom + 5.0);
}

void
DCCConnect::DetachedFromWindow (void)
{
}

void
DCCConnect::Draw (BRect)
{
  BView *top (Window()->ChildAt (0));

  if (this != top)
  {
    BeginLineArray (2);
    AddLine (
      Bounds().LeftTop(),
      Bounds().RightTop(),
      tint_color (ViewColor(), B_DARKEN_2_TINT));

    AddLine (
      Bounds().LeftTop() + BPoint (0.0, 1.0),
      Bounds().RightTop() + BPoint (0.0, 1.0),
      tint_color (ViewColor(), B_LIGHTEN_MAX_TINT));
    EndLineArray();
  }
}

void
DCCConnect::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_DCC_STOP_BUTTON:
      {
        Stopped();
      }
      break;

    case M_DCC_UPDATE_STATUS:
      {
        fLabel->SetText (msg->FindString ("text"));
      }
      break;
			
    case M_DCC_GET_CONNECT_DATA:
      {
        BMessage reply;
        reply.AddString ("port", fPort.String());
        reply.AddString ("ip", fIp.String());
        reply.AddString ("name", fFileName.String());
        reply.AddString ("nick", fNick.String());
        reply.AddString ("size", fSize.String());
        DCCReceive *recview (dynamic_cast<DCCReceive *>(this));
        if (recview != NULL)
          reply.AddBool ("resume", recview->fResume);
        DCCSend *sendview (dynamic_cast<DCCSend *>(this));
        if (sendview != NULL)
          reply.AddMessenger ("caller", sendview->fCaller);
        msg->SendReply(&reply);
      }
      break;
		
    case M_DCC_GET_RESUME_POS:
      {
        BMessage reply;
        DCCSend *sendview (dynamic_cast<DCCSend *>(this));
        if (sendview != NULL)
          reply.AddInt32 ("pos", sendview->fPos);
        msg->SendReply(&reply);
      }
      break;
		
    case M_DCC_UPDATE_TRANSFERRED:
      {
        fTotalTransferred = msg->FindInt32 ("transferred");
      }
      break;

    case M_DCC_UPDATE_AVERAGE:
      {
        fFinalRateAverage = msg->FindInt32 ("average");
      }
      break;

    default:
      BView::MessageReceived (msg);
  }
}

void
DCCConnect::Stopped (void)
{
  if (fTotalTransferred > 0)
  {
    BMessage xfermsg (M_DCC_COMPLETE);
    xfermsg.AddString("nick", fNick.String());
    xfermsg.AddString("file", fFileName.String());
    xfermsg.AddString("size", fSize.String());
    xfermsg.AddInt32("transferred", fTotalTransferred);
    xfermsg.AddInt32("transferRate", fFinalRateAverage);

    DCCReceive *recview = dynamic_cast<DCCReceive *>(this);
    if (recview)
    {
      xfermsg.AddString("type", "RECV");
    }
    else
    {
      xfermsg.AddString("type", "SEND");
    }
    fCaller.SendMessage(&xfermsg);
  }

  BMessage msg (M_DCC_FINISH);

  msg.AddPointer ("source", this);
  msg.AddBool ("stopped", true);
  Window()->PostMessage (&msg);
}

void
DCCConnect::Lock (void)
{
}

void
DCCConnect::Unlock (void)
{
}

void
DCCConnect::UpdateBar (const BMessenger &msgr, int readSize, int cps, uint32 size, bool update)
{
  BMessage msg (B_UPDATE_STATUS_BAR);

  if (update)
  {
    char text[128];

    sprintf (text, "%.1f", size / 1024.0);
    msg.AddString ("trailing_text", text);

    sprintf (text, "%d", cps);
    msg.AddString ("text", text);
  }

  msg.AddFloat ("delta", readSize);
  BLooper *looper (NULL);
  DCCConnect *transferView ((DCCConnect *)msgr.Target(&looper));
  if ((looper != NULL) && (transferView != NULL))
    looper->PostMessage (&msg, transferView->fBar);
}

void
DCCConnect::UpdateStatus (const BMessenger &msgr, const char *text)
{
  BMessage msg (M_DCC_UPDATE_STATUS);

  msg.AddString ("text", text);
  msgr.SendMessage (&msg);
}

DCCReceive::DCCReceive (
  const char *n,
  const char *fn,
  const char *sz,
  const char *i,
  const char *p,
  const BMessenger &c,
  bool cont)
    : DCCConnect (n, fn, sz, i, p, c),
      fResume (cont)
{
}

DCCReceive::~DCCReceive (void)
{
}

void
DCCReceive::AttachedToWindow (void)
{
  DCCConnect::AttachedToWindow();

  fTid = spawn_thread (
    Transfer,
    "DCC Receive",
    B_NORMAL_PRIORITY,
    this);
  resume_thread (fTid);
}

int32
DCCReceive::Transfer (void *arg)
{
  BMessenger msgr (reinterpret_cast<DCCReceive *>(arg));
  struct sockaddr_in address;
  BLooper *looper (NULL);
	
  int32 dccSock (-1);

  if ((dccSock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
  {
    UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
    return B_ERROR;
  }

  BMessage reply;

  if (msgr.SendMessage (M_DCC_GET_CONNECT_DATA, &reply) == B_ERROR)
    return B_ERROR;

  memset (&address, 0, sizeof(sockaddr_in));
  address.sin_family = AF_INET;
  address.sin_port   = htons (atoi (reply.FindString ("port")));
  address.sin_addr.s_addr = htonl(strtoul (reply.FindString("ip"), 0, 10));

  UpdateStatus (msgr, S_DCC_CONNECT_TO_SENDER);
  if (connect (dccSock, (sockaddr *)&address, sizeof (address)) < 0)
  {
    UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
    close (dccSock);
    return B_ERROR;
  }

  BPath path (reply.FindString("name"));
  BString buffer;
  off_t file_size (0);

  buffer << S_DCC_RECV1
    << path.Leaf()
    << S_DCC_RECV2
    << reply.FindString ("nick")
    << ".";

  UpdateStatus (msgr, buffer.String());

  BFile file;

  if (msgr.IsValid())
  {
    if (reply.FindBool ("resume"))
    {
      if (file.SetTo (
        reply.FindString("name"),
        B_WRITE_ONLY | B_OPEN_AT_END) == B_NO_ERROR
        &&  file.GetSize (&file_size) == B_NO_ERROR
        &&  file_size > 0LL)
          UpdateBar (msgr, file_size, 0, 0, true);
      else
        file_size = 0LL;
    }
    else
    {
      file.SetTo (
        reply.FindString("name"),
        B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
    }
  }
	
  uint32 bytes_received (file_size);
  uint32 size (atol (reply.FindString("size")));
  uint32 cps (0);

  if (file.InitCheck() == B_NO_ERROR)
  {
    bigtime_t last (system_time()), now;
    char inBuffer[8196];		
    bigtime_t start = system_time();

    while ((msgr.Target(&looper) != NULL) && bytes_received < size)
    {
      int readSize;
      if ((readSize = recv (dccSock, inBuffer, 8196, 0)) < 0)
        break;

      file.Write (inBuffer, readSize);
      bytes_received += readSize;
      BMessage msg (M_DCC_UPDATE_TRANSFERRED);
      msg.AddInt32 ("transferred", bytes_received);
      msgr.SendMessage (&msg);

      uint32 feed_back (htonl (bytes_received));
      send (dccSock, &feed_back, sizeof (uint32), 0);

      now = system_time();
      bool hit (false);

      if (now - last > 500000)
      {
        cps = (int)ceil ((bytes_received - file_size) / ((now - start) / 1000000.0));
        BMessage updmsg (M_DCC_UPDATE_AVERAGE);
        updmsg.AddInt32 ("average", cps);
        msgr.SendMessage (&updmsg);
        last = now;
        hit = true;
      }
      
      DCCConnect::UpdateBar (msgr, readSize, cps, bytes_received, hit);
    }
  }

  if (msgr.IsValid())
  {
    BMessage msg (M_DCC_STOP_BUTTON);
    msgr.SendMessage (&msg);
  }

  if (dccSock > 0)
  {
    close (dccSock);
  }

  if (file.InitCheck() == B_OK)
  {
    file.Unset();
    update_mime_info (reply.FindString("name"), 0, 0, 1);
  }
  return 0;
}

DCCSend::DCCSend (
  const char *n,
  const char *fn,
  const char *sz,
  const BMessenger &c)
    : DCCConnect (n, fn, sz, "", "", c),
      fPos (0LL)
{
  int32 dccPort (atoi (vision_app->GetString ("dccMinPort")));
  int32 diff (atoi (vision_app->GetString ("dccMaxPort")) - dccPort);
  if (diff > 0)
    dccPort += rand() % diff;
      
  fPort << dccPort;
}

DCCSend::~DCCSend (void)
{
}

void
DCCSend::AttachedToWindow (void)
{
  DCCConnect::AttachedToWindow();
  
  fTid = spawn_thread (
    Transfer,
    "DCC Send",
    B_NORMAL_PRIORITY,
    this);
  resume_thread (fTid);
}

int32
DCCSend::Transfer (void *arg)
{
  BMessenger msgr (reinterpret_cast<DCCSend *>(arg));
  BMessage reply, ipdata;
  BLooper *looper (NULL);

  if (msgr.IsValid())
    msgr.SendMessage (M_DCC_GET_CONNECT_DATA, &reply);

  BMessenger callmsgr;
  reply.FindMessenger ("caller", &callmsgr);

  callmsgr.SendMessage (M_GET_IP, &ipdata);    

  BPath path (reply.FindString ("name"));
  BString fileName, status;
  struct sockaddr_in address;
  struct in_addr sendaddr;
  memset (&sendaddr, 0, sizeof (struct in_addr));
  int sd, dccSock (-1);

  fileName.Append (path.Leaf());
  fileName.ReplaceAll (" ", "_");

  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
  {
    UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
    return 0;
  }

  memset (&address, 0, sizeof (struct sockaddr_in));
  address.sin_family      = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port        = htons (atoi (reply.FindString("port")));

  int sin_size;
  sin_size = (sizeof (struct sockaddr_in));

  UpdateStatus (msgr, S_DCC_LOCK_ACQUIRE B_UTF8_ELLIPSIS);

  vision_app->AcquireDCCLock();

  if (!msgr.IsValid() || bind (sd, (sockaddr *)&address, sin_size) < 0)
  {
    UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
    vision_app->ReleaseDCCLock();

    close (sd);
    return 0;
  }
  
  UpdateStatus (msgr, S_DCC_ACK_WAIT);

  sendaddr.s_addr = inet_addr (ipdata.FindString ("ip"));

  if (msgr.IsValid())
  {
    status = "PRIVMSG ";
    status << reply.FindString ("nick") 
      << " :\1DCC SEND "
      << fileName
      << " "
      << htonl (sendaddr.s_addr)
      << " "
      << reply.FindString ("port")
      << " "
      << reply.FindString ("size")
      << "\1";

    BMessage msg (M_SERVER_SEND);
    msg.AddString ("data", status.String());
    if (callmsgr.IsValid())
      callmsgr.SendMessage (&msg);
      
    UpdateStatus (msgr, S_DCC_LISTEN_CALL);
    if (listen (sd, 1) < 0)
    {
      UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
      vision_app->ReleaseDCCLock();
      close (sd);
      return 0;
    }
  }

  struct timeval t;
  t.tv_sec  = 2;
  t.tv_usec = 0;

  uint32 try_count (0);

  while (msgr.Target(&looper) != NULL)
  {
    fd_set rset;
      
    FD_ZERO (&rset);
    FD_SET (sd, &rset);

    if (select (sd + 1, &rset, 0, 0, &t) < 0)
    {
      UpdateStatus (msgr, S_DCC_ESTABLISH_ERROR);
      vision_app->ReleaseDCCLock();
      close (sd);
      return 0;
    }

    if (FD_ISSET (sd, &rset))
    {
      dccSock = accept (sd, (sockaddr *)&address, (socklen_t *)&sin_size);
      UpdateStatus (msgr, S_DCC_ESTABLISH_SUCCEEDED);
      break;
    }
      
    ++try_count;
    status = S_DCC_WAIT_FOR_CONNECTION;
    status << try_count << ".";
    UpdateStatus (msgr, status.String());
  }
  
  vision_app->ReleaseDCCLock();

  char set[4];
  memset(set, 1, sizeof(set));
  close (sd);
  BFile file;

  file.SetTo(reply.FindString ("name"), B_READ_ONLY);
  int32 bytes_sent (0L),
    seekpos (0L);

  BMessage resumeData;
  msgr.SendMessage (M_DCC_GET_RESUME_POS, &resumeData);

  if (resumeData.HasInt32 ("pos"))
  {
    resumeData.FindInt32 ("pos", &seekpos);
    file.Seek (seekpos, SEEK_SET);
    UpdateBar (msgr, seekpos, 0, 0, true);
    bytes_sent = seekpos;
  }

  status = S_DCC_SEND1;
  status << path.Leaf()
    << S_DCC_SEND2
    << reply.FindString ("nick")
    << ".";
  UpdateStatus (msgr, status.String());

  int cps (0);

  if (file.InitCheck() == B_NO_ERROR)
  {
    bigtime_t last (system_time()), now;
    const uint32 DCC_BLOCK_SIZE (atoi(vision_app->GetString ("dccBlockSize")));
#ifdef __INTEL__
    char buffer[DCC_BLOCK_SIZE];
#else
	char *buffer = new char[DCC_BLOCK_SIZE];
#endif
    int period (0);
    ssize_t count (0);
    bigtime_t start = system_time();

    while ((msgr.Target(&looper) != NULL)
      && (count = file.Read (buffer, DCC_BLOCK_SIZE - 1)) > 0)
    {
      int sent;
          
      if ((sent = send (dccSock, buffer, count, 0)) < count)
      {
        UpdateStatus (msgr, S_DCC_WRITE_ERROR);
        break;
      }
      
      uint32 confirm (0),
             newSize (bytes_sent + count);
      fd_set rset, eset;
      FD_ZERO (&rset);
      FD_ZERO (&eset);
      FD_SET (dccSock, &rset);
      t.tv_sec = 0;
      t.tv_usec = 10;

      while ((confirm < newSize)
          && (recv(dccSock, &confirm, sizeof (confirm), 0) > 0))
      {
        confirm = ntohl(confirm);
        bytes_sent = confirm;
      }
      
      BMessage msg (M_DCC_UPDATE_TRANSFERRED);
      msg.AddInt32 ("transferred", bytes_sent);
      msgr.SendMessage (&msg);

      now = system_time();
      period += sent;
          
      bool hit (false);
          
      if (now - last > 500000)
      {
        cps = (int) ceil ((bytes_sent - seekpos) / ((now - start) / 1000000.0));
        BMessage updmsg (M_DCC_UPDATE_AVERAGE);
        updmsg.AddInt32 ("average", cps);
        msgr.SendMessage (&updmsg);
        last = now;
        period = 0;
        hit = true;
      }
      UpdateBar (msgr, sent, cps, bytes_sent, hit);
    }
#ifndef __INTEL__
    delete [] buffer;
#endif
  }
  if (msgr.IsValid())
  {
    BMessage msg (M_DCC_STOP_BUTTON);
    msgr.SendMessage (&msg);
  }

  if (dccSock > 0)
  {
    close (dccSock);
  }

  if (file.InitCheck() == B_OK)
    file.Unset();
  
  return 0;
}

bool
DCCSend::IsMatch (const char *n, const char *p) const
{
  return fNick == n && fPort == p;
}

void
DCCSend::SetResume (off_t p)
{
  fPos = p;
}
