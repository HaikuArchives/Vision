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


// data structures for font prefs

struct FontStat
{
	font_family				family;
	int32						style_count;
	font_style				*styles;
};

static const char *ColorLabels[] =
{
	S_PREFCOLOR_TEXT,
	S_PREFCOLOR_BACKGROUND,
	S_PREFCOLOR_URL,
	S_PREFCOLOR_SERVERTEXT,
	S_PREFCOLOR_NOTICE,
	S_PREFCOLOR_ACTION,
	S_PREFCOLOR_QUIT,
	S_PREFCOLOR_ERROR,
	S_PREFCOLOR_NICK_EDGES,
	S_PREFCOLOR_UNICK_EDGES,
	S_PREFCOLOR_NICK_TEXT,
	S_PREFCOLOR_JOIN,
	S_PREFCOLOR_KICK,
	S_PREFCOLOR_WHOIS,
	S_PREFCOLOR_NAMES_NORM,
	S_PREFCOLOR_NAMES_OP,
	S_PREFCOLOR_NAMES_HELP,
	S_PREFCOLOR_NAMES_VOICE,
	S_PREFCOLOR_NAMES_SEL,
	S_PREFCOLOR_NAMES_BG,
	S_PREFCOLOR_CTCP_REQ,
	S_PREFCOLOR_CTCP_RPY,
	S_PREFCOLOR_IGNORE,
	S_PREFCOLOR_INPUT_TXT,
	S_PREFCOLOR_INPUT_BG,
	S_PREFCOLOR_WLIST_NORM,
	S_PREFCOLOR_WLIST_TXT,
	S_PREFCOLOR_WLIST_NICK,
	S_PREFCOLOR_WLIST_SEL,
	S_PREFCOLOR_WLIST_EVT,
	S_PREFCOLOR_WLIST_BG,
	S_PREFCOLOR_WALLOPS,
	S_PREFCOLOR_TIMESTAMP,
	S_PREFCOLOR_TIMESTAMP_BG,
	S_PREFCOLOR_SELECTION,
	S_PREFCOLOR_MIRCWHITE,
	S_PREFCOLOR_MIRCBLACK,
	S_PREFCOLOR_MIRCDBLUE,
	S_PREFCOLOR_MIRCGREEN,
	S_PREFCOLOR_MIRCRED,
	S_PREFCOLOR_MIRCBROWN,
	S_PREFCOLOR_MIRCPURPLE,
	S_PREFCOLOR_MIRCORANGE,
	S_PREFCOLOR_MIRCYELLOW,
	S_PREFCOLOR_MIRCLIME,
	S_PREFCOLOR_MIRCTEAL,
	S_PREFCOLOR_MIRCAQUA,
	S_PREFCOLOR_MIRCLBLUE,
	S_PREFCOLOR_MIRCPINK,
	S_PREFCOLOR_MIRCGREY,
	S_PREFCOLOR_MIRCSILVER
};

ColorPrefsView::ColorPrefsView (BRect frame)
  : BView (frame, "Color Prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
  int32 i (0);
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  for (i = 0 ; i < MAX_COLORS ; i++)
    fColors[i] = vision_app->GetColor (i);
  
  BMessage mycolors, labels;

  for (i = 0 ; i < MAX_COLORS; i++)
  {
  	mycolors.AddData ("color", B_RGB_COLOR_TYPE, &fColors[i], sizeof(rgb_color));
  	labels.AddString ("color", ColorLabels[i]);
  }

  fSelector = new ColorSelector (frame, "fSelector", NULL, mycolors, labels, new BMessage ('vtst'));
  fSelector->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  fSelector->ResizeToPreferred();
  fRevert = new BButton (BRect (0,0,0,0), "fRevert", S_PREFCOLOR_REVERT, new BMessage (M_REVERT_COLOR_SELECTIONS));
  fRevert->ResizeToPreferred();
  ResizeTo (fSelector->Bounds().Width() + 30, fSelector->Bounds().Height() + 30 + fRevert->Bounds().Height());
  AddChild (fSelector);
  AddChild (fRevert);
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
  fSelector->ResizeToPreferred();
  fSelector->MoveTo ((Bounds().Width() - fSelector->Bounds().Width()) / 2 ,
    5);
  fRevert->MoveTo (fSelector->Frame().left, fSelector->Frame().bottom + 10);
  fRevert->SetTarget (this);
}

void
ColorPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_REVERT_COLOR_SELECTIONS:
      fSelector->Revert();
      break;
  }
  BView::MessageReceived(msg);
}

