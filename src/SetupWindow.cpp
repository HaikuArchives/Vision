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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Todd Lair
 *                 Rene Gollent
 */

#include <Button.h>
#include <Bitmap.h>
#include <View.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <TranslationUtils.h>
#include <LayoutBuilder.h>

#include "ClientWindow.h"
#include "SetupWindow.h"
#include "Vision.h"
#include "NetworkMenu.h"
#include "ClickView.h"

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SetupWindow"

SetupWindow::SetupWindow(void)
	:
	BWindow(BRect(108, 88, 500, 320), B_TRANSLATE("Setup window"),
		B_TITLED_WINDOW,
		B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE |
		B_AUTO_UPDATE_SIZE_LIMITS)
{
	AddShortcut('/', B_SHIFT_KEY, new BMessage(M_PREFS_SHOW));

	BBitmap* bmp = NULL;
	ClickView* logo = new ClickView("image", B_WILL_DRAW,
		"https://github.com/HaikuArchives/Vision");

	BMenu* netMenu(new NetworkMenu(B_TRANSLATE("Choose network"),
		M_SETUP_CHOOSE_NETWORK, BMessenger(this)));
	netMenu->SetLabelFromMarked(true);

	BString text(B_TRANSLATE("Network:"));
	text.Append(" ");
	netList = new BMenuField("Network List", text.String(), netMenu);
	//netList->SetDivider(be_plain_font->StringWidth(text.String()) + 5);

	connectButton = new BButton("connect", B_TRANSLATE("Connect"),
		new BMessage(M_CONNECT_NETWORK));

	netPrefsButton = new BButton("netprefs",
		B_TRANSLATE("Network setup" B_UTF8_ELLIPSIS),
		new BMessage(M_NETWORK_SHOW));
	netPrefsButton->SetTarget(vision_app);

	prefsButton = new BButton("prefs",
		B_TRANSLATE("Preferences" B_UTF8_ELLIPSIS), new BMessage(M_PREFS_SHOW));
	prefsButton->SetTarget(vision_app);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_HALF_ITEM_INSETS)
		.Add(logo)
		.Add(netList)
		.AddGroup(B_HORIZONTAL)
			.Add(connectButton)
			.Add(netPrefsButton)
			.Add(prefsButton)
		.End()
	.End();

	if ((bmp = BTranslationUtils::GetBitmap('bits', "vision-logo")) != 0) {
		logo->SetViewBitmap(bmp);
		logo->SetExplicitMinSize(bmp->Bounds().Size());
		logo->SetExplicitMaxSize(bmp->Bounds().Size());
		delete bmp;
	}

	connectButton->SetEnabled(false);

	BRect rect = vision_app->GetRect("SetupWinRect");
	if (rect.Width() > 0)
		MoveTo(rect.LeftTop());
	ResizeTo(0, 0);
}

SetupWindow::~SetupWindow(void)
{
}

bool SetupWindow::QuitRequested(void)
{
	vision_app->SetRect("SetupWinRect", Frame());
	be_app_messenger.SendMessage(M_SETUP_CLOSE);
	return true;
}


void SetupWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_SETUP_CHOOSE_NETWORK: {
		BMenuItem* item(NULL);
		msg->FindPointer("source", reinterpret_cast<void**>(&item));
		if (item && vision_app->CheckNetworkValid(item->Label()))
			connectButton->SetEnabled(true);
		else
			connectButton->SetEnabled(false);
	} break;

	case M_CONNECT_NETWORK: {
		BMessage connMsg(M_CONNECT_NETWORK);
		connMsg.AddString("network", netList->MenuItem()->Label());
		be_app_messenger.SendMessage(&connMsg);
	} break;

	case M_PREFS_SHOW: {
		// forwarding Cmd+Shift+/ message
		be_app_messenger.SendMessage(msg);
	} break;

	default:
		BWindow::MessageReceived(msg);
	}
}
