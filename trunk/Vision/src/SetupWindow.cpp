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
 *                 Todd Lair
 *                 Rene Gollent
 */

#ifdef GNOME_BUILD
#  include "gnome/View.h"
#elif BEOS_BUILD
#  include <View.h>
#endif

#include "ClientWindow.h"
#include "SetupWindow.h"
#include "Vision.h"

#include <stdio.h>

SetupWindow::SetupWindow (bool launching)
  : BWindow (
      BRect (188.0, 88.0, 485.0, 390.0),
      "SetupWindow",
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  if (launching)
    InitServerStartup();
  else
    Init();
}


SetupWindow::~SetupWindow (void)
{
  //
}

bool
SetupWindow::QuitRequested (void)
{
  be_app_messenger.SendMessage (M_SETUP_CLOSE);
  return true;  
}

void
SetupWindow::Init (void)
{
  bgView = new BView (Bounds(),
                      "Background",
                      B_FOLLOW_ALL_SIDES,
                      B_WILL_DRAW);

  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (bgView);
}

void
SetupWindow::InitServerStartup (void)
{
  // iterates through servers that have connect on startup enabled
  // and adds them to ClientWindow
  
  
  // temp
  BMessage newserver (M_MAKE_NEW_SERVER);
  newserver.AddString ("hostname", "irc.exodusirc.net");
  newserver.AddString ("port", "6667");
  newserver.AddString ("autoexec", "");
  newserver.AddBool   ("enidentd", true);
  vision_app->pClientWin()->PostMessage (&newserver);
  
  QuitRequested();
}
