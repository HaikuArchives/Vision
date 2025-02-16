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

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <SeparatorView.h>
#include <TextControl.h>
#include <Window.h>

#include "NumericFilter.h"
#include "ServerEntryWindow.h"
#include "Vision.h"

#include <stdio.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerEntryWindow"

ServerEntryWindow::ServerEntryWindow(
	BHandler* handler, BMessage* invoked, const ServerData* data, int32 size)
	: BWindow(BRect(50, 50, 150, 150), B_TRANSLATE("Add server"), B_TITLED_WINDOW,
		  B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS),
	  invocation(invoked),
	  target(handler)
{
	ASSERT(handler != NULL);
	memset(&currentServer, 0, sizeof(ServerData));
	if (size != 0)
		memcpy(&currentServer, data, size);

	BString text(B_TRANSLATE("Server:"));
	text.Append(" ");
	serverName = new BTextControl("serverName", text.String(), (data) ? data->serverName : "",
		new BMessage(M_SERVER_NAME_CHANGED));
	BString strPort("");
	if (data)
		strPort << data->port;
	else
		strPort << 6667;

	text = B_TRANSLATE("Port:");
	text.Append(" ");
	port = new BTextControl(
		"portVal", text.String(), strPort.String(), new BMessage(M_SERVER_PORT_CHANGED));
	port->SetDivider(be_plain_font->StringWidth(text.String()) + 5);

	BMenu* stateMenu = new BMenu(B_TRANSLATE("Choose status"));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Primary"), new BMessage(M_SERVER_STATE)));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Secondary"), new BMessage(M_SERVER_STATE)));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Disabled"), new BMessage(M_SERVER_STATE)));
	stateMenu->SetRadioMode(true);
	stateMenu->SetLabelFromMarked(true);

	float menuFieldSize = be_plain_font->StringWidth(B_TRANSLATE("Primary"));
	float secondarySize = be_plain_font->StringWidth(B_TRANSLATE("Secondary"));
	float disabledSize = be_plain_font->StringWidth(B_TRANSLATE("Disabled"));

	if (secondarySize > disabledSize) {
		if (disabledSize > menuFieldSize)
			menuFieldSize = secondarySize;
	} else {
		if (secondarySize > menuFieldSize)
			menuFieldSize = disabledSize;
	}

	text = B_TRANSLATE("State:");
	text.Append(" ");
	statusField = new BMenuField("states", text.String(), stateMenu, B_WILL_DRAW | B_NAVIGABLE);
	statusField->CreateMenuBarLayoutItem()->SetExplicitSize(
		BSize(menuFieldSize + 30, B_SIZE_UNSET));

	okButton = new BButton(
		"serverOk", B_TRANSLATE("Done"), new BMessage(M_SERVER_DONE), B_WILL_DRAW | B_NAVIGABLE);

	cancelButton = new BButton("serverCancel", B_TRANSLATE("Cancel"), new BMessage(M_SERVER_CANCEL),
		B_WILL_DRAW | B_NAVIGABLE);

	BString password("");
	if (strlen(currentServer.password) > 0)
		password = currentServer.password;

	text = B_TRANSLATE("Use password:");
	text.Append(" ");
	usePassword = new BCheckBox(
		"usePass", text.String(), new BMessage(M_SERVER_USEPASS), B_WILL_DRAW | B_NAVIGABLE);

	text = B_TRANSLATE("Secure port");
	securePort = new BCheckBox(
		"securePort", text.String(), new BMessage(M_SERVER_SECUREPORT), B_WILL_DRAW | B_NAVIGABLE);

	passwordField
		= new BTextControl("password", NULL, password.String(), NULL, B_WILL_DRAW | B_NAVIGABLE);

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_HALF_ITEM_SPACING)
		.SetInsets(0)
		.AddGrid()
			.SetInsets(B_USE_WINDOW_INSETS)
			.Add(serverName->CreateLabelLayoutItem(), 0, 0)
			.Add(serverName->CreateTextViewLayoutItem(), 1, 0)
			.Add(port->CreateLabelLayoutItem(), 0, 1)
			.Add(port->CreateTextViewLayoutItem(), 1, 1)
			.Add(securePort, 0, 2, 2)
			.Add(usePassword, 0, 3)
			.Add(passwordField, 1, 3)
			.Add(statusField->CreateLabelLayoutItem(), 0, 4)
			.Add(statusField->CreateMenuBarLayoutItem(), 1, 4)
		.End()
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(
				B_USE_WINDOW_INSETS, B_USE_HALF_ITEM_SPACING, B_USE_WINDOW_INSETS,
				B_USE_WINDOW_INSETS)
			.AddGlue()
			.Add(cancelButton)
			.Add(okButton)
		.End()
	.End();
	// clang-format on

	serverName->SetTarget(this);

	port->SetTarget(this);
	port->TextView()->AddFilter(new NumericFilter());

	statusField->Menu()->SetTargetForItems(this);

	usePassword->SetTarget(this);
	usePassword->SetValue((strlen(currentServer.password) > 0) ? B_CONTROL_ON : B_CONTROL_OFF);
	passwordField->SetEnabled(usePassword->Value() == B_CONTROL_ON);
	passwordField->TextView()->HideTyping(true);
	// HideTyping erases the text from the field, so set the password only now.
	passwordField->TextView()->SetText(password.String());

	securePort->SetTarget(this);
	securePort->SetValue(currentServer.secure ? B_CONTROL_ON : B_CONTROL_OFF);

	cancelButton->SetTarget(this);
	okButton->SetTarget(this);

	okButton->SetEnabled(false);

	dynamic_cast<BInvoker*>(statusField->Menu()->ItemAt(currentServer.state))->Invoke();

	port->MakeFocus(false);
	port->MakeFocus(true);
	serverName->MakeFocus(false);
	serverName->MakeFocus(true);
}

ServerEntryWindow::~ServerEntryWindow()
{
	delete invocation;
}

void
ServerEntryWindow::CheckDoneState()
{
	if (serverName->TextView()->TextLength() != 0) {
		if (port->TextView()->TextLength() != 0) {
			if (statusField->Menu()->FindMarked() != NULL) {
				okButton->SetEnabled(true);
				return;
			}
		}
	}
	okButton->SetEnabled(false);
}

void
ServerEntryWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_SERVER_STATE:
		{
			// use SetRadioMode and SetLabelFromMarked
		}

		case M_SERVER_NAME_CHANGED:
		case M_SERVER_PORT_CHANGED:
			CheckDoneState();
			break;

		case M_SERVER_USEPASS:
		{
			passwordField->SetEnabled(usePassword->Value() == B_CONTROL_ON);
		} break;

		case M_SERVER_DONE:
		{
			BMenu* menu(statusField->Menu());
			ServerData data;
			memset(&data, 0, sizeof(ServerData));
			strcpy(data.serverName, serverName->Text());
			data.port = atoi(port->Text());
			data.state = menu->IndexOf(menu->FindMarked());
			data.secure = securePort->Value() == B_CONTROL_ON;
			if (usePassword->Value() == B_CONTROL_ON)
				strcpy(data.password, passwordField->TextView()->Text());
			BMessenger msgr(target);
			BMessage msg(*invocation);
			msg.AddData("server", B_RAW_TYPE, &data, sizeof(data));
			msgr.SendMessage(&msg);
		}

		case M_SERVER_CANCEL:
			PostMessage(B_QUIT_REQUESTED);
			break;

		default:
			BWindow::MessageReceived(msg);
	}
}
