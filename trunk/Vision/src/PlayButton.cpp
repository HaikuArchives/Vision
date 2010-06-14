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
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Rene Gollent
 *								 Todd Lair
 */
 
#include <Window.h>

#include "PlayButton.h"


StopButton::StopButton (BPoint left_top, BMessage *msg)
	: BButton (
		BRect (left_top, left_top + BPoint (21, 17)),
		"stop", "", msg)
{
}

StopButton::~StopButton (void)
{
}

void
StopButton::AttachedToWindow (void)
{
	BButton::AttachedToWindow();
	ResizeTo (22, 18);
}

void
StopButton::Draw (BRect frame)
{
	rgb_color black = {0, 0, 0, 255};
	rgb_color white = {255, 255, 255, 255};

	BButton::Draw (frame);

	frame = Bounds();
	frame.InsetBy (9, 7);
	SetHighColor (Value() ? white : black);
	FillRect (frame);
}

