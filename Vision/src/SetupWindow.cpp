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
 *                 Todd Lair
 *                 Rene Gollent
 */

#include <Button.h>
#include <Bitmap.h>
#include <View.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <TranslationUtils.h>

#include "ClickView.h"
#include "ClientWindow.h"
#include "SetupWindow.h"
#include "Vision.h"
#include "NetworkMenu.h"

#include <stdio.h>

SetupWindow::SetupWindow (void)
  : BWindow (
      BRect (188.0, 88.0, 535.0, 290.0),
      "Setup Window",
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  bgView = new BView (Bounds(), "background", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);
  AddChild (bgView);
  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BBitmap *bmp (NULL);
  if ((bmp = BTranslationUtils::GetBitmap ('bits', "vision-logo")) != 0)
  {
    BRect bounds (Bounds());
    bounds.left = (bounds.Width() - bmp->Bounds().Width()) / 2.0;
    bounds.right = (bounds.left + bmp->Bounds().Width());
    bounds.top = 16.0;
    bounds.bottom = bounds.top + bmp->Bounds().Height(); 
    ClickView *logo = new ClickView (
                    bounds,
                    "image",
                    B_FOLLOW_LEFT | B_FOLLOW_TOP,
                    B_WILL_DRAW,
                    "http://vision.sourceforge.net");
    bgView->AddChild (logo);
    logo->SetViewBitmap (bmp);
    delete bmp;
  }
  
  connectButton = new BButton (BRect (0,0,0,0), "connect", "Connect",
    new BMessage (M_CONNECT_NETWORK));
  connectButton->ResizeToPreferred();
  netPrefsButton = new BButton (BRect (0,0,0,0), "netprefs", "Network Setup"B_UTF8_ELLIPSIS,
    new BMessage (M_NETWORK_SHOW));
  netPrefsButton->ResizeToPreferred();
  netPrefsButton->SetTarget (vision_app);
  prefsButton = new BButton (BRect (0,0,0,0), "prefs", "Preferences"B_UTF8_ELLIPSIS,
    new BMessage (M_PREFS_SHOW));
  prefsButton->ResizeToPreferred();
  prefsButton->SetTarget (vision_app);
  prefsButton->MoveTo (bgView->Bounds().right - (prefsButton->Bounds().Width() + 10),
    bgView->Bounds().bottom - (prefsButton->Bounds().Height() + 5));
  bgView->AddChild (prefsButton);
  netPrefsButton->MoveTo (prefsButton->Frame().left - (netPrefsButton->Bounds().Width() + 5),
    prefsButton->Frame().top);
  bgView->AddChild (netPrefsButton);
  connectButton->MoveTo (netPrefsButton->Frame().left - (connectButton->Bounds().Width() + 15),
    prefsButton->Frame().top);
  bgView->AddChild (connectButton);
  BuildNetworkMenu();
  connectButton->SetEnabled (false);
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
SetupWindow::BuildNetworkMenu (void)
{
  BMenu *netMenu (new NetworkMenu ("Choose Network", M_SETUP_CHOOSE_NETWORK, BMessenger(this)));
  netMenu->SetLabelFromMarked (true);
  netList = new BMenuField (BRect (0,0,0,0), "Network List", "Network: ", netMenu);
  netList->ResizeToPreferred();
  netList->SetDivider (be_plain_font->StringWidth ("Network: ") + 5);
  bgView->AddChild (netList);
  netList->MoveTo (10, connectButton->Frame().top - (netList->Bounds().Height() + 20));
}

void
SetupWindow::MessageReceived (BMessage *msg)
{
  switch(msg->what)
  {
    case M_SETUP_CHOOSE_NETWORK:
      {
        BMenuItem *item (NULL);
        msg->FindPointer ("source", reinterpret_cast<void **>(&item));
        if (item && vision_app->CheckNetworkValid (item->Label()))
          connectButton->SetEnabled (true);
        else
          connectButton->SetEnabled (false);
      }
      break;
    
    case M_CONNECT_NETWORK:
      {
        BMessage connMsg (M_CONNECT_NETWORK);
        connMsg.AddString ("network", netList->MenuItem()->Label());
        vision_app->PostMessage (&connMsg);
      }
      break;
      
    default:
      BWindow::MessageReceived (msg);
  }
}
