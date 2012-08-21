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

#include "PrefFont.h"
#include "NumericFilter.h"
#include "Vision.h"
#include "VTextControl.h"

#include <ctype.h>
#include <stdlib.h>

#include <Catalog.h>
#include <Menu.h>
#include <MenuBar.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ScrollView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "FontPrefs"

struct FontStat
{
	font_family				family;
	int32						style_count;
	font_style				*styles;
};

class FontMenuItem : public BMenuItem
{
	public:
		FontMenuItem (const char *, const char *, BMessage *);
		FontMenuItem (const char *, const char *, BMenu *, BMessage* = NULL); 
		virtual ~FontMenuItem (void);
		virtual void DrawContent (void);
	
	private:
		const char *fontName;
		const char *fontStyle;
		BFont myFont;
};

FontMenuItem::FontMenuItem (const char *font, const char *style, BMessage *msg)
		: BMenuItem (style, msg),
			 fontName (font),
			 fontStyle (style)
{
	myFont.SetFamilyAndStyle (font, style);
}

FontMenuItem::FontMenuItem (const char *font, const char *style, BMenu *menu, BMessage *msg)
		: BMenuItem (menu, msg),
			 fontName (font),
			 fontStyle (style)
{
	myFont.SetFamilyAndStyle (font, style);
}

FontMenuItem::~FontMenuItem (void)
{
}

void
FontMenuItem::DrawContent (void)
{
	BMenu *menu (Menu());
	if (menu)
		menu->SetFont (&myFont, B_FONT_FAMILY_AND_STYLE);

	BMenuItem::DrawContent();
}

class FontMenu : public BMenu
{
	public:
		FontMenu (const char *);
		virtual ~FontMenu (void);
		virtual void AttachedToWindow (void);
};

FontMenu::FontMenu (const char *name)
		: BMenu (name)
{
	if (CountItems() == 0)
	{
		int32 i, family_count (count_font_families());

		FontStat *font_stat = new FontStat [family_count];

		for (i = 0; i < family_count; ++i)
		{
			uint32 flags;

			*font_stat[i].family			 = '\0';
			font_stat[i].style_count	 = 0;
			font_stat[i].styles				= 0;

			if (get_font_family (i, &font_stat[i].family, &flags) == B_OK
			&& (font_stat[i].style_count = count_font_styles (font_stat[i].family)) > 0)
			{
				font_stat[i].styles = new font_style [font_stat[i].style_count];

				for (int32 j = 0; j < font_stat[i].style_count; ++j)
				{
					*font_stat[i].styles[j] = '\0';
					get_font_style (font_stat[i].family, j, font_stat[i].styles + j, &flags);
				}
			}
		}

		int32 j;

		for (j = 0; j < family_count; ++j)
			if (*font_stat[j].family && font_stat[j].style_count)
			{
				BMenu *menu (new BMenu (font_stat[j].family));
				FontMenuItem *fontMenu (new FontMenuItem (font_stat[j].family, "", menu));
				AddItem (fontMenu);

				for (int32 k = 0; k < font_stat[j].style_count; ++k)
				{
					BMessage *msg (new BMessage (M_FONT_CHANGE));
					FontMenuItem *item;

					msg->AddString ("family", font_stat[j].family);
					msg->AddString ("style", font_stat[j].styles[k]);
					msg->AddInt32 ("which", i);

					menu->AddItem (item = new FontMenuItem (font_stat[j].family, font_stat[j].styles[k], msg));
				}
			}

		for (i = 0; i < family_count; ++i)
			delete [] font_stat[i].styles;
		delete [] font_stat;
	}
}

FontMenu::~FontMenu()
{
}

void
FontMenu::AttachedToWindow (void)
{
	BMenu::AttachedToWindow();
}

static const char *FontControlLabels[] =
{
	B_TRANSLATE_MARK("Text"),
	B_TRANSLATE_MARK("Server Messages"),
	B_TRANSLATE_MARK("URLs"),
	B_TRANSLATE_MARK("Names List"),
	B_TRANSLATE_MARK("Input Text"),
	B_TRANSLATE_MARK("Window List"),
	B_TRANSLATE_MARK("Channel List"),
	B_TRANSLATE_MARK("Timestamp"),
	0
};


FontPrefsView::FontPrefsView (BRect frame)
	: BView (frame, "Font prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
		fFontMenuField (NULL),
		fFontElementField (NULL),
		fActiveFont (0)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BMenu *fElementMenu (new BMenu ("elements"));
	for (int32 i = 0; FontControlLabels[i]; i++)
	{
		BMessage *msg (new BMessage (M_FONT_ELEMENT_CHANGE));
		msg->AddInt32 ("index", i);
		fElementMenu->AddItem (new BMenuItem (B_TRANSLATE(FontControlLabels[i]), msg));
	}
	fFontElementField = new BMenuField (BRect (10, 10, 200, 50), "elements", "Element: ",
		fElementMenu);
	AddChild (fFontElementField);
	FontMenu *menu (new FontMenu ("fonts"));
	BString itemText = B_TRANSLATE("Font");
	itemText += ": ";
	fFontMenuField = new BMenuField (BRect (10, 10, 200, 50), "fonts", itemText.String(), menu);
	AddChild (fFontMenuField);
	itemText = B_TRANSLATE("Size");
	itemText += ": ";
	fTextControl = new VTextControl (BRect (60, 60, 200, 90), "", itemText.String(), "",
		new BMessage (M_FONT_SIZE_CHANGE));
	fTextControl->TextView()->AddFilter (new NumericFilter());
	AddChild (fTextControl);
}

FontPrefsView::~FontPrefsView (void)
{
}

void
FontPrefsView::AttachedToWindow (void)
{
	BView::AttachedToWindow ();
}

void
FontPrefsView::AllAttached (void)
{
	BView::AllAttached ();
	fFontElementField->SetDivider (fFontElementField->StringWidth(
		fFontElementField->Label()) + 5);
	fFontElementField->ResizeToPreferred();
	fFontMenuField->SetDivider (fFontMenuField->StringWidth (fFontMenuField->Label()) + 5);
	fFontMenuField->ResizeToPreferred();
	fTextControl->SetDivider (fTextControl->StringWidth (fTextControl->Label()) + 5);
	BMenu *menu (fFontElementField->Menu());
	fTextControl->SetTarget (this);
	menu->SetTargetForItems (this);
	menu->SetLabelFromMarked (true);

	BMenuItem *it (menu->ItemAt (0));

	if (it)
		dynamic_cast<BInvoker *>(it)->Invoke();

	BRect frame (fFontElementField->Frame());
	fFontMenuField->MoveTo (frame.left, frame.bottom + 20);
	menu = fFontMenuField->Menu();
	menu->SetTargetForItems (this);
	menu->SetLabelFromMarked (true);
	float width;
	float height;
	fFontMenuField->GetPreferredSize(&width, &height);
	fTextControl->ResizeToPreferred();
	fTextControl->MoveTo (fFontMenuField->Frame().right + width + 5,
		fFontMenuField->Frame().top);

	for (int32 i = 0; i < menu->CountItems(); i++)
		menu->ItemAt(i)->Submenu()->SetTargetForItems (this);

	const BFont *current (vision_app->GetClientFont (0));
	font_family family;
	font_style style;
	current->GetFamilyAndStyle (&family, &style);
	it = menu->FindItem (family);

	if (it)
	{
		it = it->Submenu()->FindItem (style);
		if (it)
			dynamic_cast<BInvoker *>(it)->Invoke();
	}
}

void
FontPrefsView::FrameResized (float width, float height)
{
	BView::FrameResized (width, height);
}

static inline void
UnsetMarked (BMenuField *field)
{
	BMenuItem *item;
	for (int32 i = 0; i < field->Menu()->CountItems(); ++i)
	{
		BMenu *menu (field->Menu()->SubmenuAt (i));
		 
		if ((item = menu->FindMarked()) != 0)
		{
			item->SetMarked (false);
			break;
		}
	}
}

void
FontPrefsView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_FONT_CHANGE:
			{
				BMenuItem *item (NULL);
				const char *family (NULL);
				const char *style (NULL);
				UnsetMarked (fFontMenuField);

				msg->FindPointer ("source", reinterpret_cast<void **>(&item));
		item->SetMarked (true);
				msg->FindString ("family", &family);
				msg->FindString ("style", &style);
				vision_app->ClientFontFamilyAndStyle (fActiveFont, family, style);
				fFontMenuField->MenuItem()->SetLabel (family);
			}
			break;
		
		case M_FONT_SIZE_CHANGE:
			{
				const char *text (fTextControl->TextView()->Text());
				float size (atof (text));
				vision_app->ClientFontSize (fActiveFont, size);
			}
			break;
			
		case M_FONT_ELEMENT_CHANGE:
		{
			 fActiveFont = msg->FindInt32 ("index");
			 UnsetMarked (fFontMenuField);
			 const BFont *font (vision_app->GetClientFont (fActiveFont));
			 font_family family;
			 font_style style;
			 font->GetFamilyAndStyle (&family, &style);
			 char line[100];
			 memset (line, 0, sizeof(line));
			 sprintf(line, "%ld", (long)(font->Size()));
			 fTextControl->TextView()->SetText (line);
			 BMenuItem *it = fFontMenuField->Menu()->FindItem (family);
			 if (it)
			 {
				 it = it->Submenu()->FindItem (style);
				 if (it)
					 it->SetMarked (true);
			 }
			 fFontMenuField->MenuItem()->SetLabel (family);
		}
		break;
		
		default:
			BView::MessageReceived (msg);
	}
}
