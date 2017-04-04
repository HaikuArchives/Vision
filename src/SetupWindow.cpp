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
#include <Resources.h>
#include <TranslationUtils.h>

#include "ClickView.h"
#include "ClientWindow.h"
#include "LogoView.h"
#include "SetupWindow.h"
#include "Vision.h"
#include "NetworkMenu.h"

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "SetupWindow"

SetupWindow::SetupWindow(void)
	: BWindow(BRect(108.0, 88.0, 500.0, 320.0), B_TRANSLATE("Setup window"), B_TITLED_WINDOW,
			  B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{

	AddShortcut('/', B_SHIFT_KEY, new BMessage(M_PREFS_SHOW));

	bgView = new BView(Bounds(), "background", B_FOLLOW_ALL, B_WILL_DRAW);
	AddChild(bgView);
	bgView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BRect rect = Bounds();
	LogoView* logo = new LogoView(rect);

	rect.top = logo->PreferredHeight();
	rect.bottom = Bounds().bottom;
	rect.left = kItemSpacing;

	BMenu* netMenu(new NetworkMenu(B_TRANSLATE("Choose network"), M_SETUP_CHOOSE_NETWORK, BMessenger(this)));
	netMenu->SetLabelFromMarked(true);
	netList = new BMenuField(rect, "Network List", B_TRANSLATE("Network: "), netMenu);
	netList->ResizeToPreferred();
	netList->SetDivider(be_plain_font->StringWidth(B_TRANSLATE("Network: ")) + 5);

	rect = netList->Frame();
	rect.OffsetBy(0, rect.Height() + kItemSpacing);
	connectButton = new BButton(rect, "connect", B_TRANSLATE("Connect"),
								new BMessage(M_CONNECT_NETWORK));
	connectButton->ResizeToPreferred();

	rect = connectButton->Frame();
	rect.OffsetBy(rect.Width() + kItemSpacing, 0);

	netPrefsButton = new BButton(rect, "netprefs", B_TRANSLATE("Network setup" B_UTF8_ELLIPSIS),
								 new BMessage(M_NETWORK_SHOW));
	netPrefsButton->ResizeToPreferred();
	netPrefsButton->SetTarget(vision_app);

	rect = netPrefsButton->Frame();
	rect.OffsetBy(rect.Width() + kItemSpacing, 0);

	prefsButton = new BButton(rect, "prefs", B_TRANSLATE("Preferences" B_UTF8_ELLIPSIS),
							  new BMessage(M_PREFS_SHOW));
	prefsButton->ResizeToPreferred();
	prefsButton->SetTarget(vision_app);

	rect = prefsButton->Frame();
	float newWidth = rect.right + kItemSpacing;
	float newHeight = rect.bottom + kItemSpacing;

	ResizeTo(newWidth, newHeight);

	bgView->AddChild(logo);
	bgView->AddChild(netList);
	bgView->AddChild(prefsButton);
	bgView->AddChild(netPrefsButton);
	bgView->AddChild(connectButton);

	rect = Bounds();
	rect.bottom = netList->Frame().bottom - (kItemSpacing * 2);
	logo->ResizeTo(newWidth, rect.Height());

	connectButton->SetEnabled(false);

	rect = vision_app->GetRect("SetupWinRect");
	if (rect.Width() > 0)
		MoveTo(rect.LeftTop());
}

SetupWindow::~SetupWindow(void)
{
	//
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
