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
 */

#include <View.h>
#include <Messenger.h>

#include "ClientWindow.h"
#include "NetworkPrefsView.h"
#include "NetPrefsServerView.h"
#include "NetworkWindow.h"
#include "Vision.h"

#include <stdio.h>

NetworkWindow::NetworkWindow (void)
  : BWindow (
      BRect (50, 50, 500, 350),
      S_NETWORK_WINDOW_TITLE,
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  AddChild (new NetworkPrefsView (Bounds(), "network"));
  BRect netFrame (0,0,0,0);
  netFrame = vision_app->GetRect ("NetPrefWinRect");
  if (netFrame.Width() != 0.0)
    MoveTo(netFrame.left, netFrame.top);
}


NetworkWindow::~NetworkWindow (void)
{
  //
}

bool
NetworkWindow::QuitRequested (void)
{
  vision_app->SetRect ("NetPrefWinRect", Frame());
  be_app_messenger.SendMessage (M_NETWORK_CLOSE);
  return true;  
}

NetPrefServerWindow::NetPrefServerWindow (BHandler *target)
  : BWindow (
      BRect (50, 50, 350, 250),
      S_SERVERPREFS_TITLE,
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
	AddChild ((serverView = new NetPrefsServerView (Bounds(), "server settings", BMessenger(target))));
}


NetPrefServerWindow::~NetPrefServerWindow (void)
{
  //
}

bool
NetPrefServerWindow::QuitRequested (void)
{
  return true;
}

void
NetPrefServerWindow::SetNetworkData (BMessage *msg)
{
  serverView->SetNetworkData (msg);
}
