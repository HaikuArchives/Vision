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
 *                 Todd Lair
 */

// TODO: Color Schemes/Themes


#include "PrefColor.h"
#include "ColorSelector.h"
#include "Vision.h"
#include <Box.h>
#include <Button.h>
#include <MenuField.h>
#include <StringView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Point.h>

#include <stdio.h>

const uint32 M_REVERT_SELECTION = 'mcrs';
// data structures for font prefs

struct FontStat
{
	font_family				family;
	int32						style_count;
	font_style				*styles;
};

static const char *ColorLabels[] =
{
	"Text",
	"Background",
	"URL",
	"Server text",
	"Notice",
	"Action",
	"Quit",
	"Error",
	"Nickname edges",
	"User nickname edges",
	"Nickname text",
	"Join",
	"Kick",
	"Whois",
	"Names (Normal)",
	"Names (Op)",
	"Names (Helper)",
	"Names (Voice)",
	"Names selection",
	"Names Background",
	"CTCP Request",
	"CTCP Reply",
	"Ignore",
	"Input text",
	"Input background",
	"Winlist normal status",
	"Winlist text status",
	"Winlist nick alert status",
	"Winlist selection status",
	"Winlist event status",
	"Winlist background",
	"Wallops",
	"Timestamp",
	"Timestamp background",
	"Selection",
	"mIRC White",
	"mIRC Black",
	"mIRC Dark Blue",
	"mIRC Green",
	"mIRC Red",
	"mIRC Brown",
	"mIRC Purple",
	"mIRC Orange",
	"mIRC Yellow",
	"mIRC Lime",
	"mIRC Teal",
	"mIRC Aqua",
	"mIRC Light Blue",
	"mIRC Pink",
	"mIRC Grey",
	"mIRC Silver"
};

ColorPrefsView::ColorPrefsView (BRect frame)
  : BView (frame, "Color Prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  for (int32 i = 0 ; i < MAX_COLORS ; i++)
    colors[i] = vision_app->GetColor (i);
  
  BMessage mycolors, labels;

  for (int32 i = 0 ; i < MAX_COLORS; i++)
  {
  	mycolors.AddData ("color", B_RGB_COLOR_TYPE, &colors[i], sizeof(rgb_color));
  	labels.AddString ("color", ColorLabels[i]);
  }

  selector = new ColorSelector (frame, "selector", NULL, mycolors, labels, new BMessage ('vtst'));
  selector->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  selector->ResizeToPreferred();
  revert = new BButton (BRect (0,0,0,0), "revert", "Revert", new BMessage (M_REVERT_SELECTION));
  revert->ResizeToPreferred();
  ResizeTo (selector->Bounds().Width() + 30, selector->Bounds().Height() + 30 + revert->Bounds().Height());
  AddChild (selector);
  AddChild (revert);
}

ColorPrefsView::~ColorPrefsView (void)
{
}

void
ColorPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow();
}

void
ColorPrefsView::AllAttached (void)
{
  BView::AllAttached();
  selector->ResizeToPreferred();
  selector->MoveTo ((Bounds().Width() - selector->Bounds().Width()) / 2 ,
    5);
  revert->MoveTo (selector->Frame().left, selector->Frame().bottom + 10);
  revert->SetTarget (this);
}

void
ColorPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_REVERT_SELECTION:
      printf("received revert message\n");
      selector->Revert();
      break;
  }
  BView::MessageReceived(msg);
}

