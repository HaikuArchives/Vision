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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefColor"

// data structures for font prefs

struct FontStat {
	font_family family;
	int32 style_count;
	font_style* styles;
};

static const char* ColorLabels[] = {
	B_TRANSLATE("Text"),		  B_TRANSLATE("Background"), B_TRANSLATE("URL"),
	B_TRANSLATE("Server text"),   B_TRANSLATE("Notice"),	 B_TRANSLATE("Action"),
	B_TRANSLATE("Quit"),		  B_TRANSLATE("Error"),	  B_TRANSLATE("Nickname edges"),
	B_TRANSLATE("User nickname edges"),  B_TRANSLATE("Nickname text"),  B_TRANSLATE("Join"),
	B_TRANSLATE("Kick"),		  B_TRANSLATE("Whois"),	  B_TRANSLATE("Names (normal)"),
	B_TRANSLATE("Names (OP)"),	 B_TRANSLATE("Names (helper)"), B_TRANSLATE("Names (voice)"),
	B_TRANSLATE("Names selection"),	B_TRANSLATE("Names background"),   B_TRANSLATE("CTCP request"),
	B_TRANSLATE("CTCP reply"),	 B_TRANSLATE("Ignore"),	 B_TRANSLATE("Input text"),
	B_TRANSLATE("Input background"),	 B_TRANSLATE("Winlist normal status"), B_TRANSLATE("Winlist text status"),
	B_TRANSLATE("Winlist nick alert status"),   B_TRANSLATE("Winlist selection status"),  B_TRANSLATE("Winlist event status"),
	B_TRANSLATE("Winlist background"),	 B_TRANSLATE("Wallops"),	B_TRANSLATE("Timestamp"),
	B_TRANSLATE("Timestamp background"), B_TRANSLATE("Selection"),  B_TRANSLATE("mIRC white"),
	B_TRANSLATE("mIRC black"),	B_TRANSLATE("mIRC dark blue"),  B_TRANSLATE("mIRC green"),
	B_TRANSLATE("mIRC red"),	  B_TRANSLATE("mIRC brown"),  B_TRANSLATE("mIRC purple"),
	B_TRANSLATE("mIRC orange"),   B_TRANSLATE("mIRC yellow"), B_TRANSLATE("mIRC lime"),
	B_TRANSLATE("mIRC teal"),	 B_TRANSLATE("mIRC aqua"),   B_TRANSLATE("mIRC light blue"),
	B_TRANSLATE("mIRC pink"),	 B_TRANSLATE("mIRC grey"),   B_TRANSLATE("mIRC silver"),
	B_TRANSLATE("Notify online"),	B_TRANSLATE("Notify offline"), B_TRANSLATE("Notify list background"),
	B_TRANSLATE("Notify list selection")};

ColorPrefsView::ColorPrefsView(BRect frame)
	: BView(frame, "Color Prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	int32 i(0);
	AdoptSystemColors();
	for (i = 0; i < MAX_COLORS; i++) fColors[i] = vision_app->GetColor(i);

	BMessage mycolors, labels;

	for (i = 0; i < MAX_COLORS; i++) {
		mycolors.AddData("color", B_RGB_COLOR_TYPE, &fColors[i], sizeof(rgb_color));
		labels.AddString("color", ColorLabels[i]);
	}

	fSelector = new ColorSelector(frame, "fSelector", NULL, mycolors, labels, new BMessage('vtst'));
	fSelector->AdoptSystemColors();
	fSelector->ResizeToPreferred();
	fRevert = new BButton(BRect(0, 0, 0, 0), "fRevert", B_TRANSLATE("Revert"),
						  new BMessage(M_REVERT_COLOR_SELECTIONS));
	fRevert->ResizeToPreferred();
	ResizeTo(fSelector->Bounds().Width() + 30,
			 fSelector->Bounds().Height() + 30 + fRevert->Bounds().Height());
	AddChild(fSelector);
	AddChild(fRevert);
}

ColorPrefsView::~ColorPrefsView(void)
{
}

void ColorPrefsView::AttachedToWindow(void)
{
	BView::AttachedToWindow();
}

void ColorPrefsView::AllAttached(void)
{
	BView::AllAttached();
	fSelector->ResizeToPreferred();
	fSelector->MoveTo((Bounds().Width() - fSelector->Bounds().Width()) / 2, 5);
	fRevert->MoveTo(fSelector->Frame().left, fSelector->Frame().bottom + 10);
	fRevert->SetTarget(this);
}

void ColorPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_REVERT_COLOR_SELECTIONS:
		fSelector->Revert();
		break;
	}
	BView::MessageReceived(msg);
}
