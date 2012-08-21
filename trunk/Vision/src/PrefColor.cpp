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

// TODO: Color Schemes/Themes


#include "PrefColor.h"
#include "ColorSelector.h"
#include "Vision.h"
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <MenuField.h>
#include <StringView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Point.h>

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ColorPrefs"

// data structures for font prefs

struct FontStat
{
	font_family				family;
	int32						style_count;
	font_style				*styles;
};

static const char *ColorLabels[] =
{
	B_TRANSLATE_MARK("Text"),
	B_TRANSLATE_MARK("Background"),
	B_TRANSLATE_MARK("URL"),
	B_TRANSLATE_MARK("Server Text"),
	B_TRANSLATE_MARK("Notice"),
	B_TRANSLATE_MARK("Action"),
	B_TRANSLATE_MARK("Quit"),
	B_TRANSLATE_MARK("Error"),
	B_TRANSLATE_MARK("Nickname edges (other users)"),
	B_TRANSLATE_MARK("Nickname edges (self)"),
	B_TRANSLATE_MARK("Nickname text"),
	B_TRANSLATE_MARK("Join"),
	B_TRANSLATE_MARK("Kick"),
	B_TRANSLATE_MARK("Whois"),
	B_TRANSLATE_MARK("Names (Normal)"),
	B_TRANSLATE_MARK("Names (Op)"),
	B_TRANSLATE_MARK("Names (Helper)"),
	B_TRANSLATE_MARK("Names (Voice)"),
	B_TRANSLATE_MARK("Names selected item"),
	B_TRANSLATE_MARK("Names background"),
	B_TRANSLATE_MARK("CTCP Request"),
	B_TRANSLATE_MARK("CTCP Reply"),
	B_TRANSLATE_MARK("Ignore"),
	B_TRANSLATE_MARK("Input Text"),
	B_TRANSLATE_MARK("Input Background"),
	B_TRANSLATE_MARK("Winlist normal status"),
	B_TRANSLATE_MARK("Winlist text status"),
	B_TRANSLATE_MARK("Winlist nick alert status"),
	B_TRANSLATE_MARK("Winlist selected item"),
	B_TRANSLATE_MARK("Winlist event status"),
	B_TRANSLATE_MARK("Winlist background"),
	B_TRANSLATE_MARK("Wwallops"),
	B_TRANSLATE_MARK("Timestamp"),
	B_TRANSLATE_MARK("Timestamp background"),
	B_TRANSLATE_MARK("Text selection"),
	B_TRANSLATE_MARK("mIRC White"),
	B_TRANSLATE_MARK("mIRC Black"),
	B_TRANSLATE_MARK("mIRC Dark Blue"),
	B_TRANSLATE_MARK("mIRC Green"),
	B_TRANSLATE_MARK("mIRC Red"),
	B_TRANSLATE_MARK("mIRC Brown"),
	B_TRANSLATE_MARK("mIRC Purple"),
	B_TRANSLATE_MARK("mIRC Orange"),
	B_TRANSLATE_MARK("mIRC Yellow"),
	B_TRANSLATE_MARK("mIRC Lime"),
	B_TRANSLATE_MARK("mIRC Teal"),
	B_TRANSLATE_MARK("mIRC Aqua"),
	B_TRANSLATE_MARK("mIRC Light Blue"),
	B_TRANSLATE_MARK("mIRC Pink"),
	B_TRANSLATE_MARK("mIRC Grey"),
	B_TRANSLATE_MARK("mIRC Silver"),
	B_TRANSLATE_MARK("Notify Online"),
	B_TRANSLATE_MARK("Notify Offline"),
	B_TRANSLATE_MARK("Notify List background"),
	B_TRANSLATE_MARK("Notify List selected item")
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
		labels.AddString ("color", B_TRANSLATE(ColorLabels[i]));
	}

	fSelector = new ColorSelector (frame, "fSelector", NULL, mycolors, labels, new BMessage ('vtst'));
	fSelector->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	fSelector->ResizeToPreferred();
	fRevert = new BButton (BRect (0,0,0,0), "fRevert", B_TRANSLATE("Revert"), new BMessage (M_REVERT_COLOR_SELECTIONS));
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

