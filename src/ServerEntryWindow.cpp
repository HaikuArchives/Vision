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
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <TextControl.h>
#include <Window.h>
#include <LayoutBuilder.h>

#include "NumericFilter.h"
#include "ServerEntryWindow.h"
#include "Vision.h"

#include <stdio.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerEntryWindow"

ServerEntryWindow::ServerEntryWindow(BHandler* handler, BMessage* invoked,
	const ServerData* data, int32 size)
	: BWindow(BRect(50, 50, 350, 250), B_TRANSLATE("Add server"),
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS |
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS),
	invocation(invoked),
	target(handler)
{
	ASSERT(handler != NULL);
	memset(&currentServer, 0, sizeof(ServerData));
	if (size != 0)
		memcpy(&currentServer, data, size);

	BString text(B_TRANSLATE("Server:"));
	text.Append(" ");
	serverName = new BTextControl(
		"serverName", text.String(), (data) ? data->serverName : "",
		new BMessage(M_SERVER_NAME_CHANGED));
	BString strPort("");
	if (data)
		strPort << data->port;
	else
		strPort << 6667;

	text = B_TRANSLATE("Port:");
	text.Append(" ");
	port = new BTextControl("portVal", text.String(), strPort.String(),
		new BMessage(M_SERVER_PORT_CHANGED));
	port->SetDivider(be_plain_font->StringWidth(text.String()) + 5);

	BMenu* stateMenu = new BMenu(B_TRANSLATE("Choose status"));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Primary"),
		new BMessage(M_SERVER_STATE)));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Secondary"),
		new BMessage(M_SERVER_STATE)));
	stateMenu->AddItem(new BMenuItem(B_TRANSLATE("Disabled"),
		new BMessage(M_SERVER_STATE)));

	text = B_TRANSLATE("State:");
	text.Append(" ");
	statusField = new BMenuField("states",
		text.String(), stateMenu, B_WILL_DRAW | B_NAVIGABLE);

	okButton = new BButton("serverOk", B_TRANSLATE("Done"),
		new BMessage(M_SERVER_DONE), B_WILL_DRAW | B_NAVIGABLE);

	cancelButton = new BButton("serverCancel", B_TRANSLATE("Cancel"),
		new BMessage(M_SERVER_CANCEL), B_WILL_DRAW | B_NAVIGABLE);

	BString password("");
	if (strlen(currentServer.password) > 0)
		password = currentServer.password;

	text = B_TRANSLATE("Use password:");
	text.Append(" ");
	usePassword = new BCheckBox("usePass", text.String(),
		new BMessage(M_SERVER_USEPASS), B_WILL_DRAW | B_NAVIGABLE);

	text = B_TRANSLATE("Secure port:");
	text.Append(" ");
	securePort = new BCheckBox("securePort", text.String(),
		new BMessage(M_SERVER_SECUREPORT), B_WILL_DRAW | B_NAVIGABLE);

	passwordField = new BTextControl("password", NULL, password.String(), NULL,
		B_WILL_DRAW | B_NAVIGABLE);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_HALF_ITEM_INSETS)
		.Add(serverName)
		.Add(port)
		.AddGroup(B_HORIZONTAL)
			.Add(usePassword)
			.Add(passwordField)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(securePort)
			.AddGlue()
		.End()
		.Add(statusField)
		.AddGroup(B_HORIZONTAL, B_USE_HALF_ITEM_SPACING)
			.AddGlue()
			.Add(cancelButton)
			.Add(okButton)
		.End()
	.End();

	text = B_TRANSLATE("Server:");
	text.Append(" ");
	serverName->SetDivider(be_plain_font->StringWidth(text.String()) + 5);
	serverName->SetTarget(this);

	port->SetTarget(this);
	port->TextView()->AddFilter(new NumericFilter());

	ResizeToPreferred();

	statusField->Menu()->SetTargetForItems(this);

	usePassword->SetTarget(this);
	usePassword->SetValue((strlen(currentServer.password) > 0) ? B_CONTROL_ON : B_CONTROL_OFF);
	passwordField->SetEnabled(usePassword->Value() == B_CONTROL_ON);
	passwordField->TextView()->HideTyping(true);

	securePort->SetTarget(this);
	securePort->SetValue(currentServer.secure ? B_CONTROL_ON : B_CONTROL_OFF);

	cancelButton->SetTarget(this);
	okButton->SetTarget(this);

	okButton->SetEnabled(false);

	dynamic_cast<BInvoker*>(
		statusField->Menu()->ItemAt(currentServer.state))->Invoke();

	port->MakeFocus(false);
	port->MakeFocus(true);
	serverName->MakeFocus(false);
	serverName->MakeFocus(true);
}

ServerEntryWindow::~ServerEntryWindow()
{
	delete invocation;
}

void ServerEntryWindow::CheckDoneState()
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

void ServerEntryWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_SERVER_STATE: {
		BMenu* menu(statusField->Menu());
		BMenuItem* item(NULL);
		item = menu->FindMarked();
		if (item) item->SetMarked(false);
		msg->FindPointer("source", reinterpret_cast<void**>(&item));
		item->SetMarked(true);
		statusField->MenuItem()->SetLabel(item->Label());
	}

	case M_SERVER_NAME_CHANGED:
	case M_SERVER_PORT_CHANGED:
		CheckDoneState();
		break;

	case M_SERVER_USEPASS: {
		passwordField->SetEnabled(usePassword->Value() == B_CONTROL_ON);
	} break;

	case M_SERVER_DONE: {
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
