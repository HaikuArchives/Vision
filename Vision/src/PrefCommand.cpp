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

#include "PrefCommand.h"
#include "Vision.h"
#include "VTextControl.h"

#include <ScrollView.h>

static const char *CommandControlLabels[] =
{
	"Quit:",
	"Kick:",
	"Ignore:",
	"Unignore:",
	"Away:",
	"Back:",
	"Uptime:",
	0
};


CommandPrefsView::CommandPrefsView (BRect frame)
  : BView (frame, "Command prefs", B_FOLLOW_NONE, B_WILL_DRAW)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BRect bounds (Bounds());
  int32 i (0);
  bounds.left += 3;
  bounds.right -= B_V_SCROLL_BAR_WIDTH + 3;
  bounds.top += 3;
  bounds.bottom -= 5;
  float label_width (0.0);

  for (i = 0; CommandControlLabels[i]; ++i)
    if (StringWidth (CommandControlLabels[i]) > label_width)
      label_width = StringWidth (CommandControlLabels[i]);
  
  BView *bgView (new BView (bounds, "", B_FOLLOW_NONE, B_WILL_DRAW));
  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  commands = new VTextControl * [MAX_COMMANDS];

  for (i = 0; i < MAX_COMMANDS; ++i)
  {
    commands[i] = new VTextControl (
      BRect (5, be_plain_font->Size() + ((1.5 * i) * 1.5 * be_plain_font->Size()), 5 + bounds.right - be_plain_font->StringWidth("gP"),
      be_plain_font->Size() + (1.5 * (i + 1) * 1.5 * be_plain_font->Size())),
      "commands",
      CommandControlLabels[i],
      vision_app->GetCommand (i).String(),
      0);

    commands[i]->SetDivider (label_width + 5);

    BMessage *msg (new BMessage (M_COMMAND_MODIFIED));

    msg->AddInt32 ("which", i);
    commands[i]->SetModificationMessage (msg);
    bgView->AddChild (commands[i]);
  }
  BScrollView *scroller (new BScrollView("command scroller", bgView, B_FOLLOW_LEFT | B_FOLLOW_TOP,
    0, false, true));
  BScrollBar *bar (scroller->ScrollBar(B_VERTICAL));
 
  bar->SetRange (0.0, 0.5 * commands[MAX_COMMANDS-1]->Frame().bottom - 15);
  bar->SetProportion (115.0 / 181.0);

  AddChild (scroller);
}

CommandPrefsView::~CommandPrefsView (void)
{
  delete [] commands;
}

void
CommandPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow ();
  for (int32 i = 0; i < MAX_COMMANDS; i++)
    commands[i]->SetTarget (this);
}

void
CommandPrefsView::AllAttached (void)
{
  BView::AllAttached ();
}

void
CommandPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_COMMAND_MODIFIED:
    {
      int32 which;

      msg->FindInt32 ("which", &which);
      vision_app->SetCommand (
        which,
        commands[which]->TextView()->Text());
    }
    break;

    default:
      BView::MessageReceived (msg);
  }
}
