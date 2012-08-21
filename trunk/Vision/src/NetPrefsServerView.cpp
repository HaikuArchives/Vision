#include <Autolock.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
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
#define B_TRANSLATION_CONTEXT "ServerListView"

const rgb_color serverItemNormalColor = {0, 0, 0, 255};
const rgb_color serverItemDefaultColor = {0, 127, 0, 255};
const rgb_color serverItemDisabledColor = {0, 0, 192, 255};

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

ServerListItem::ServerListItem (ServerData & data)
	:	BStringItem ("")
{
	state = data.state;
	serverName = data.serverName;
	port = data.port;
	UpdateItemText ();
}

ServerListItem::~ServerListItem (void)
{
}

void
ServerListItem::UpdateItemText (void)
{
	BString itemText;
	itemText += serverName;
	itemText += ":";
	itemText << port;
	SetText (itemText.String ());
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
ServerListItem::DrawItem (BView * parent, BRect frame, bool complete)
{
	parent->SetHighColor (GetItemColor ());
	BStringItem::DrawItem (parent, frame, complete);
}

void
ServerListItem::SetState (uint32 newState)
{
	state = newState;
	UpdateItemText ();
}

void
ServerListItem::SetServer (const char *newServer)
{
	serverName = newServer;
	UpdateItemText ();
}

void
ServerListItem::SetPort (uint32 newPort)
{
	port = newPort;
	UpdateItemText ();
}

ServerData
ServerListItem::GetServerInfo (void)
{
	ServerData data;
	data.port = port;
	data.state = state;
	strcpy (data.serverName, serverName.String ());
	return data;
}
// server window

NetPrefsServerView::NetPrefsServerView (BRect bounds, const char *name, BMessenger target)
	:	BView (
		bounds,
		name,
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW),
	fEntryWin (NULL),
	fNetWin (target)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BRect boundsRect (Bounds ());
	BBox *mainBox (new BBox (bounds.InsetByCopy (-1, -1), NULL, B_FOLLOW_ALL_SIDES));
	AddChild (mainBox);
	fSelectTitleString = new BStringView (BRect (0, 0, 0, 0), NULL, "Select servers for");
	fSelectTitleString->ResizeToPreferred ();
	mainBox->AddChild (fSelectTitleString);
	fSelectTitleString->MoveTo (11, 11);
	fServerList = new BColumnListView (BRect (0, 0, boundsRect.Width () - 10,
		boundsRect.Height () / 2), "fServerList", B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW,
		B_PLAIN_BORDER);
	fServerList->SetSelectionMessage (new BMessage (M_SERVER_ITEM_SELECTED));
	mainBox->AddChild (fServerList);
	fServerList->MoveTo (5, fSelectTitleString->Frame ().bottom + 3);
	BString itemText = B_TRANSLATE("Status");
	BStringColumn *status (new BStringColumn (itemText.String(), be_plain_font->StringWidth (itemText.String()) * 2,
		0, bounds.Width (), 0, B_ALIGN_CENTER));
	fServerList->AddColumn (status, 0);
	itemText = B_TRANSLATE("Server");
	BStringColumn *data (new BStringColumn (itemText.String(), be_plain_font->StringWidth (itemText.String()) * 2,
		0, bounds.Width (), 0));
	fServerList->AddColumn (data, 1);
	itemText = B_TRANSLATE("Port");
	BStringColumn *port (new BStringColumn (itemText.String(), be_plain_font->StringWidth (itemText.String()) * 2,
		0, bounds.Width (), 0));
	fServerList->AddColumn (port, 2);
	itemText = B_TRANSLATE("Add" B_UTF8_ELLIPSIS);
	fAddButton = new BButton (BRect (0, 0, 0, 0), NULL, itemText.String(),
		new BMessage (M_SERVER_ADD_ITEM));
	fRemoveButton = new BButton (BRect (0, 0, 0, 0), NULL, B_TRANSLATE("Remove"),
		new BMessage (M_SERVER_REMOVE_ITEM));
	itemText = B_TRANSLATE("Edit" B_UTF8_ELLIPSIS);
	fEditButton = new BButton (BRect (0, 0, 0, 0), NULL, itemText.String(),
		new BMessage (M_SERVER_EDIT_ITEM));
	fAddButton->ResizeToPreferred ();
	fRemoveButton->ResizeToPreferred ();
	fEditButton->ResizeToPreferred ();
	fRemoveButton->MoveTo (fServerList->Frame ().right - fRemoveButton->Frame ().Width (),
	 fServerList->Frame ().bottom + 5);
	mainBox->AddChild (fRemoveButton);
	fAddButton->MoveTo (fRemoveButton->Frame ().left - (fAddButton->Frame ().Width () + 5),
		fRemoveButton->Frame ().top);
	mainBox->AddChild (fAddButton);
	fEditButton->MoveTo (fAddButton->Frame ().left - (fEditButton->Frame ().Width () + 15),
		fAddButton->Frame ().top);
	mainBox->AddChild (fEditButton);
	itemText = B_TRANSLATE("Key");
	itemText += ": ";
	BStringView *legend1 = new BStringView (BRect (0, 0, 0, 0), "str1", itemText.String());
	legend1->ResizeToPreferred ();
	mainBox->AddChild (legend1);
	legend1->MoveTo (fServerList->Frame ().left + 5, fAddButton->Frame ().bottom + 5);
	BStringView *legend2 = new BStringView (BRect (0, 0, 0, 0), "str1", B_TRANSLATE("* = primary"));
	legend2->ResizeToPreferred ();
	mainBox->AddChild (legend2);
	legend2->MoveTo (legend1->Frame ().left, legend1->Frame ().bottom);
	BStringView *legend3 = new BStringView (BRect (0, 0, 0, 0), "str1", B_TRANSLATE("+ = secondary (fallback)"));
	legend3->ResizeToPreferred ();
	mainBox->AddChild (legend3);
	legend3->MoveTo (legend2->Frame ().left, legend2->Frame ().bottom);
	fLegend4 = new BStringView (BRect (0, 0, 0, 0), "str1", B_TRANSLATE("- = disabled"));
	fLegend4->ResizeToPreferred ();
	mainBox->AddChild (fLegend4);
	fLegend4->MoveTo (legend3->Frame ().left, legend3->Frame ().bottom);
	fOkButton = new BButton (BRect (0, 0, 0, 0), NULL, B_TRANSLATE("Close"),
		new BMessage (B_QUIT_REQUESTED));
	fOkButton->ResizeToPreferred ();
	mainBox->AddChild (fOkButton);
	fOkButton->MoveTo (fServerList->Frame ().right - fOkButton->Frame ().Width (),
		fLegend4->Frame ().bottom + 5);
}

NetPrefsServerView::~NetPrefsServerView (void)
{
	BMessenger msgr (fEntryWin);
	if (msgr.IsValid ())
		msgr.SendMessage (B_QUIT_REQUESTED);
}

void
NetPrefsServerView::AttachedToWindow (void)
{
	BView::AttachedToWindow ();
	fAddButton->SetTarget (this);
	fEditButton->SetTarget (this);
	fRemoveButton->SetTarget (this);
	fServerList->SetTarget (this);
	fEditButton->SetEnabled (false);
	fRemoveButton->SetEnabled (false);
	fOkButton->SetTarget (Window ());

	if (fOkButton->Frame ().bottom > Bounds ().Height ())
		Window ()->ResizeTo (Bounds ().Width (), fOkButton->Frame ().bottom + 5);
}

void
NetPrefsServerView::AddServer (const ServerData * data)
{
	BAutolock lock (Looper ());
	if (!lock.IsLocked ())
		return;

	BRow *row (new BRow);
	switch (data->state)
		{
			case SERVER_PRIMARY:
				row->SetField (new BStringField ("*"), 0);
				break;

			case SERVER_SECONDARY:
				row->SetField (new BStringField ("+"), 0);
				break;

			case SERVER_DISABLED:
				row->SetField (new BStringField ("-"), 0);
				break;
		}
	BString server ("");
	server = data->serverName;
	BStringField *serverField (new BStringField (server.String ()));
	row->SetField (serverField, 1);
	server = "";
	server << data->port;
	BStringField *portField (new BStringField (server.String ()));
	row->SetField (portField, 2);
	fServerList->AddRow (row);
}

void
NetPrefsServerView::RemoveServer ()
{
	BAutolock lock (Looper ());

	if (!lock.IsLocked ())
		return;

	BRow *row (fServerList->CurrentSelection ());
	if (row)
	{
		BStringField *field ((BStringField *) row->GetField (1));

		int32 count;
		ssize_t size;
		type_code type;
		fActiveNetwork->GetInfo ("server", &type, &count);

		const ServerData *data;
		for (int32 i = 0; i < count; i++)
		{
			fActiveNetwork->FindData ("server", B_ANY_TYPE, i, reinterpret_cast < const void **>(&data),
				&size);

			if (!strcmp (data->serverName, field->String ()))
			{
				fActiveNetwork->RemoveData ("server", i);
				break;
			}
	}
		fServerList->RemoveRow (row);
		delete row;
	}
}

void
NetPrefsServerView::UpdateNetworkData (const ServerData * newServer)
{
	if (newServer == NULL)
		return;
	type_code type;
	int32 count;
	ssize_t size;
	fActiveNetwork->GetInfo ("server", &type, &count);
	const ServerData *data (NULL);
	for (int32 i = 0; i < count; i++)
	{
		fActiveNetwork->FindData ("server", B_ANY_TYPE, i, reinterpret_cast < const void **>(&data), &size);
		if (!strcmp (newServer->serverName, data->serverName))
		{
			fActiveNetwork->ReplaceData ("server", B_RAW_TYPE, i, newServer, sizeof (ServerData));
			return;
		}
	}
	fActiveNetwork->AddData ("server", B_RAW_TYPE, newServer, sizeof (ServerData));
}

void
NetPrefsServerView::SetNetworkData (BMessage * msg)
{
	// this shouldn't theoretically be able to happen but better safe than sorry
	BLooper *looper (Looper());
	if (looper == NULL)
		return;

	BAutolock lock (Looper ());
	if (!lock.IsLocked ())
		return;
	// clear previous servers (if any)
	while (fServerList->CountRows () > 0)
	{
		BRow *row (fServerList->RowAt (0));
		fServerList->RemoveRow (row);
		delete row;
	}

	BString netString = B_TRANSLATE("Select servers for %1");
	netString.ReplaceFirst("%1", msg->FindString("name"));
	netString += ":";
	type_code type;
	int32 count;
	ssize_t size;
	const ServerData *data;
	msg->GetInfo ("server", &type, &count);
	for (int32 i = 0; i < count; i++)
	{
		msg->FindData ("server", B_ANY_TYPE, i, reinterpret_cast < const void **>(&data), &size);
		AddServer (data);
	}
	fActiveNetwork = msg;
	fSelectTitleString->SetText (netString.String ());
	fSelectTitleString->ResizeToPreferred ();
}

void
NetPrefsServerView::MessageReceived (BMessage * msg)
{
	switch (msg->what)
	{
		case M_SERVER_ITEM_SELECTED:
			{
				BRow *row (fServerList->CurrentSelection ());
				if (row)
				{
					fEditButton->SetEnabled (true);
					fRemoveButton->SetEnabled (true);
				}
				else
				{
					fEditButton->SetEnabled (false);
					fRemoveButton->SetEnabled (false);
				}
			}
			break;

		case M_SERVER_ADD_ITEM:
			{
				BMessenger msgr (fEntryWin);
				if (msgr.IsValid ())
					fEntryWin->Activate ();
				else
				{
					fEntryWin = new ServerEntryWindow (this, new BMessage (M_SERVER_RECV_DATA), NULL, 0);
					fEntryWin->Show ();
				}
			}
			break;

		case M_SERVER_EDIT_ITEM:
			{
				BMessenger msgr (fEntryWin);
				if (msgr.IsValid ())
					fEntryWin->Activate ();
				else
				{
					BRow *row (fServerList->CurrentSelection ());
					if (!row)
						break;
					int32 count (0);
					ssize_t size (0);
					type_code type;
					fActiveNetwork->GetInfo ("server", &type, &count);
					const ServerData *compData;
					for (int32 i = 0; i < count; i++)
					{
						fActiveNetwork->FindData ("server", B_RAW_TYPE, i, reinterpret_cast < const void **>(&compData), &size);
						if (!strcmp (compData->serverName, ((BStringField *) row->GetField (1))->String ()))
							break;
				}
					BMessage *invoke (new BMessage (M_SERVER_RECV_DATA));
					invoke->AddBool ("edit", true);
					fEntryWin = new ServerEntryWindow (this, invoke, compData, size);
					fEntryWin->Show ();
					}
			}
			break;


		case M_SERVER_REMOVE_ITEM:
			{
				RemoveServer ();
				fNetWin.SendMessage (M_SERVER_DATA_CHANGED);
			}
			break;

		case M_SERVER_RECV_DATA:
			{
				const ServerData *data;
				ssize_t size;
				Window ()->DisableUpdates ();
				msg->FindData ("server", B_RAW_TYPE, reinterpret_cast < const void **>(&data), &size);
				if (msg->HasBool ("edit"))
					RemoveServer ();
				UpdateNetworkData (data);
				AddServer (data);
				Window ()->EnableUpdates ();
				fNetWin.SendMessage (M_SERVER_DATA_CHANGED);
			}
			break;

		default:
			BView::MessageReceived (msg);
			break;
	}
}
