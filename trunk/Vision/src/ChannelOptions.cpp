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
 */

#include "ChannelAgent.h"
#include "ChannelOptions.h"

#include <stdio.h>

ChannelOptions::ChannelOptions (BString chan_name_, ChannelAgent *parent_)
  : BWindow (
      BRect (188.0, 88.0, 600.0, 390.0),
      "",
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
  parent (parent_),
  chan_name (chan_name_)
{
  Init();
}


ChannelOptions::~ChannelOptions (void)
{
  //
}

bool
ChannelOptions::QuitRequested (void)
{
  parent->msgr.SendMessage (M_CHANNEL_OPTIONS_CLOSE);
  return true;  
}

void
ChannelOptions::Init (void)
{
  BString temp (" Options");
  temp.Prepend (chan_name);
  SetTitle (temp.String());
  
  bgView = new BView (Bounds(),
                      "Background",
                      B_FOLLOW_ALL_SIDES,
                      B_WILL_DRAW);

  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (bgView);
}

