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
 */

#include "PrefApp.h"
#include "Vision.h"

#include <stdio.h>

#include <Catalog.h>
#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <UTF8.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AppPrefs"

AppWindowPrefsView::AppWindowPrefsView (BRect frame)
	: BView (frame, "App/Window Prefs", B_FOLLOW_NONE, B_WILL_DRAW)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BMessage msg (M_APPWINDOWPREFS_SETTING_CHANGED);
	float maxWidth (0),
		maxHeight (0);
	BRect trackingBoundsRect (0.0, 0.0, 0, 0);
	BRect checkboxRect(Bounds());
	checkboxRect.bottom = checkboxRect.top;
	msg.AddString ("setting", "versionParanoid");
	fVersionParanoid = new BCheckBox (checkboxRect, "version Paranoid",
		B_TRANSLATE("Show OS information in version reply"),
		new BMessage (msg));
	fVersionParanoid->SetValue ((!vision_app->GetBool ("versionParanoid")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fVersionParanoid->MoveBy(be_plain_font->StringWidth("S"), 0);
	fVersionParanoid->ResizeToPreferred();
	trackingBoundsRect = fVersionParanoid->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height(); 
	AddChild (fVersionParanoid);
	
	checkboxRect.OffsetBy(0.0, fVersionParanoid->Bounds().Height() * 1.2);
	msg.ReplaceString ("setting", "catchAltW");
	fCatchAltW = new BCheckBox (checkboxRect, "catch AltW",
		B_TRANSLATE("Require double Cmd+Q/W to close"),
		new BMessage (msg));
	fCatchAltW->SetValue ((vision_app->GetBool ("catchAltW")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fCatchAltW->MoveBy(be_plain_font->StringWidth("S"), 0);
	fCatchAltW->ResizeToPreferred();
	trackingBoundsRect = fCatchAltW->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.2; 
	AddChild (fCatchAltW);
	
	checkboxRect.OffsetBy(0.0, fCatchAltW->Bounds().Height() * 1.2);
	msg.ReplaceString ("setting", "stripcolors");
	fStripColors = new BCheckBox (checkboxRect, "stripcolors",
		B_TRANSLATE("Strip mIRC Colors"),
		new BMessage (msg));
	fStripColors->SetValue ((vision_app->GetBool ("stripcolors")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fStripColors->MoveBy(be_plain_font->StringWidth("S"), 0);
	fStripColors->ResizeToPreferred();
	trackingBoundsRect = fStripColors->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5; 
	AddChild (fStripColors);

	checkboxRect.OffsetBy(0.0, fStripColors->Bounds().Height() * 1.2);
	msg.ReplaceString ("setting", "Newbie Spam Mode");
	fSpamMode = new BCheckBox (checkboxRect, "newbiespammode",
		B_TRANSLATE("Warn when multiline pasting"),
		new BMessage (msg));
	fSpamMode->SetValue ((vision_app->GetBool ("Newbie Spam Mode")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fSpamMode->MoveBy(be_plain_font->StringWidth("S"), 0);
	fSpamMode->ResizeToPreferred();
	trackingBoundsRect = fSpamMode->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5; 
	AddChild (fSpamMode);

	checkboxRect.OffsetBy(0.0, fSpamMode->Bounds().Height() * 1.2);
	msg.ReplaceString ("setting", "queryOnMsg");
	fQueryMsg = new BCheckBox (checkboxRect, "queryOnMsg",
		B_TRANSLATE("Open new query window on message"),
		new BMessage (msg));
	fQueryMsg->SetValue ((vision_app->GetBool ("queryOnMsg")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fQueryMsg->MoveBy(be_plain_font->StringWidth("S"), 0);
	fQueryMsg->ResizeToPreferred();
	trackingBoundsRect = fSpamMode->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5; 
	AddChild (fQueryMsg);

	checkboxRect.OffsetBy(0.0, fQueryMsg->Bounds().Height() * 1.2);
	BMenu *encMenu(CreateEncodingMenu());
	
	checkboxRect.left = 0.0;
	checkboxRect.right = Bounds().Width();
	checkboxRect.bottom += fQueryMsg->Bounds().Height() * 1.2;
	
	fEncodings = new BMenuField(checkboxRect, "encoding", "Encoding: ", encMenu);

	AddChild (fEncodings);
	fEncodings->Menu()->SetLabelFromMarked(true);

	trackingBoundsRect = fEncodings->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5; 
	
	ResizeTo(maxWidth, maxHeight);
}

AppWindowPrefsView::~AppWindowPrefsView (void)
{
}

void
AppWindowPrefsView::AttachedToWindow (void)
{
	BView::AttachedToWindow();
}

void
AppWindowPrefsView::AllAttached (void)
{
	fVersionParanoid->SetTarget (this);
	fCatchAltW->SetTarget (this);
	fStripColors->SetTarget (this);
	fSpamMode->SetTarget (this);
	fQueryMsg->SetTarget (this);
	fEncodings->Menu()->SetTargetForItems (this);
	fEncodings->ResizeTo(Bounds().Width() - 15, fEncodings->Bounds().Height());
	fEncodings->SetDivider(StringWidth("Encoding: ") + 5);
	fEncodings->MoveTo (fQueryMsg->Frame().left + 5, fQueryMsg->Frame().bottom + 5);
	SetEncodingItem(vision_app->GetInt32("encoding"));
	BView::AllAttached();
}

void
AppWindowPrefsView::SetEncodingItem(int32 encoding)
{
	BMenuItem *item (NULL);
	for (int32 i = 0; i < fEncodings->Menu()->CountItems(); i++)
	{
		item = fEncodings->Menu()->ItemAt(i);
		if (item->Message()->FindInt32("encoding") == encoding)
		{
			item->SetMarked(true);
			break;
		}
	}
}

BMenu *
AppWindowPrefsView::CreateEncodingMenu(void)
{
	BMessage msg(M_APPWINDOWPREFS_ENCODING_CHANGED);
	BMenu *encMenu (new BMenu("Encodings"));
	msg.AddInt32("encoding", B_ISO1_CONVERSION);
	encMenu->AddItem (new BMenuItem("Western (ISO 8859-1)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO2_CONVERSION);
	encMenu->AddItem (new BMenuItem("Central European (ISO 8859-2)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO5_CONVERSION);
	encMenu->AddItem (new BMenuItem("Cyrillic (ISO 8859-5)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_KOI8R_CONVERSION);
	encMenu->AddItem (new BMenuItem("Cyrillic (KOI8-R)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO13_CONVERSION);
	encMenu->AddItem (new BMenuItem("Baltic (ISO 8859-13)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_DOS_866_CONVERSION);
	encMenu->AddItem (new BMenuItem("Cyrillic (MS-DOS 866)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_WINDOWS_1251_CONVERSION);
	encMenu->AddItem (new BMenuItem("Cyrillic (Windows 1251)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO7_CONVERSION);
	encMenu->AddItem (new BMenuItem("Greek (ISO 8859-7)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_SJIS_CONVERSION);
	encMenu->AddItem (new BMenuItem("Japanese (Shift-JIS)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_EUC_CONVERSION);
	encMenu->AddItem (new BMenuItem("Japanese (EUC)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_JIS_CONVERSION);
	encMenu->AddItem (new BMenuItem("Japanese (JIS)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_EUC_KR_CONVERSION);
	encMenu->AddItem (new BMenuItem("Korean (EUC)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_UNICODE_CONVERSION);
	encMenu->AddItem (new BMenuItem("Unicode (UTF-8)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MAC_ROMAN_CONVERSION);
	encMenu->AddItem (new BMenuItem("Western (Mac Roman)", new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_WINDOWS_CONVERSION);
	encMenu->AddItem (new BMenuItem("Western (Windows)", new BMessage(msg)));
	return encMenu;
}

void
AppWindowPrefsView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_APPWINDOWPREFS_ENCODING_CHANGED:
			{
				BMenuItem *source (NULL);
				msg->FindPointer ("source", reinterpret_cast<void **>(&source));
				source->SetMarked(true);
				int32 encoding (msg->FindInt32("encoding"));
				vision_app->SetInt32("encoding", encoding);
			}
			break;
			
		case M_APPWINDOWPREFS_SETTING_CHANGED:
			{
				BControl *source (NULL);
				msg->FindPointer ("source", reinterpret_cast<void **>(&source));
				BString setting;
				msg->FindString ("setting", &setting);
				int32 value (source->Value() == B_CONTROL_ON);
				if ((setting.ICompare ("versionParanoid") == 0))
					value = !value;
				vision_app->SetBool (setting.String(), value);
			}
			break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}
