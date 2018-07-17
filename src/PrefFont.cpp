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

#include "PrefFont.h"
#include "Vision.h"

#include <ctype.h>
#include <stdlib.h>

#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <TextControl.h>

#include "NumericFilter.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefFont"

struct FontStat {
	font_family family;
	int32 style_count;
	font_style* styles;
};

class FontMenuItem : public BMenuItem
{
public:
	FontMenuItem(const char*, const char*, BMessage*);
	FontMenuItem(const char*, const char*, BMenu*, BMessage* = NULL);
	virtual ~FontMenuItem();
	virtual void DrawContent();

private:
	const char* fontName;
	const char* fontStyle;
	BFont myFont;
};

FontMenuItem::FontMenuItem(const char* font, const char* style, BMessage* msg)
	: BMenuItem(style, msg), fontName(font), fontStyle(style)
{
	myFont.SetFamilyAndStyle(font, style);
}

FontMenuItem::FontMenuItem(const char* font, const char* style, BMenu* menu, BMessage* msg)
	: BMenuItem(menu, msg), fontName(font), fontStyle(style)
{
	myFont.SetFamilyAndStyle(font, style);
}

FontMenuItem::~FontMenuItem()
{
}

void FontMenuItem::DrawContent()
{
	BMenu* menu(Menu());
	if (menu) menu->SetFont(&myFont, B_FONT_FAMILY_AND_STYLE);

	BMenuItem::DrawContent();
}

class FontMenu : public BMenu
{
public:
	FontMenu(const char*);
	virtual ~FontMenu();
	virtual void AttachedToWindow();
};

FontMenu::FontMenu(const char* name) : BMenu(name)
{
	if (CountItems() == 0) {
		int32 i, family_count(count_font_families());

		FontStat* font_stat = new FontStat[family_count];

		for (i = 0; i < family_count; ++i) {
			uint32 flags;

			*font_stat[i].family = '\0';
			font_stat[i].style_count = 0;
			font_stat[i].styles = 0;

			if (get_font_family(i, &font_stat[i].family, &flags) == B_OK &&
				(font_stat[i].style_count = count_font_styles(font_stat[i].family)) > 0) {
				font_stat[i].styles = new font_style[font_stat[i].style_count];

				for (int32 j = 0; j < font_stat[i].style_count; ++j) {
					*font_stat[i].styles[j] = '\0';
					get_font_style(font_stat[i].family, j, font_stat[i].styles + j, &flags);
				}
			}
		}

		int32 j;

		for (j = 0; j < family_count; ++j)
			if (*font_stat[j].family && font_stat[j].style_count) {
				BMenu* menu(new BMenu(font_stat[j].family));
				FontMenuItem* fontMenu(new FontMenuItem(font_stat[j].family, "", menu));
				AddItem(fontMenu);

				for (int32 k = 0; k < font_stat[j].style_count; ++k) {
					BMessage* msg(new BMessage(M_FONT_CHANGE));
					FontMenuItem* item;

					msg->AddString("family", font_stat[j].family);
					msg->AddString("style", font_stat[j].styles[k]);
					msg->AddInt32("which", i);

					menu->AddItem(
						item = new FontMenuItem(font_stat[j].family, font_stat[j].styles[k], msg));
				}
			}

		for (i = 0; i < family_count; ++i) delete[] font_stat[i].styles;
		delete[] font_stat;
	}
}

FontMenu::~FontMenu()
{
}

void FontMenu::AttachedToWindow()
{
	BMenu::AttachedToWindow();
}

static const char* FontControlLabels[] = {
	B_TRANSLATE("Text"),	  B_TRANSLATE("Server messages"),  B_TRANSLATE("URLs"),
	B_TRANSLATE("Names list"), B_TRANSLATE("Input text"), B_TRANSLATE("Window list"),
	B_TRANSLATE("Channel list"),  B_TRANSLATE("Timestamp"),	 0};

FontPrefsView::FontPrefsView()
	: BView("Font prefs", 0),
	  fFontMenuField(NULL),
	  fFontElementField(NULL),
	  fActiveFont(0)
{
	AdoptSystemColors();
	BMenu* fElementMenu(new BMenu("elements"));
	for (int32 i = 0; FontControlLabels[i]; i++) {
		BMessage* msg(new BMessage(M_FONT_ELEMENT_CHANGE));
		msg->AddInt32("index", i);
		fElementMenu->AddItem(new BMenuItem(FontControlLabels[i], msg));
	}

	fFontElementField =
		new BMenuField("elements", B_TRANSLATE("Element:"), fElementMenu);

	FontMenu* menu(new FontMenu("fonts"));
	fFontMenuField = new BMenuField("fonts", B_TRANSLATE("Font:"), menu);

	fTextControl = new BTextControl("", "Size:", "",
		new BMessage(M_FONT_SIZE_CHANGE));
	fTextControl->TextView()->AddFilter(new NumericFilter());

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_HALF_ITEM_INSETS)
		.AddGroup(B_HORIZONTAL)
			.Add(fFontElementField)
			.AddGlue(10.0)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(fFontMenuField)
			.Add(fTextControl)
			.AddGlue(10.0)
		.End()
		.AddGlue()
	.End();
}

FontPrefsView::~FontPrefsView()
{
}

void FontPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
}

void FontPrefsView::AllAttached()
{
	BView::AllAttached();
	BMenu* menu = fFontElementField->Menu();
	fTextControl->SetTarget(this);
	menu->SetTargetForItems(this);
	menu->SetLabelFromMarked(true);

	BMenuItem* it(menu->ItemAt(0));

	if (it) dynamic_cast<BInvoker*>(it)->Invoke();

	menu = fFontMenuField->Menu();
	menu->SetTargetForItems(this);
	menu->SetLabelFromMarked(true);

	for (int32 i = 0; i < menu->CountItems(); i++)
		menu->ItemAt(i)->Submenu()->SetTargetForItems(this);

	const BFont* current(vision_app->GetClientFont(0));
	font_family family;
	font_style style;
	current->GetFamilyAndStyle(&family, &style);
	it = menu->FindItem(family);

	if (it) {
		it = it->Submenu()->FindItem(style);
		if (it) dynamic_cast<BInvoker*>(it)->Invoke();
	}
}

void FontPrefsView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
}

static inline void UnsetMarked(BMenuField* field)
{
	BMenuItem* item;
	for (int32 i = 0; i < field->Menu()->CountItems(); ++i) {
		BMenu* menu(field->Menu()->SubmenuAt(i));

		if ((item = menu->FindMarked()) != 0) {
			item->SetMarked(false);
			break;
		}
	}
}

void FontPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_FONT_CHANGE: {
		BMenuItem* item(NULL);
		const char* family(NULL);
		const char* style(NULL);
		UnsetMarked(fFontMenuField);

		msg->FindPointer("source", reinterpret_cast<void**>(&item));
		item->SetMarked(true);
		msg->FindString("family", &family);
		msg->FindString("style", &style);
		vision_app->ClientFontFamilyAndStyle(fActiveFont, family, style);
		fFontMenuField->MenuItem()->SetLabel(family);
	} break;

	case M_FONT_SIZE_CHANGE: {
		const char* text(fTextControl->TextView()->Text());
		float size(atof(text));
		vision_app->ClientFontSize(fActiveFont, size);
	} break;

	case M_FONT_ELEMENT_CHANGE: {
		fActiveFont = msg->FindInt32("index");
		UnsetMarked(fFontMenuField);
		const BFont* font(vision_app->GetClientFont(fActiveFont));
		font_family family;
		font_style style;
		font->GetFamilyAndStyle(&family, &style);
		char line[100];
		memset(line, 0, sizeof(line));
		sprintf(line, "%ld", (long)(font->Size()));
		fTextControl->TextView()->SetText(line);
		BMenuItem* it = fFontMenuField->Menu()->FindItem(family);
		if (it) {
			it = it->Submenu()->FindItem(style);
			if (it) it->SetMarked(true);
		}
		fFontMenuField->MenuItem()->SetLabel(family);
	} break;

	default:
		BView::MessageReceived(msg);
	}
}
