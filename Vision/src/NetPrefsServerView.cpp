#include <Box.h>
#include <Button.h>
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

const uint32 M_SERVER_ADD_ITEM = 'msai';
const uint32 M_SERVER_EDIT_ITEM = 'msae';
const uint32 M_SERVER_REMOVE_ITEM = 'msri';
const uint32 M_SERVER_RECV_DATA = 'msrd';
const uint32 M_SERVER_ITEM_SELECTED = 'msis';

const rgb_color serverItemNormalColor = { 0,0,0,255 };
const rgb_color serverItemDefaultColor = { 0, 127, 0, 255 };
const rgb_color serverItemDisabledColor = { 0, 0, 192, 255 };

class ServerListItem : public BStringItem
{
	public:
		ServerListItem (ServerData &);
		~ServerListItem (void);
		virtual void DrawItem (BView *, BRect, bool);
		void SetState (uint32);
		void SetServer (const char *);
		void SetPort (uint32);
		
		ServerData GetServerInfo (void);
	
	private:
		
		void UpdateItemText (void);
		inline rgb_color GetItemColor (void);
		
		uint32 state;
		BString serverName;
		uint32 port;
};

ServerListItem::ServerListItem (ServerData &data)
	: BStringItem ("")
{
	state = data.state;
	serverName = data.serverName;
	port = data.port;
	UpdateItemText();
}

ServerListItem::~ServerListItem (void)
{
}

void
ServerListItem::UpdateItemText(void)
{
	BString itemText;
	itemText += serverName;
	itemText += ":";
	itemText << port;
	SetText (itemText.String());
}

inline rgb_color
ServerListItem::GetItemColor (void)
{
	switch (state)
	{
		case 1:
			return serverItemDefaultColor;
		
		case 2:
			return serverItemDisabledColor;
	}
	
	return serverItemNormalColor;
}

void
ServerListItem::DrawItem (BView *parent, BRect frame, bool complete)
{
	parent->SetHighColor (GetItemColor());
	BStringItem::DrawItem(parent, frame, complete);
}

void
ServerListItem::SetState (uint32 newState)
{
	state = newState;
	UpdateItemText();
}

void
ServerListItem::SetServer (const char *newServer)
{
	serverName = newServer;
	UpdateItemText();
}

void
ServerListItem::SetPort (uint32 newPort)
{
	port = newPort;
	UpdateItemText();
}

ServerData
ServerListItem::GetServerInfo (void)
{
	ServerData data;
	data.port = port;
	data.state = state;
	strcpy (data.serverName, serverName.String());
	return data;
}
// server window

NetPrefsServerView::NetPrefsServerView (BRect bounds, const char *name, BMessenger target)
	:	BView (
			bounds,
			name,
			B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW),
			entryWin (NULL),
			netWin (target)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BRect boundsRect (Bounds());
	BBox *mainBox (new BBox (bounds.InsetByCopy (-1, -1), NULL, B_FOLLOW_ALL_SIDES));
	AddChild (mainBox);
	selectTitleString = new BStringView (BRect (0,0,0,0), NULL, "Select servers for");
	selectTitleString->ResizeToPreferred();
	mainBox->AddChild (selectTitleString);
	selectTitleString->MoveTo (11,11);
	serverList = new BColumnListView (BRect (0, 0, boundsRect.Width() - 10,
		boundsRect.Height() / 2), "serverList", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW,
		B_PLAIN_BORDER);
	serverList->SetSelectionMessage (new BMessage (M_SERVER_ITEM_SELECTED));
	mainBox->AddChild (serverList);
	serverList->MoveTo (5, selectTitleString->Frame().bottom + 3);
	BStringColumn *status (new BStringColumn ("Status", be_plain_font->StringWidth ("Status") * 2,
		0, bounds.Width(), 0, B_ALIGN_CENTER));
	serverList->AddColumn (status, 0);
	BStringColumn *data (new BStringColumn ("Server", be_plain_font->StringWidth ("Server") * 2,
		0, bounds.Width(), 0));
	serverList->AddColumn (data, 1);
	BStringColumn *port (new BStringColumn ("Port", be_plain_font->StringWidth ("Port") * 2,
		0, bounds.Width(), 0));
	serverList->AddColumn (port, 2);
	addButton = new BButton (BRect (0,0,0,0), NULL, "Add"B_UTF8_ELLIPSIS,
		new BMessage (M_SERVER_ADD_ITEM));
	removeButton = new BButton (BRect (0,0,0,0), NULL, "Remove",
		new BMessage (M_SERVER_REMOVE_ITEM));
	editButton = new BButton (BRect (0,0,0,0), NULL, "Edit"B_UTF8_ELLIPSIS,
		new BMessage (M_SERVER_EDIT_ITEM));
	addButton->ResizeToPreferred();
	removeButton->ResizeToPreferred();
	editButton->ResizeToPreferred();
	removeButton->MoveTo (serverList->Frame().right - removeButton->Frame().Width(),
		serverList->Frame().bottom + 5);
	mainBox->AddChild (removeButton);
	addButton->MoveTo (removeButton->Frame().left - (addButton->Frame().Width() + 5),
		removeButton->Frame().top);
	mainBox->AddChild (addButton);
	editButton->MoveTo (addButton->Frame().left - (editButton->Frame().Width() +15),
		addButton->Frame().top);
	mainBox->AddChild (editButton);
	BStringView *legend1 = new BStringView (BRect (0,0,0,0), "str1", "Key: ");
	legend1->ResizeToPreferred();
	mainBox->AddChild (legend1);
	legend1->MoveTo (serverList->Frame().left + 5, addButton->Frame().bottom + 5);
	BStringView *legend2 = new BStringView (BRect (0,0,0,0), "str1", "  * = primary");
	legend2->ResizeToPreferred();
	mainBox->AddChild (legend2);
	legend2->MoveTo (legend1->Frame().left, legend1->Frame().bottom);
	BStringView *legend3 = new BStringView (BRect (0,0,0,0), "str1", "  + = secondary (fallback)");
	legend3->ResizeToPreferred();
	mainBox->AddChild (legend3);
	legend3->MoveTo (legend2->Frame().left, legend2->Frame().bottom);
	legend4 = new BStringView (BRect (0,0,0,0), "str1", "  - = disabled");
	legend4->ResizeToPreferred();
	mainBox->AddChild (legend4);
	legend4->MoveTo (legend3->Frame().left, legend3->Frame().bottom);
	okButton = new BButton (BRect (0,0,0,0), NULL, "OK",
		new BMessage (B_QUIT_REQUESTED));
	okButton->ResizeToPreferred();
	mainBox->AddChild (okButton);
	okButton->MoveTo (serverList->Frame().right - okButton->Frame().Width(),
	  legend4->Frame().bottom + 5);
}

NetPrefsServerView::~NetPrefsServerView (void)
{
  BMessenger msgr (entryWin);
  if (msgr.IsValid())
    msgr.SendMessage (B_QUIT_REQUESTED);
}

void
NetPrefsServerView::AttachedToWindow (void)
{
	BView::AttachedToWindow();
	addButton->SetTarget (this);
	editButton->SetTarget (this);
	removeButton->SetTarget (this);
	serverList->SetTarget (this);
	editButton->SetEnabled (false);
	removeButton->SetEnabled (false);
	okButton->SetTarget (Window());

	if (okButton->Frame().bottom > Bounds().Height())
	  Window()->ResizeTo (Bounds().Width(), okButton->Frame().bottom + 5);
}

void
NetPrefsServerView::DetachedFromWindow (void)
{
  BView::DetachedFromWindow();
}

void
NetPrefsServerView::AddServer (const ServerData *data)
{
	BRow *row (new BRow);
	switch (data->state)
	{
		case 0:
			row->SetField (new BStringField ("*"), 0);
			break;
		
		case 1:
			row->SetField (new BStringField ("+"), 0);
			break;
		
		case 2:
			row->SetField (new BStringField ("-"), 0);
			break;		
	}
	BString server ("");
	server = data->serverName;
	BStringField *serverField (new BStringField (server.String()));
	row->SetField (serverField, 1);
	server = "";
	server << data->port;
	BStringField *portField (new BStringField (server.String()));
	row->SetField (portField, 2);
	serverList->AddRow (row);
}

void
NetPrefsServerView::RemoveServer ()
{
	BRow *row (serverList->CurrentSelection());
	if (row)
	{
		BStringField *field ((BStringField *)row->GetField (1));
		
		int32 count, size;
		type_code type;
		activeNetwork->GetInfo ("server", &type, &count);
		
		const ServerData *data;
		for (int32 i = 0; i < count; i++)
		{
			activeNetwork->FindData ("server", B_ANY_TYPE, i, reinterpret_cast<const void **>(&data),
			 &size);
			 
			if (!strcmp (data->serverName, field->String()))
			{
				activeNetwork->RemoveData ("server", i);
				break;
			}
		}
		
		serverList->RemoveRow (row);
		delete row;
	}
}

void
NetPrefsServerView::UpdateNetworkData (const ServerData *newServer)
{
	if (newServer == NULL)
		return;
	type_code type;
	int32 count, size;
	activeNetwork->GetInfo ("server", &type, &count);
	const ServerData *data (NULL);
	for (int32 i = 0; i < count; i++)
	{
		activeNetwork->FindData ("server", B_ANY_TYPE, i, reinterpret_cast<const void **>(&data), &size);
		if (!strcmp (newServer->serverName, data->serverName))
		{
			activeNetwork->ReplaceData ("server", B_RAW_TYPE, i, newServer, sizeof(ServerData));
			return;
		}
	}
	activeNetwork->AddData ("server", B_RAW_TYPE, newServer, sizeof (ServerData));
}

void
NetPrefsServerView::SetNetworkData (BMessage *msg)
{
	BString netString ("Select servers for ");
	netString += msg->FindString ("name");
	netString += ":";
	type_code type;
	int32 count, size;
	const ServerData *data;
	msg->GetInfo ("server", &type, &count);
	for (int32 i = 0; i < count; i++)
	{
		msg->FindData ("server", B_ANY_TYPE, i, reinterpret_cast<const void **>(&data), &size);
		AddServer (data);
	}
	activeNetwork = msg;
	selectTitleString->SetText (netString.String());
	selectTitleString->ResizeToPreferred();
}

void
NetPrefsServerView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_SERVER_ITEM_SELECTED:
			{
				BRow *row (serverList->CurrentSelection());
				if (row)
				{
					editButton->SetEnabled (true);
					removeButton->SetEnabled (true);
				}
				else
				{
					editButton->SetEnabled (false);
					removeButton->SetEnabled (false);
				}
			}
			break;
			
		case M_SERVER_ADD_ITEM:
		{
			BMessenger msgr (entryWin);
			if (msgr.IsValid())
			{
				entryWin->Activate();
			}
			else
			{
				entryWin = new ServerEntryWindow(this, new BMessage (M_SERVER_RECV_DATA), NULL);
				entryWin->Show();
			}
		}
		break;

		case M_SERVER_EDIT_ITEM:
		{
			BMessenger msgr (entryWin);
			if (msgr.IsValid())
			{
				entryWin->Activate();
			}
			else
			{
				BRow *row (serverList->CurrentSelection());
				if (!row)
					break;
				int32 count (0), size (0);
				type_code type;
				activeNetwork->GetInfo ("server", &type, &count);
				const ServerData *compData; 
				for (int32 i = 0; i < count; i++)
				{
					activeNetwork->FindData ("server", B_RAW_TYPE, i, reinterpret_cast<const void **>(&compData), &size);
					if (!strcmp (compData->serverName, ((BStringField *)row->GetField(1))->String()))
						break;
				}
				BMessage *invoke (new BMessage (M_SERVER_RECV_DATA));
				invoke->AddBool ("edit", true);
				entryWin = new ServerEntryWindow (this, invoke, compData);
				entryWin->Show();
			}
		}
		break;

		
		case M_SERVER_REMOVE_ITEM:
		{
			RemoveServer();
		}
		break;
		
		case M_SERVER_RECV_DATA:
		{
			const ServerData *data;
			int32 size;
			Window()->DisableUpdates();
			msg->FindData ("server", B_RAW_TYPE, reinterpret_cast<const void **>(&data), &size);
			if (msg->HasBool ("edit"))
				RemoveServer();
			UpdateNetworkData (data);
			AddServer (data);
			Window()->EnableUpdates();
			netWin.SendMessage (M_SERVER_DATA_CHANGED);
			
		}
		break;
		
		default:
			BView::MessageReceived (msg);
			break;
	}
}
