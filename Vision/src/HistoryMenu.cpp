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
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
 
#include <Window.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <String.h>

#include <stdio.h>

#include "VTextControl.h"
#include "VisionBase.h"
#include "HistoryMenu.h"

#define BACK_BUFFER_SIZE                20

HistoryMenu::HistoryMenu (BRect frame)
  : BView (
    frame,
    "History",
    B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT,
    B_WILL_DRAW),
  bufferFree (0),
  bufferPos (0),
  tracking (false)
{
  tricolor.red = tricolor.blue = tricolor.green = 96;
  tricolor.alpha = 255;
}

void
HistoryMenu::Draw (BRect)
{
	BRect bounds (Bounds());

	rgb_color bright = {255, 255, 255, 255};
	rgb_color dark   = {152, 152, 152, 255};

	BeginLineArray (10);
	float center = floor (bounds.left + bounds.Width() / 2);
	rgb_color color = tricolor;
	rgb_color viewcolor = ViewColor();

	if (viewcolor.red   == tricolor.red
	&&  viewcolor.blue  == tricolor.blue
	&&  viewcolor.green == tricolor.green)
		color.red = color.green = color.blue = 255;

	AddLine (
		bounds.LeftTop(),
		bounds.LeftBottom() + BPoint (0, -1),
		bright);
	AddLine (
		bounds.LeftTop() + BPoint (1, 0),
		bounds.RightTop(),
		bright);
	AddLine (
		bounds.RightTop() + BPoint (0, 1),
		bounds.RightBottom(),
		dark);
	AddLine (
		bounds.RightBottom() + BPoint (-1, 0),
		bounds.LeftBottom(),
		dark);
	AddLine (
		BPoint (center - 2, bounds.bottom - 8),
		BPoint (center + 2, bounds.bottom - 8),
		color);
	AddLine (
		BPoint (center - 1, bounds.bottom - 7),
		BPoint (center + 1, bounds.bottom - 7),
		color);
	AddLine (
		BPoint (center, bounds.bottom - 6),
		BPoint (center, bounds.bottom - 6),
		color);
	EndLineArray();
}

void
HistoryMenu::AttachedToWindow (void)
{
  SetViewColor (Parent()->ViewColor());
}

void
HistoryMenu::MouseDown (BPoint)
{
  BMessage *msg (Window()->CurrentMessage());

  int32 buttons;
  int32 keymodifiers;

  msg->FindInt32 ("buttons", &buttons);
  msg->FindInt32 ("modifiers", &keymodifiers);

  if (buttons == B_PRIMARY_MOUSE_BUTTON
  && (keymodifiers & B_SHIFT_KEY)   == 0
  && (keymodifiers & B_OPTION_KEY)  == 0
  && (keymodifiers & B_COMMAND_KEY) == 0
  && (keymodifiers & B_CONTROL_KEY) == 0)
  {
    tracking = true;
    SetViewColor (tricolor);
    Invalidate();
    Window()->UpdateIfNeeded();

    mousedown = system_time();
    SetMouseEventMask (B_POINTER_EVENTS);
  }
}

void
HistoryMenu::MouseMoved (BPoint, uint32 transit, const BMessage *)
{
  if (tracking)
    switch (transit)
    {
      case B_ENTERED_VIEW:
      {
        SetViewColor (tricolor);
        Invalidate();
        break;
      }
      
      case B_EXITED_VIEW:
      {
        SetViewColor (Parent()->ViewColor());
        Invalidate();
        break;
      }
    }
}

void
HistoryMenu::MouseUp (BPoint point)
{
  if (tracking)
  {
    if (Bounds().Contains (point))
    {
      bigtime_t now (system_time());

      if (now - mousedown < 100000)
        snooze (100000 - (now - mousedown));

      SetViewColor (Parent()->ViewColor());
      Invalidate();

      DoPopUp (false);
    }

  tracking = false;
  }
}

void
HistoryMenu::PreviousBuffer (VTextControl *input)
{
  if (bufferPos)
  {
    if (input->TextView()->TextLength() > 0  && 
    bufferFree < BACK_BUFFER_SIZE &&
    bufferPos == bufferFree)
      backBuffer[bufferFree++] = input->Text();
			
    --bufferPos;

    input->SetText (backBuffer[bufferPos].String());
    input->TextView()->Select (
      input->TextView()->TextLength(),
      input->TextView()->TextLength());
  }
}

void
HistoryMenu::NextBuffer (VTextControl *input)
{
  BString buffer;

  if (bufferPos + 1 < bufferFree)
  {
    ++bufferPos;
    buffer = backBuffer[bufferPos].String();
  }
  else if (bufferFree > 0)
  {
    if (backBuffer[bufferFree-1] == input->Text())
    {
      buffer = "";
      ++bufferPos;
    }
    else
      buffer = input->Text();
  }  
  input->SetText (buffer.String());
  input->TextView()->Select (
  input->TextView()->TextLength(),
  input->TextView()->TextLength());
}

BString
HistoryMenu::Submit (const char *buffer)
{
	// All filled up
	if (bufferFree == BACK_BUFFER_SIZE)
	{
		for (i = 0; i < BACK_BUFFER_SIZE - 1; ++i)
			backBuffer[i] = backBuffer[i + 1];
		bufferFree = BACK_BUFFER_SIZE - 1;
	}
	
	backBuffer[bufferFree] = buffer;

	BString cmd;
	
	for (i = 0; i < backBuffer[bufferFree].Length(); ++i)
		if (backBuffer[bufferFree][i] == '\n')
			cmd += " ";
		else if (backBuffer[bufferFree][i] != '\r')
			cmd += backBuffer[bufferFree][i];

	bufferPos = ++bufferFree;

	return cmd;
}

bool
HistoryMenu::HasHistory (void) const
{
	return bufferFree != 0;
}

void
HistoryMenu::DoPopUp (bool animation)
{
	if (bufferFree)
	{
		if (animation)
		{
			SetViewColor (tricolor);
			Invalidate();
			Window()->UpdateIfNeeded();
			snooze (100000);
			SetViewColor (Parent()->ViewColor());
			Invalidate();
		}

		BPopUpMenu *menu (new BPopUpMenu ("history"));
		const char **inputs (const_cast<const char **>(new char * [bufferFree]));
		char **outputs      (new char * [bufferFree]);
		BMenuItem *item;
		BMessage *msg;

		for (i = 0; i < bufferFree; ++i)
		{
			inputs[i]   = backBuffer[i].String();
			outputs[i]  = new char [backBuffer[i].Length() + 5];
		}

		be_plain_font->GetTruncatedStrings (
			inputs,
			bufferFree,
			B_TRUNCATE_END,
			100,
			outputs);

		for (i = 0; i < bufferFree; ++i)
		{
			msg = new BMessage (M_SUBMIT);

			msg->AddString ("input", backBuffer[i].String());
			menu->AddItem (item = new BMenuItem (outputs[i], msg));

			if (i == bufferPos)
				item->SetMarked (true);
			delete [] outputs[i];
		}

		delete [] inputs;
		delete [] outputs;

		menu->SetFont (be_plain_font);
		menu->SetTargetForItems (Window());
		menu->Go (
			ConvertToScreen (Bounds().LeftBottom()),
			true,
			true);

		delete menu;
	}
}
