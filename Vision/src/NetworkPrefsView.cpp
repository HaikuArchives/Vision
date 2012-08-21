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
 *								 Alan Ellis
 */

#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ListItem.h>
#include <ListView.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <StringView.h>
#include <ScrollView.h>
#include <SupportDefs.h>
#include <TextControl.h>
#include <TextView.h>
#include <TranslationUtils.h>

#include <stdio.h>

#include "NetworkPrefsView.h"
#include "NetworkWindow.h"
#include "Prompt.h"
#include "SpeedButton.h"
#include "Vision.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NetworkPrefView"


class InvokingTextView : public BTextView, public BInvoker
{
	public:
		InvokingTextView(BRect, const char *, BMessage *, BHandler *, uint32, uint32);
		virtual ~InvokingTextView (void);

		virtual void KeyDown(const char *, int32);
};

InvokingTextView::InvokingTextView(BRect frame, const char *name, BMessage *msg, BHandler *target, uint32 resize, uint32 flags)
 : BTextView(frame, name, BRect(0.0, 0.0, frame.Width(), frame.Height()), resize, flags),
	 BInvoker(msg, target)
{
}

InvokingTextView::~InvokingTextView(void)
{
}

void
InvokingTextView::KeyDown(const char *bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes, numBytes);
	Invoke();
}


NetworkPrefsView::NetworkPrefsView (BRect bounds, const char *name)
	: BView (
			bounds,
			name,
			B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
				fNickUpButton (NULL),
				fNickDnButton (NULL),
				fNickPrompt (NULL),
					fNetPrompt (NULL),
					fDupePrompt (NULL),
					fServerPrefs (NULL)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BMenu *menu (new BMenu (B_TRANSLATE("Network")));
	menu->AddItem (new BMenuItem (B_TRANSLATE("Defaults"), new BMessage (M_NETWORK_DEFAULTS)));
	menu->AddSeparatorItem ();
	BString itemText = B_TRANSLATE("Add network" B_UTF8_ELLIPSIS);
	menu->AddItem (new BMenuItem (itemText.String(), new BMessage (M_ADD_NEW_NETWORK)));
	menu->AddItem (fRemoveItem = new BMenuItem (B_TRANSLATE("Remove current network"), new BMessage (M_REMOVE_CURRENT_NETWORK)));
	itemText = B_TRANSLATE("Duplicate current network" B_UTF8_ELLIPSIS);
	menu->AddItem (fDupeItem = new BMenuItem (itemText.String(), new BMessage (M_DUPE_CURRENT_NETWORK)));
	fNetworkMenu = new BMenuField (BRect (0, 0, 100, 30), "NetList", NULL, menu);
	fMainNetBox = new BBox (Bounds ().InsetByCopy (5, 5));
	fMainNetBox->SetLabel (fNetworkMenu);
	AddChild (fMainNetBox);
	BRect boundsRect (Bounds ().InsetByCopy (10, 15));
	boundsRect.right /= 2;
	boundsRect.right -= 8;
	boundsRect.top += 15;
	boundsRect.bottom -= 30;
	fNetDetailsBox = new BBox (boundsRect);
	fNetDetailsBox->SetLabel (B_TRANSLATE("Network Details"));
	fMainNetBox->AddChild (fNetDetailsBox);
	fPersonalBox = new BBox (boundsRect);
	fPersonalBox->SetLabel (B_TRANSLATE("Personal Details"));
	fMainNetBox->AddChild (fPersonalBox);
	fPersonalBox->MoveBy (boundsRect.Width () + 16, 0);
	boundsRect.left += 10;
	boundsRect.right = boundsRect.left + 10;
	boundsRect.top = 10;
	boundsRect.bottom += 20;
	BStringView *stringView1 (new BStringView(boundsRect, NULL,
		B_TRANSLATE("Primary server: ")));
	stringView1->ResizeToPreferred();
	fNetDetailsBox->AddChild(stringView1);
	stringView1->MoveTo(fNetDetailsBox->Frame().left, fNetDetailsBox->Frame().top);
	fConnectServer = new BStringView(boundsRect, NULL, "irc.freenode.net");
	fConnectServer->ResizeToPreferred();
	fNetDetailsBox->AddChild(fConnectServer);
	fConnectServer->MoveTo(stringView1->Frame().left, stringView1->Frame().bottom);
	fAlternates = new BStringView(boundsRect, NULL, B_TRANSLATE("Fallbacks: %1"));
	fAlternates->ResizeToPreferred();
	fNetDetailsBox->AddChild(fAlternates);
	fAlternates->MoveTo(fConnectServer->Frame().left, fConnectServer->Frame().bottom);
	itemText = B_TRANSLATE("Edit servers" B_UTF8_ELLIPSIS);
	fServerButton = new BButton (boundsRect, NULL, itemText.String(),
						new BMessage (M_SERVER_DIALOG));
	fServerButton->ResizeToPreferred ();
	fServerButton->MoveTo (fNetDetailsBox->Frame().right - (fServerButton->Frame().Width() + 20),
			fAlternates->Frame().bottom + 10.0);
	fNetDetailsBox->AddChild (fServerButton);
	itemText = B_TRANSLATE("Autoexec");
	itemText += ":";
	BStringView *stringView4 (new BStringView (boundsRect, NULL, itemText.String()));
	stringView4->ResizeToPreferred ();
	stringView4->MoveTo (fNetDetailsBox->Frame().left, fServerButton->Frame ().bottom);
	fNetDetailsBox->AddChild (stringView4);
	boundsRect = fNetDetailsBox->Frame ();
	boundsRect.right -= (20 + B_V_SCROLL_BAR_WIDTH);;
	boundsRect.top = stringView4->Frame ().bottom + 5;
	boundsRect.bottom -= 65;
	fTextView = new InvokingTextView (boundsRect, NULL, new BMessage(M_NETPREFS_TEXT_INVOKE), this, B_FOLLOW_NONE, B_WILL_DRAW);
	BScrollView *scrollView (new BScrollView (NULL, fTextView, B_FOLLOW_LEFT | B_FOLLOW_TOP,
							0, false, true));
	fTextView->MakeEditable (true);
	fTextView->SetStylable (false);
	fNetDetailsBox->AddChild (scrollView);
	fLagCheckBox = new BCheckBox (boundsRect, NULL, B_TRANSLATE("Enable lag checking"),
									new BMessage (M_NET_CHECK_LAG));
	fLagCheckBox->ResizeToPreferred ();
	fLagCheckBox->MoveTo (scrollView->Frame().left,
		scrollView->Frame ().bottom + 5);
	fNetDetailsBox->AddChild (fLagCheckBox);
	fStartupBox = new BCheckBox (boundsRect, NULL, B_TRANSLATE("Connect to this network when Vision starts up"),
						new BMessage (M_CONNECT_ON_STARTUP));
	fStartupBox->ResizeToPreferred ();
	fStartupBox->MoveTo (fNetDetailsBox->Frame ().left, fMainNetBox->Frame ().bottom - (fStartupBox->Bounds ().Height () + 12));
	fMainNetBox->AddChild (fStartupBox);
	fNickDefaultsBox = new BCheckBox (boundsRect, NULL, B_TRANSLATE("Use Defaults"),
					 new BMessage (M_USE_NICK_DEFAULTS));
	fNickDefaultsBox->ResizeToPreferred ();
	fNickDefaultsBox->MoveTo (fNetDetailsBox->Frame ().left, fNetDetailsBox->Frame ().top);
	fPersonalBox->AddChild (fNickDefaultsBox);
	itemText = B_TRANSLATE("Preferred Nicknames:");
	BStringView *stringView5 (new BStringView (boundsRect, NULL, B_TRANSLATE("Preferred Nicknames:")));
	stringView5->ResizeToPreferred ();
	stringView5->MoveTo (fNetDetailsBox->Frame ().left, fNickDefaultsBox->Frame ().bottom + 5);
	fPersonalBox->AddChild (stringView5);
	fListView = new BListView (scrollView->Frame (), NULL);
	fListView->SetSelectionMessage (new BMessage (M_NICK_SELECTED));
	fListView->ResizeBy (-B_V_SCROLL_BAR_WIDTH, -10.0);
	fListView->MoveTo (fListView->Frame ().left, stringView5->Frame ().bottom + 5);
	BScrollView *listScroll (new BScrollView (NULL, fListView, B_FOLLOW_LEFT | B_FOLLOW_TOP,
							0, false, true));
	fPersonalBox->AddChild (listScroll);
	itemText = B_TRANSLATE("Add" B_UTF8_ELLIPSIS);
	fNickAddButton = new BButton (boundsRect, NULL, itemText.String(), new BMessage (M_ADD_NICK));
	fNickAddButton->ResizeToPreferred ();
	fNickRemoveButton = new BButton (boundsRect, NULL, B_TRANSLATE("Remove"), new BMessage (M_REMOVE_NICK));
	fNickRemoveButton->ResizeToPreferred ();
	fNickRemoveButton->MoveTo (listScroll->Frame ().right - fNickRemoveButton->Frame ().Width (),
					listScroll->Frame ().bottom + 5);
	fPersonalBox->AddChild (fNickRemoveButton);
	fNickAddButton->MoveTo (fNickRemoveButton->Frame ().left - (fNickAddButton->Frame ().Width () + 5),
			 fNickRemoveButton->Frame ().top);
	fPersonalBox->AddChild (fNickAddButton);
	BBitmap *bmp (BTranslationUtils::GetBitmap ('bits', "UpArrow"));
	if (bmp)
	{
		boundsRect = bmp->Bounds ().InsetByCopy (-2, -2);
		fNickUpButton = new TSpeedButton (boundsRect, NULL, NULL, new BMessage (M_NICK_UP), bmp);
		fNickUpButton->MoveTo (listScroll->Frame ().left, fNickAddButton->Frame ().top);
		fPersonalBox->AddChild (fNickUpButton);
		delete bmp;
	}
	bmp = BTranslationUtils::GetBitmap ('bits', "DownArrow");
	if (bmp)
	{
		boundsRect = bmp->Bounds ().InsetByCopy (-2, -2);
		fNickDnButton = new TSpeedButton (boundsRect, NULL, NULL, new BMessage (M_NICK_DOWN), bmp);
		fNickDnButton->MoveTo (fNickUpButton->Frame ().left, fNickUpButton->Frame ().bottom);
		fPersonalBox->AddChild (fNickDnButton);
		delete bmp;
	}
	itemText = B_TRANSLATE("Ident");
	itemText += ": ";
	fIdent = new BTextControl (listScroll->Frame (), NULL, itemText.String(), NULL, NULL);
	itemText = B_TRANSLATE("Real name");
	itemText += ": ";
	fRealName = new BTextControl (listScroll->Frame (), NULL, itemText.String(), NULL, NULL);
	fRealName->ResizeTo (listScroll->Frame ().Width (), fRealName->Frame ().Height ());
	fRealName->MoveTo (listScroll->Frame ().left, fNickAddButton->Frame ().bottom + 5);
	fRealName->SetDivider (fRealName->StringWidth (fRealName->Label ()) + 5);
	fPersonalBox->AddChild (fRealName);
	fIdent->ResizeTo (fRealName->Frame ().Width (), fRealName->Frame ().Height ());
	fIdent->MoveTo (listScroll->Frame ().left, fRealName->Frame ().bottom + 5);
	fIdent->SetDivider (fRealName->Divider ());
	fPersonalBox->AddChild (fIdent);
}

NetworkPrefsView::~NetworkPrefsView (void)
{
	BMessenger(fNickPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fNetPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fDupePrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fServerPrefs).SendMessage(B_QUIT_REQUESTED);
}

void
NetworkPrefsView::AttachedToWindow (void)
{
	fNetworkMenu->Menu ()->SetTargetForItems (this);
	fServerButton->SetTarget (this);
	fLagCheckBox->SetTarget (this);
	fStartupBox->SetTarget (this);
	fNickDefaultsBox->SetTarget (this);
	fNickAddButton->SetTarget (this);
	fNickRemoveButton->SetTarget (this);
	fRealName->SetTarget (this);
	fListView->SetTarget (this);
	dynamic_cast<BInvoker *>(fTextView)->SetTarget(this);
	// for some unknown reason the bmp doesn't seem to get retrieved right on
	// some people's systems...if this is the case the up and down buttons will
	// not get created, hence this check
	if (fNickUpButton)
	{
		fNickUpButton->SetTarget (this);
		fNickDnButton->SetTarget (this);
		fNickUpButton->SetEnabled (false);
		fNickDnButton->SetEnabled (false);
	}
	fNickRemoveButton->SetEnabled (false);
	fLagCheckBox->SetEnabled (false);
	fIdent->SetTarget (this);
	dynamic_cast < BInvoker * >(fNetworkMenu->Menu ()->ItemAt (0))->Invoke ();
	BuildNetworkList ();
}

void
NetworkPrefsView::DetachedFromWindow (void)
{
	// save changes to active network
	// get autoexec commands

	SaveCurrentNetwork ();
}

void
NetworkPrefsView::BuildNetworkList (void)
{
	if (fNetworkMenu == NULL)
		return;

	BMessage msg;

	BMenu *menu (fNetworkMenu->Menu ());

	for (int32 i = 0; (msg = vision_app->GetNetwork (i)), !msg.HasBool ("error"); i++)
	{
		BMenuItem *item (new BMenuItem (msg.FindString ("name"), new BMessage (M_CHOOSE_NETWORK)));
		menu->AddItem (item, 0);
		item->SetTarget (this);
	}
}

void
NetworkPrefsView::UpdateNetworkData (BMessage & msg)
{
	// enable network controls
	fStartupBox->SetEnabled (true);
	fServerButton->SetEnabled (true);
	fTextView->MakeEditable (true);
	fLagCheckBox->SetEnabled (true);
	SetPrimaryServer("<N/A>");
	SetAlternateCount(0);

	bool startup (false),
			 lagcheck (true);
	if (msg.FindBool ("connectOnStartup", &startup) == B_OK)
		fStartupBox->SetValue ((startup) ? B_CONTROL_ON : B_CONTROL_OFF);
	else
		fStartupBox->SetValue (B_CONTROL_OFF);
	if (msg.FindBool ("lagCheck", &lagcheck) == B_OK)
		fLagCheckBox->SetValue ((lagcheck) ? B_CONTROL_ON : B_CONTROL_OFF);
	else
		fLagCheckBox->SetValue (B_CONTROL_OFF);

	const char *autoexec (NULL);
	if ((autoexec = msg.FindString ("autoexec")) != NULL)
		fTextView->SetText (autoexec);
	else
		fTextView->SetText ("");

	uint32 altCount(0);
	ssize_t size;
	const ServerData *data (NULL);
	for (int32 i = 0; msg.FindData("server", B_ANY_TYPE, i,
		reinterpret_cast<const void **>(&data), &size) == B_OK; i++)
	{
		if (data->state == 0)
			SetPrimaryServer(data->serverName);
		else if(data->state == 1)
			++altCount;
	}
	SetAlternateCount(altCount);
}

void
NetworkPrefsView::UpdatePersonalData (BMessage & msg)
{
	BString curIdent ("");
	BString curRname ("");
	BString curNick ("");
	int32 count (fListView->CountItems ()),
				i (0);
	for (i = 0; i < count; i++)
		delete (fListView->RemoveItem ((int32)0));

	if ((msg.HasBool ("useDefaults") && msg.FindBool ("useDefaults")))
	{
		BMessage defaults (vision_app->GetNetwork ("defaults"));
		defaults.FindString ("ident", &curIdent);
		defaults.FindString ("realname", &curRname);
		for (i = 0; defaults.FindString ("nick", i, &curNick) == B_OK; i++)
			fListView->AddItem (new BStringItem (curNick.String()));
		fIdent->SetEnabled (false);
		fRealName->SetEnabled (false);
		fNickAddButton->SetEnabled (false);
		fNickRemoveButton->SetEnabled (false);
		fNickDefaultsBox->SetValue (B_CONTROL_ON);
	}
	else
	{
		msg.FindString ("ident", &curIdent);
		msg.FindString ("realname", &curRname);
		for (i = 0; msg.FindString ("nick", i, &curNick) == B_OK; i++)
			fListView->AddItem (new BStringItem (curNick.String()));
		fIdent->SetEnabled (true);
		fRealName->SetEnabled (true);
		fNickAddButton->SetEnabled (true);
		fNickDefaultsBox->SetValue (B_CONTROL_OFF);
	}

	if (fActiveNetwork.what != VIS_NETWORK_DEFAULTS)
		fNickDefaultsBox->SetEnabled (true);

	if (curIdent.Length())
		fIdent->SetText (curIdent.String());
	else
		fIdent->SetText ("");
	if (curRname.Length())
		fRealName->SetText (curRname.String());
	else
		fRealName->SetText ("");

	if (fListView->CountItems () == 0)
		fNickRemoveButton->SetEnabled (false);
}

void
NetworkPrefsView::SetupDefaults (BMessage & msg)
{
	// disable appropriate controls
	fTextView->MakeEditable (false);
	fTextView->SetText ("");
	fServerButton->SetEnabled (false);
	SetPrimaryServer("<N/A>");
	SetAlternateCount(0);
	fNickDefaultsBox->SetEnabled (false);
	fStartupBox->SetEnabled (false);
	fLagCheckBox->SetEnabled (false);
	// update personal data
	UpdatePersonalData (msg);
}

void
NetworkPrefsView::SaveCurrentNetwork (void)
{
	if (fActiveNetwork.FindString ("name") == NULL)
		return;

	// check real name and fIdent, update if needed
	if (fNickDefaultsBox->Value () == 0)
	{
		const char *curRname (fRealName->Text ());
		if (curRname != NULL)
	{
		if (!fActiveNetwork.HasString ("realname"))
			fActiveNetwork.AddString ("realname", curRname);
		else
			fActiveNetwork.ReplaceString ("realname", curRname);
	}

		const char *curIdent (fIdent->Text ());
		if (curIdent != NULL)
	{
		if (!fActiveNetwork.HasString ("ident"))
			fActiveNetwork.AddString ("ident", curIdent);
		else
			fActiveNetwork.ReplaceString ("ident", curIdent);
	}
	}

	// Not usually the best thing, but lets just do this until we
	// split the functionality into a function.
	BMessage m(M_NETPREFS_TEXT_INVOKE);
	MessageReceived(&m);

	const char *name (fActiveNetwork.FindString ("name"));

	if (!strcmp (fActiveNetwork.FindString ("name"), "defaults"))
	{
		vision_app->SetNetwork ("defaults", &fActiveNetwork);
		return;
	}

	vision_app->SetNetwork (name, &fActiveNetwork);
}

void
NetworkPrefsView::SetPrimaryServer(const char *serverName)
{
	fConnectServer->SetText(serverName);
	fConnectServer->ResizeToPreferred();
}

void
NetworkPrefsView::SetAlternateCount(uint32 altCount)
{
	BString text = B_TRANSLATE("Fallbacks: %1");
	BString value;
	value << altCount;
	text.ReplaceFirst("%1", value);
	fAlternates->SetText(text.String());
	fAlternates->ResizeToPreferred();
}

void
NetworkPrefsView::MessageReceived (BMessage * msg)
{
	switch (msg->what)
	{
		case M_NETWORK_DEFAULTS:
			if (fActiveNetwork.HasString ("name"))
				vision_app->SetNetwork (fActiveNetwork.FindString ("name"), &fActiveNetwork);
			fActiveNetwork = vision_app->GetNetwork ("defaults");
			fNetworkMenu->MenuItem ()->SetLabel ("Defaults");
			SetupDefaults (fActiveNetwork);
			fDupeItem->SetEnabled (false);
			fRemoveItem->SetEnabled (false);
			break;

		case M_CHOOSE_NETWORK:
			{
				BMenuItem *item (NULL);
				msg->FindPointer ("source", reinterpret_cast < void **>(&item));
				SaveCurrentNetwork ();
				fActiveNetwork = vision_app->GetNetwork (item->Label ());
				fNetworkMenu->MenuItem ()->SetLabel (item->Label ());
				UpdatePersonalData (fActiveNetwork);
				UpdateNetworkData (fActiveNetwork);
				if (BMessenger (fServerPrefs).IsValid ())
					fServerPrefs->SetNetworkData (&fActiveNetwork);
				fDupeItem->SetEnabled (true);
				fRemoveItem->SetEnabled (true);
			}
			break;

		case M_ADD_NEW_NETWORK:
			{
				if (msg->HasString ("text"))
				{
					fNetPrompt = NULL;
					BString network (msg->FindString ("text"));
					network.RemoveAll (" ");
					BMenu *menu (fNetworkMenu->Menu ());
					for (int32 i = 0; i < menu->CountItems (); i++)
				{
						BMenuItem *item (menu->ItemAt (i));
						if (item && network == item->Label ())
						{
							dynamic_cast < BInvoker * >(item)->Invoke ();
							return;
						}
					}
					BMessage newNet (VIS_NETWORK_DATA);
					newNet.AddString ("name", network.String ());
					vision_app->SetNetwork (network.String (), &newNet);
					BMenuItem *item (new BMenuItem (network.String (), new BMessage (M_CHOOSE_NETWORK)));
					menu->AddItem (item, 0);
					item->SetTarget (this);
					dynamic_cast < BInvoker * >(item)->Invoke ();
			}
				else
				{
					BString promptText = B_TRANSLATE("Network Name");
					promptText += ": ";
					fNetPrompt = new PromptWindow (BPoint (Window ()->Frame ().left + Window ()->Frame ().Width () / 2, Window ()->Frame ().top + Window ()->Frame ().Height () / 2),
						promptText.String(), B_TRANSLATE("Add Network"), NULL, this, new BMessage (M_ADD_NEW_NETWORK), NULL, false);
					fNetPrompt->Show ();
				}
			}
			break;

		case M_REMOVE_CURRENT_NETWORK:
			{
				const char *name (fActiveNetwork.FindString ("name"));
				vision_app->RemoveNetwork (name);
				BMenu *menu (fNetworkMenu->Menu ());
				for (int32 i = 0; i < menu->CountItems (); i++)
				{
					BMenuItem *item (menu->ItemAt (i));
					if (!strcmp (item->Label (), name))
					{
						delete menu->RemoveItem (i);
						fActiveNetwork.MakeEmpty ();
						dynamic_cast < BInvoker * >(menu->ItemAt (0))->Invoke ();
						break;
					}
				}
			}
			break;

		case M_DUPE_CURRENT_NETWORK:
			{
				if (msg->HasString ("text"))
				{
					fDupePrompt = NULL;
					BString network (msg->FindString ("text"));
					network.RemoveAll (" ");
					BMenu *menu (fNetworkMenu->Menu ());
					for (int32 i = 0; i < menu->CountItems (); i++)
					{
						BMenuItem *item (menu->ItemAt (i));
						if (item && network == item->Label ())
						{
							dynamic_cast < BInvoker * >(item)->Invoke ();
							return;
						}
					}
					BMessage newNet = fActiveNetwork;
					newNet.ReplaceString ("name", network.String ());
					vision_app->SetNetwork (network.String (), &newNet);
					BMenuItem *item (new BMenuItem (network.String (), new BMessage (M_CHOOSE_NETWORK)));
					menu->AddItem (item, 0);
					item->SetTarget (this);
					dynamic_cast < BInvoker * >(item)->Invoke ();
				}
				else
				{
					BString promptText = B_TRANSLATE("Network Name");
					promptText += ": ";
					fDupePrompt = new PromptWindow (BPoint (Window ()->Frame ().left + Window ()->Frame ().Width () / 2, Window ()->Frame ().top + Window ()->Frame ().Height () / 2),
						promptText.String(), B_TRANSLATE("Duplicate Current Network"), NULL, this, new BMessage (M_DUPE_CURRENT_NETWORK), NULL, false);
					fDupePrompt->Show ();
				}
			}
			break;

		case M_SERVER_DATA_CHANGED:
			{
				UpdateNetworkData (fActiveNetwork);
			}
			break;

		case M_SERVER_DIALOG:
			{
				BMessenger msgr (fServerPrefs);
				if (msgr.IsValid ())
					fServerPrefs->Activate ();
				else
				{
					fServerPrefs = new NetPrefServerWindow (this);
					fServerPrefs->SetNetworkData (&fActiveNetwork);
					fServerPrefs->Show ();
				}
			}
			break;

		case M_NET_CHECK_LAG:
			{
				bool value = msg->FindInt32 ("be:value");
				if (fActiveNetwork.HasBool ("lagCheck"))
					fActiveNetwork.ReplaceBool ("lagCheck", value);
				else
					fActiveNetwork.AddBool ("lagCheck", value);
			}
			break;

		case M_CONNECT_ON_STARTUP:
			{
				bool value = msg->FindInt32 ("be:value");
				if (fActiveNetwork.HasBool ("connectOnStartup"))
					fActiveNetwork.ReplaceBool ("connectOnStartup", value);
				else
					fActiveNetwork.AddBool ("connectOnStartup", value);
			}
			break;

		case M_USE_NICK_DEFAULTS:
			{
				bool value = msg->FindInt32 ("be:value");
				if (fActiveNetwork.HasBool ("useDefaults"))
					fActiveNetwork.ReplaceBool ("useDefaults", value);
				else
					fActiveNetwork.AddBool ("useDefaults", value);
				UpdatePersonalData (fActiveNetwork);
			}
			break;

		case M_NETPREFS_TEXT_INVOKE:
			{
				if (fActiveNetwork.HasString("autoexec"))
					fActiveNetwork.ReplaceString("autoexec", fTextView->Text());
				else
					fActiveNetwork.AddString("autoexec", fTextView->Text());
			}
			break;

		case M_ADD_NICK:
			if (msg->HasString ("text"))
			{
				fNickPrompt = NULL;
				BString nick (msg->FindString ("text"));
				nick.RemoveAll (" ");
				for (int32 i = 0; i < fListView->CountItems (); i++)
				{
					BStringItem *item ((BStringItem *) fListView->ItemAt (i));
					if (item && nick == item->Text ())
						return;
				}
			fActiveNetwork.AddString ("nick", nick.String ());
			fListView->AddItem (new BStringItem (nick.String ()));
			}
			else
			{
				BString promptString = B_TRANSLATE("Nickname");
				promptString += ": ";
				fNickPrompt = new PromptWindow (BPoint (Window ()->Frame ().left + Window ()->Frame ().Width () / 2, Window ()->Frame ().top + Window ()->Frame ().Height () / 2),
					promptString.String(), B_TRANSLATE("Add Nickname"), NULL, this, new BMessage (M_ADD_NICK), NULL, false);
				fNickPrompt->Show ();
			}
			break;

		case M_REMOVE_NICK:
			{
				int32 current (fListView->CurrentSelection ());
				if (current >= 0)
			{
					delete fListView->RemoveItem (current);
					fActiveNetwork.RemoveData ("nick", current);
				}
			}
			break;

		case M_NICK_UP:
			{
				int32 current (fListView->CurrentSelection ());
				BString nick1, nick2;
				nick1 = fActiveNetwork.FindString ("nick", current);
				nick2 = fActiveNetwork.FindString ("nick", current - 1);
				fListView->SwapItems (current, current - 1);
				fActiveNetwork.ReplaceString ("nick", current - 1, nick1.String ());
				fActiveNetwork.ReplaceString ("nick", current, nick2.String ());
				current = fListView->CurrentSelection ();
				Window ()->DisableUpdates ();
				fListView->DeselectAll ();
				fListView->Select (current);
				Window ()->EnableUpdates ();
			}
			break;

		case M_NICK_DOWN:
			{
				int32 current (fListView->CurrentSelection ());
				BString nick1, nick2;
				nick1 = fActiveNetwork.FindString ("nick", current);
				nick2 = fActiveNetwork.FindString ("nick", current + 1);
				fListView->SwapItems (current, current + 1);
				fActiveNetwork.ReplaceString ("nick", current + 1, nick1.String ());
				fActiveNetwork.ReplaceString ("nick", current, nick2.String ());
				current = fListView->CurrentSelection ();
				Window ()->DisableUpdates ();
				fListView->DeselectAll ();
				fListView->Select (current);
				Window ()->EnableUpdates ();
			}
			break;

		case M_NICK_SELECTED:
			{
				int32 index (msg->FindInt32 ("index"));
				if (index >= 0 && !fNickDefaultsBox->Value ())
				{
					fNickUpButton->SetEnabled (index > 0);
					fNickDnButton->SetEnabled (index != (fListView->CountItems () - 1));
					fNickRemoveButton->SetEnabled (true);
				}
				else
				{
					fNickUpButton->SetEnabled (false);
					fNickDnButton->SetEnabled (false);
					fNickRemoveButton->SetEnabled (false);
				}
			}
			break;

		default:
			BView::MessageReceived (msg);
			break;
	}
}
