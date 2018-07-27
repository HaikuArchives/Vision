#include <Autolock.h>
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <StringView.h>
#include <SupportDefs.h>

#include <stdio.h>

#include "NetPrefsServerView.h"
#include "NetworkWindow.h"
#include "ServerEntryWindow.h"
#include "ColumnListView.h"
#include "ColumnTypes.h"
#include "Vision.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NetPrefsServerView"

// server window

NetPrefsServerView::NetPrefsServerView(BRect bounds, const char* name, BMessenger target)
	: BView(bounds, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW), fEntryWin(NULL), fNetWin(target)
{
	AdoptSystemColors();

	fServerList = new BColumnListView("fServerList", B_WILL_DRAW | B_NAVIGABLE, B_PLAIN_BORDER);
	fServerList->SetSelectionMessage(new BMessage(M_SERVER_ITEM_SELECTED));

	const float columnTitleOffset = 8;
	const float statusInitialSize = be_plain_font->StringWidth(B_TRANSLATE("Status")) + columnTitleOffset * 2;
	BStringColumn* status(new BStringColumn(B_TRANSLATE("Status"), statusInitialSize, statusInitialSize,
		bounds.Width(), B_TRUNCATE_END));
	fServerList->AddColumn(status, 0);

	const float serverInitialSize = be_plain_font->StringWidth(B_TRANSLATE("Server")) + columnTitleOffset * 2;
	BStringColumn* data(new BStringColumn(B_TRANSLATE("Server"),
		serverInitialSize, serverInitialSize, bounds.Width(), B_TRUNCATE_END));
	fServerList->AddColumn(data, 1);

	const float portInitialSize = be_plain_font->StringWidth(B_TRANSLATE("Port")) + columnTitleOffset * 2;
	BStringColumn* port(new BStringColumn(B_TRANSLATE("Port"), portInitialSize, portInitialSize,
		bounds.Width(), B_TRUNCATE_END));
	fServerList->AddColumn(port, 2);

	const float secureInitialSize = be_plain_font->StringWidth(B_TRANSLATE("Secure")) + columnTitleOffset * 2;
	BStringColumn* secure(new BStringColumn(
		B_TRANSLATE("Secure"), secureInitialSize, secureInitialSize, bounds.Width(), B_TRUNCATE_END));
	fServerList->AddColumn(secure, 3);

	fAddButton = new BButton(NULL, B_TRANSLATE("Add" B_UTF8_ELLIPSIS),
		new BMessage(M_SERVER_ADD_ITEM));
	fRemoveButton = new BButton(NULL, B_TRANSLATE("Remove"),
		new BMessage(M_SERVER_REMOVE_ITEM));
	fEditButton = new BButton(NULL, B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		 new BMessage(M_SERVER_EDIT_ITEM));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
		.Add(fServerList)
		.AddGroup(B_HORIZONTAL, 0)
			.AddGlue()
			.Add(fAddButton)
			.Add(fRemoveButton)
			.Add(fEditButton)
		.End()
	.End();
}

NetPrefsServerView::~NetPrefsServerView()
{
	BMessenger msgr(fEntryWin);
	if (msgr.IsValid()) msgr.SendMessage(B_QUIT_REQUESTED);
}

void NetPrefsServerView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fAddButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
	fServerList->SetTarget(this);
	fEditButton->SetEnabled(false);
	fRemoveButton->SetEnabled(false);
}

void NetPrefsServerView::AddServer(const ServerData* data)
{
	BAutolock lock(Looper());
	if (!lock.IsLocked()) return;

	BRow* row(new BRow);
	switch (data->state) {
	case SERVER_PRIMARY:
		row->SetField(new BStringField(B_TRANSLATE("Primary")), 0);
		break;

	case SERVER_SECONDARY:
		row->SetField(new BStringField(B_TRANSLATE("Secondary")), 0);
		break;

	case SERVER_DISABLED:
		row->SetField(new BStringField(B_TRANSLATE("Disabled")), 0);
		break;
	}
	BString server("");
	server = data->serverName;
	BStringField* serverField(new BStringField(server.String()));
	row->SetField(serverField, 1);
	server = "";
	server << data->port;
	BStringField* portField(new BStringField(server.String()));
	row->SetField(portField, 2);
	BStringField* secureField(new BStringField(data->secure ?
		B_TRANSLATE("Yes") : ""));
	row->SetField(secureField, 3);
	fServerList->AddRow(row);
}

void NetPrefsServerView::RemoveServer()
{
	BAutolock lock(Looper());

	if (!lock.IsLocked()) return;

	BRow* row(fServerList->CurrentSelection());
	if (row) {
		BStringField* field((BStringField*)row->GetField(1));

		int32 count;
		ssize_t size;
		type_code type;
		fActiveNetwork->GetInfo("server", &type, &count);

		const ServerData* data;
		for (int32 i = 0; i < count; i++) {
			fActiveNetwork->FindData("server", B_ANY_TYPE, i, reinterpret_cast<const void**>(&data),
									 &size);

			if (!strcmp(data->serverName, field->String())) {
				fActiveNetwork->RemoveData("server", i);
				break;
			}
		}
		fServerList->RemoveRow(row);
		delete row;
	}
}

void NetPrefsServerView::UpdateNetworkData(const ServerData* newServer)
{
	if (newServer == NULL) return;
	type_code type;
	int32 count;
	ssize_t size;
	fActiveNetwork->GetInfo("server", &type, &count);
	const ServerData* data(NULL);
	for (int32 i = 0; i < count; i++) {
		fActiveNetwork->FindData("server", B_ANY_TYPE, i, reinterpret_cast<const void**>(&data),
								 &size);
		if (!strcmp(newServer->serverName, data->serverName)) {
			fActiveNetwork->ReplaceData("server", B_RAW_TYPE, i, newServer, sizeof(ServerData));
			return;
		}
	}
	fActiveNetwork->AddData("server", B_RAW_TYPE, newServer, sizeof(ServerData));
}

void NetPrefsServerView::SetNetworkData(BMessage* msg)
{
	// this shouldn't theoretically be able to happen but better safe than sorry
	BLooper* looper(Looper());
	if (looper == NULL) return;

	BAutolock lock(Looper());
	if (!lock.IsLocked()) return;
	// clear previous servers (if any)
	while (fServerList->CountRows() > 0) {
		BRow* row(fServerList->RowAt(0));
		fServerList->RemoveRow(row);
		delete row;
	}

	BString netString(B_TRANSLATE("Servers for %name%"));
	netString.ReplaceFirst("%name%", msg->FindString("name"));
	type_code type;
	int32 count;
	ssize_t size;
	const ServerData* data;
	msg->GetInfo("server", &type, &count);
	for (int32 i = 0; i < count; i++) {
		msg->FindData("server", B_ANY_TYPE, i, reinterpret_cast<const void**>(&data), &size);
		AddServer(data);
	}
	fActiveNetwork = msg;
	fServerList->ResizeAllColumnsToPreferred();
	Window()->SetTitle(netString.String());
}

void NetPrefsServerView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_SERVER_ITEM_SELECTED: {
		BRow* row(fServerList->CurrentSelection());
		if (row) {
			fEditButton->SetEnabled(true);
			fRemoveButton->SetEnabled(true);
		} else {
			fEditButton->SetEnabled(false);
			fRemoveButton->SetEnabled(false);
		}
	} break;

	case M_SERVER_ADD_ITEM: {
		BMessenger msgr(fEntryWin);
		if (msgr.IsValid())
			fEntryWin->Activate();
		else {
			fEntryWin = new ServerEntryWindow(this, new BMessage(M_SERVER_RECV_DATA), NULL, 0);
			fEntryWin->Show();
		}
	} break;

	case M_SERVER_EDIT_ITEM: {
		BMessenger msgr(fEntryWin);
		if (msgr.IsValid())
			fEntryWin->Activate();
		else {
			BRow* row(fServerList->CurrentSelection());
			if (!row) break;
			int32 count(0);
			ssize_t size(0);
			type_code type;
			fActiveNetwork->GetInfo("server", &type, &count);
			const ServerData* compData;
			for (int32 i = 0; i < count; i++) {
				fActiveNetwork->FindData("server", B_RAW_TYPE, i,
										 reinterpret_cast<const void**>(&compData), &size);
				if (!strcmp(compData->serverName, ((BStringField*)row->GetField(1))->String()))
					break;
			}
			BMessage* invoke(new BMessage(M_SERVER_RECV_DATA));
			invoke->AddBool("edit", true);
			fEntryWin = new ServerEntryWindow(this, invoke, compData, size);
			fEntryWin->SetTitle(B_TRANSLATE("Edit server"));
			fEntryWin->Show();
		}
	} break;

	case M_SERVER_REMOVE_ITEM: {
		RemoveServer();
		fNetWin.SendMessage(M_SERVER_DATA_CHANGED);
	} break;

	case M_SERVER_RECV_DATA: {
		const ServerData* data;
		ssize_t size;
		Window()->DisableUpdates();
		msg->FindData("server", B_RAW_TYPE, reinterpret_cast<const void**>(&data), &size);
		if (msg->HasBool("edit")) RemoveServer();
		UpdateNetworkData(data);
		AddServer(data);
		Window()->EnableUpdates();
		fNetWin.SendMessage(M_SERVER_DATA_CHANGED);
	} break;

	default:
		BView::MessageReceived(msg);
		break;
	}
}
