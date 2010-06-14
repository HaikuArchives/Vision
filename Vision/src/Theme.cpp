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
 
#define NUMBER_THEME_READERS	1000

#include <Message.h>
#include <Messenger.h>
#include <View.h>
#include <Font.h>

#include "Theme.h"
#include "VisionBase.h"

int16 Theme::TimestampFore			= 0;
int16 Theme::TimestampBack			= 0;
int16 Theme::TimestampFont			= 0;
int16 Theme::TimespaceFore			= 1;
int16 Theme::TimespaceBack			= 1;
int16 Theme::TimespaceFont			= 1;
int16 Theme::NormalFore				 = 2;
int16 Theme::NormalBack				 = 2;
int16 Theme::NormalFont				 = 2;
int16 Theme::SelectionBack			= 3;

Theme::Theme (
	const char *n,
	int16 foreCount,
	int16 backCount,
	int16 fontCount)
	:	name (NULL),
		 fores (NULL),
		 backs (NULL),
		 fonts (NULL),
		 fore_count (max_c (foreCount, 4)),
		 back_count (max_c (backCount, 4)),
		 font_count (max_c (fontCount, 4))
{
	name = strcpy (new char [strlen (n) + 1], n);

	fores = new rgb_color [fore_count];
	backs = new rgb_color [back_count];
	fonts = new BFont		 [font_count];

	sid = create_sem (NUMBER_THEME_READERS, name);

	rgb_color def_timestamp_fore	= {200, 150, 150, 255};
	rgb_color def_timestamp_back	= {255, 255, 255, 255};
	rgb_color def_fore						= {0, 0, 0, 255};
	rgb_color def_back						= {255, 255, 255, 255};

	fores[0] = def_timestamp_fore;

	int16 i;
	for (i = 1; i < fore_count; ++i)
		fores[i] = def_fore;

	backs[0] = def_timestamp_back;
	for (i = 1; i < back_count; ++i)
		backs[i] = def_back;
}

Theme::~Theme (void)
{
	delete_sem (sid);

	delete [] fonts;
	delete [] backs;
	delete [] fores;
	delete [] name;
}

int16
Theme::CountForegrounds (void) const
{
	return fore_count;
}

int16
Theme::CountBackgrounds (void) const
{
	return back_count;
}

int16
Theme::CountFonts (void) const
{
	return font_count;
}

void
Theme::ReadLock (void)
{
	acquire_sem (sid);
}

void
Theme::ReadUnlock (void)
{
	release_sem (sid);
}

void
Theme::WriteLock (void)
{
	acquire_sem_etc (sid, NUMBER_THEME_READERS, 0, 0);
}

void
Theme::WriteUnlock (void)
{
	release_sem_etc (sid, NUMBER_THEME_READERS, 0);
}

const rgb_color 
Theme::ForegroundAt (int16 which) const
{
	rgb_color color = {0, 0, 0, 255};

	if (which >= fore_count || which < 0)
		return color;

	return fores[which];
}

const rgb_color
Theme::BackgroundAt (int16 which) const
{
	rgb_color color = {255, 255, 255, 255};

	if (which >= back_count || which < 0)
		return color;

	return backs[which];
}

const BFont &
Theme::FontAt (int16 which) const
{
	if (which >= font_count || which < 0)
		return *be_plain_font;

	return fonts[which];
}

bool
Theme::SetForeground (int16 which, const rgb_color color)
{
	if (which >= fore_count || which < 0)
		return false;

	int32 count (list.CountItems());
	fores[which] = color;

	BMessage msg (M_THEME_FOREGROUND_CHANGE);
	msg.AddInt16 ("which", which);

	for (int32 i = 0; i < count; ++i)
	{
		BView *view ((BView *)list.ItemAt (i));
		BMessenger msgr (view);

		msgr.SendMessage (&msg);
	}

	return true;
}

bool
Theme::SetBackground (int16 which, const rgb_color color)
{
	if (which >= back_count || which < 0)
		return false;

	int32 count (list.CountItems());
	backs[which] = color;

	BMessage msg (M_THEME_BACKGROUND_CHANGE);
	msg.AddInt16 ("which", which);

	for (int32 i = 0; i < count; ++i)
	{
		BView *view ((BView *)list.ItemAt (i));
		BMessenger msgr (view);

		msgr.SendMessage (&msg);
	}

	return true;
}

bool
Theme::SetFont (int16 which, const BFont &font)
{
	if (which >= font_count || which < 0)
		return false;

	int32 count (list.CountItems());
	fonts[which] = font;

	BMessage msg (M_THEME_FONT_CHANGE);
	msg.AddInt16 ("which", which);

	for (int32 i = 0; i < count; ++i)
	{
		BView *view ((BView *)list.ItemAt (i));
		BMessenger msgr (view);

		msgr.SendMessage (&msg);
	}

	return true;
}

void
Theme::AddView (BView *view)
{
	list.AddItem (view);
}

void
Theme::RemoveView (BView *view)
{
	list.RemoveItem (view);
}


