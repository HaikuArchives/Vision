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
 */

#include "PrefApp.h"
#include "Vision.h"

#include <stdio.h>

#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <UTF8.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefApp"

AppWindowPrefsView::AppWindowPrefsView()
	: BView("App/Window Prefs", 0)
{
	AdoptSystemColors();

	BMessage msg(M_APPWINDOWPREFS_SETTING_CHANGED);

	msg.AddString("setting", "versionParanoid");
	fVersionParanoid = new BCheckBox(
		"version Paranoid", B_TRANSLATE("Show OS information in version reply"), new BMessage(msg));
	fVersionParanoid->SetValue(
		(!vision_app->GetBool("versionParanoid")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "catchAltW");
	fCatchAltW = new BCheckBox(
		"catch AltW", B_TRANSLATE("Require double CMD+Q/W to close"), new BMessage(msg));
	fCatchAltW->SetValue((vision_app->GetBool("catchAltW")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "stripcolors");
	fStripColors
		= new BCheckBox("stripcolors", B_TRANSLATE("Strip mIRC colors"), new BMessage(msg));
	fStripColors->SetValue((vision_app->GetBool("stripcolors")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "Newbie spam mode");
	fSpamMode = new BCheckBox(
		"newbiespammode", B_TRANSLATE("Warn when multiline pasting"), new BMessage(msg));
	fSpamMode->SetValue((vision_app->GetBool("Newbie Spam Mode")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "queryOnMsg");
	fQueryMsg = new BCheckBox(
		"queryOnMsg", B_TRANSLATE("Open new query on private message"), new BMessage(msg));
	fQueryMsg->SetValue((vision_app->GetBool("queryOnMsg")) ? B_CONTROL_ON : B_CONTROL_OFF);

	BMenu* encMenu(CreateEncodingMenu());

	BString text(B_TRANSLATE("Encoding:"));
	text.Append(" ");
	fEncodings = new BMenuField("encoding", text.String(), encMenu);

	fEncodings->Menu()->SetLabelFromMarked(true);

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fVersionParanoid)
		.Add(fCatchAltW)
		.Add(fStripColors)
		.Add(fSpamMode)
		.Add(fQueryMsg)
		.Add(fEncodings)
		.AddGlue()
	.End();
	// clang-format on
}

AppWindowPrefsView::~AppWindowPrefsView() {}

void
AppWindowPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
}

void
AppWindowPrefsView::AllAttached()
{
	fVersionParanoid->SetTarget(this);
	fCatchAltW->SetTarget(this);
	fStripColors->SetTarget(this);
	fSpamMode->SetTarget(this);
	fQueryMsg->SetTarget(this);
	fEncodings->Menu()->SetTargetForItems(this);
	SetEncodingItem(vision_app->GetInt32("encoding"));
	BView::AllAttached();
}

void
AppWindowPrefsView::SetEncodingItem(int32 encoding)
{
	BMenuItem* item(NULL);
	for (int32 i = 0; i < fEncodings->Menu()->CountItems(); i++) {
		item = fEncodings->Menu()->ItemAt(i);
		if (item->Message()->FindInt32("encoding") == encoding) {
			item->SetMarked(true);
			break;
		}
	}
}

BMenu*
AppWindowPrefsView::CreateEncodingMenu()
{
	BMessage msg(M_APPWINDOWPREFS_ENCODING_CHANGED);
	BMenu* encMenu(new BMenu("Encodings"));
	msg.AddInt32("encoding", B_ISO1_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Western (ISO 8859-1)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO2_CONVERSION);
	encMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Central European (ISO 8859-2)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO5_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Cyrillic (ISO 8859-5)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_KOI8R_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Cyrillic (KOI8-R)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO13_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Baltic (ISO 8859-13)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_DOS_866_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Cyrillic (MS-DOS 866)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_WINDOWS_1251_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Cyrillic (Windows 1251)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_ISO7_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Greek (ISO 8859-7)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_SJIS_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Japanese (Shift-JIS)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_EUC_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Japanese (EUC)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_JIS_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Japanese (JIS)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_EUC_KR_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Korean (EUC)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_UNICODE_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Unicode (UTF-8)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MAC_ROMAN_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Western (Mac Roman)"), new BMessage(msg)));
	msg.ReplaceInt32("encoding", B_MS_WINDOWS_CONVERSION);
	encMenu->AddItem(new BMenuItem(B_TRANSLATE("Western (Windows)"), new BMessage(msg)));
	return encMenu;
}

void
AppWindowPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_APPWINDOWPREFS_ENCODING_CHANGED:
		{
			BMenuItem* source(NULL);
			msg->FindPointer("source", reinterpret_cast<void**>(&source));
			source->SetMarked(true);
			int32 encoding(msg->FindInt32("encoding"));
			vision_app->SetInt32("encoding", encoding);
		} break;

		case M_APPWINDOWPREFS_SETTING_CHANGED:
		{
			BControl* source(NULL);
			msg->FindPointer("source", reinterpret_cast<void**>(&source));
			BString setting;
			msg->FindString("setting", &setting);
			int32 value(source->Value() == B_CONTROL_ON);
			if ((setting.ICompare("versionParanoid") == 0))
				value = !value;
			vision_app->SetBool(setting.String(), value);
		} break;
		default:
			BView::MessageReceived(msg);
			break;
	}
}
