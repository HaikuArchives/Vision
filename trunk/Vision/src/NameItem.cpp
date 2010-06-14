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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 */
 
#include <stdio.h>

#include "Vision.h"
#include "Names.h"
#include "Theme.h"

NameItem::NameItem (
	const char *name,
	int32 userStatus)

	: BListItem (),
		myName (name),
		myAddress (""),
		myStatus (userStatus)
{
}

BString
NameItem::Name (void) const
{
	return myName;
}

BString
NameItem::Address (void) const
{
	return myAddress;
}

int32
NameItem::Status() const
{
	return myStatus;
}

void
NameItem::SetName (const char *name)
{
	myName = name;
}

void
NameItem::SetAddress (const char *address)
{
	myAddress = address;
}

void
NameItem::SetStatus (int32 userStatus)
{
	myStatus = userStatus;
}

void
NameItem::DrawItem (BView *father, BRect frame, bool complete)
{
	Theme *activeTheme (vision_app->ActiveTheme());
	activeTheme->ReadLock();
	if (IsSelected())
	{
		father->SetLowColor (activeTheme->ForegroundAt (C_NAMES_SELECTION));
		father->FillRect (frame, B_SOLID_LOW);
	}
	else if (complete)
	{
		father->SetLowColor (activeTheme->ForegroundAt (C_NAMES_BACKGROUND));
		father->FillRect (frame, B_SOLID_LOW);
	}

	font_height fh;
	father->GetFontHeight (&fh);

	father->MovePenTo (
	frame.left + 4,
	frame.bottom - fh.descent);

	BString drawString (myName);
	rgb_color color = activeTheme->ForegroundAt (C_NAMES);

	if ((myStatus & STATUS_FOUNDER_BIT) != 0)
	{
		drawString.Prepend ("*");
		color = activeTheme->ForegroundAt (C_OP);
	}
	else if ((myStatus & STATUS_PROTECTED_BIT) != 0)
	{
		drawString.Prepend ("!");
		color = activeTheme->ForegroundAt (C_OP);
	}
	else if ((myStatus & STATUS_OP_BIT) != 0)
	{
		drawString.Prepend ("@");
		color = activeTheme->ForegroundAt (C_OP);
	}
	else if ((myStatus & STATUS_HELPER_BIT) != 0)
	{
		drawString.Prepend ("%");
		color = activeTheme->ForegroundAt (C_HELPER);
	}
	else if ((myStatus & STATUS_VOICE_BIT) != 0)
	{
		drawString.Prepend("+");
		color = activeTheme->ForegroundAt (C_VOICE);
	}

	if ((myStatus & STATUS_IGNORE_BIT) != 0)
		color = activeTheme->ForegroundAt (C_IGNORE);

	activeTheme->ReadUnlock();

	father->SetHighColor (color);

	father->SetDrawingMode (B_OP_OVER);
	father->DrawString (drawString.String());
	father->SetDrawingMode (B_OP_COPY);
}
