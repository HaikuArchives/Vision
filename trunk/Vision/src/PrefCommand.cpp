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
	S_PREFCOMMAND_QUIT,
	S_PREFCOMMAND_KICK,
	S_PREFCOMMAND_IGNORE,
	S_PREFCOMMAND_UNIGNORE,
	S_PREFCOMMAND_AWAY,
	S_PREFCOMMAND_BACK,
	S_PREFCOMMAND_UPTIME,
	0
};


CommandPrefsView::CommandPrefsView (BRect frame)
  : BView (frame, "Command prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
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
  
  BView *bgView (new BView (bounds, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW));
  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  fCommands = new VTextControl * [MAX_COMMANDS];

  for (i = 0; i < MAX_COMMANDS; ++i)
  {
    fCommands[i] = new VTextControl (
      BRect (5, be_plain_font->Size() + ((1.5 * i) * 1.5 * be_plain_font->Size()), 5 + bounds.right - be_plain_font->StringWidth("gP"),
      be_plain_font->Size() + (1.5 * (i + 1) * 1.5 * be_plain_font->Size())),
      "commands",
      CommandControlLabels[i],
      vision_app->GetCommand (i).String(),
      NULL,
      B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);

    fCommands[i]->SetDivider (label_width + 5);

    BMessage *msg (new BMessage (M_COMMAND_MODIFIED));

    msg->AddInt32 ("which", i);
    fCommands[i]->SetModificationMessage (msg);
    bgView->AddChild (fCommands[i]);
  }
  fScroller = new BScrollView("command scroller", bgView, B_FOLLOW_ALL_SIDES,
    0, false, true);
  BScrollBar *bar (fScroller->ScrollBar(B_VERTICAL));
 
  fMaxheight = bgView->Bounds().Height();
  fProportionheight = fCommands[MAX_COMMANDS-1]->Frame().bottom;
  bar->SetRange (0.0, fMaxheight);
  bar->SetProportion (fScroller->Bounds().Height() / fProportionheight);

  AddChild (fScroller);
}

CommandPrefsView::~CommandPrefsView (void)
{
  delete [] fCommands;
}

void
CommandPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow ();
  for (int32 i = 0; i < MAX_COMMANDS; i++)
    fCommands[i]->SetTarget (this);
}

void
CommandPrefsView::AllAttached (void)
{
  BView::AllAttached ();
}

void
CommandPrefsView::FrameResized (float width, float height)
{
  BView::FrameResized (width, height);
  BScrollBar *bar(fScroller->ScrollBar(B_VERTICAL));
  if (!bar)
    return;
  float min, max, scrollheight (fScroller->Bounds().Height());
  
  bar->GetRange (&min, &max);
  if (scrollheight < fProportionheight)
  {
    if (max != fMaxheight)
      bar->SetRange (0.0, scrollheight);
    bar->SetProportion (scrollheight / fProportionheight);
  }
  else
  {
    bar->SetProportion (1.0);
    bar->SetRange (0.0, 0.0);
  }
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
        fCommands[which]->TextView()->Text());
    }
    break;

    default:
      BView::MessageReceived (msg);
  }
}
