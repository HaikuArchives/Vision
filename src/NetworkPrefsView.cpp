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
 *                 Alan Ellis
 */

#include <Bitmap.h>
#include <Box.h>
#include <Button.h>
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
#include "Vision.h"

class InvokingTextView : public BTextView, public BInvoker
{
public:
	InvokingTextView(BRect, const char*, BMessage*, BHandler*, uint32, uint32);
	virtual ~InvokingTextView(void);

	virtual void KeyDown(const char*, int32);
};

InvokingTextView::InvokingTextView(BRect frame, const char* name, BMessage* msg, BHandler* target,
								   uint32 resize, uint32 flags)
	: BTextView(frame, name, BRect(0.0, 0.0, frame.Width(), frame.Height()), resize, flags),
	  BInvoker(msg, target)
{
}

InvokingTextView::~InvokingTextView(void)
{
}

void InvokingTextView::KeyDown(const char* bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes, numBytes);
	Invoke();
}

NetworkPrefsView::NetworkPrefsView(BRect bounds, const char* name)
	: BView(bounds, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS),
	  fNickPrompt(NULL),
	  fNetPrompt(NULL),
	  fDupePrompt(NULL),
	  fServerPrefs(NULL)
{
	AdoptSystemColors();

	BMenu* menu(new BMenu(S_NETPREFS_NETMENU));
	menu->AddItem(new BMenuItem(S_NETPREFS_DEFAULTS_ITEM, new BMessage(M_NETWORK_DEFAULTS)));
	menu->AddSeparatorItem();
	menu->AddItem(
		new BMenuItem(S_NETPREFS_ADD_NEW B_UTF8_ELLIPSIS, new BMessage(M_ADD_NEW_NETWORK)));
	menu->AddItem(fRemoveItem =
					  new BMenuItem(S_NETPREFS_REMOVE, new BMessage(M_REMOVE_CURRENT_NETWORK)));
	menu->AddItem(fDupeItem = new BMenuItem(S_NETPREFS_DUPE B_UTF8_ELLIPSIS,
											new BMessage(M_DUPE_CURRENT_NETWORK)));
	fNetworkMenu = new BMenuField(BRect(0, 0, 100, 30), "NetList", NULL, menu);
	fMainNetBox = new BBox(Bounds().InsetByCopy(kItemSpacing/2, kItemSpacing/2), "", B_FOLLOW_ALL);
	fMainNetBox->SetLabel(fNetworkMenu);
	AddChild(fMainNetBox);

	BRect boundsRect(fMainNetBox->InnerFrame());
	boundsRect.right /= 2;
	boundsRect.right -= kItemSpacing/2;
	boundsRect.top += kItemSpacing;

	fNetDetailsBox = new BBox(boundsRect);
	fNetDetailsBox->SetLabel(S_NETPREFS_NET_BOX);
	fMainNetBox->AddChild(fNetDetailsBox);
	fPersonalBox = new BBox(boundsRect);
	fPersonalBox->SetLabel(S_NETPREFS_PERSONAL_BOX);
	fMainNetBox->AddChild(fPersonalBox);
	fPersonalBox->MoveBy(boundsRect.Width() + kItemSpacing, 0);

	boundsRect.left += kItemSpacing;
	boundsRect.right = boundsRect.left + kItemSpacing;
	boundsRect.top = kItemSpacing;
	boundsRect.bottom += kItemSpacing * 2;

	BStringView* stringView1(new BStringView(boundsRect, NULL, S_NETPREFS_CONN1));
	stringView1->ResizeToPreferred();
	stringView1->MoveTo(fNetDetailsBox->InnerFrame().left + kItemSpacing,
		fNetDetailsBox->InnerFrame().top + kItemSpacing);
	fNetDetailsBox->AddChild(stringView1);

	fConnectServer = new BStringView(boundsRect, NULL, "irc.sorcery.net,");
	fConnectServer->ResizeToPreferred();
	fConnectServer->MoveTo(stringView1->Frame().left, stringView1->Frame().bottom);

	fAlternates = new BStringView(boundsRect, NULL, S_NETPREFS_CONN2);
	fAlternates->ResizeToPreferred();
	fAlternates->MoveTo(fConnectServer->Frame().left, fConnectServer->Frame().bottom);

	fNetDetailsBox->AddChild(fConnectServer);
	fNetDetailsBox->AddChild(fAlternates);

	fServerButton = new BButton(boundsRect, NULL, S_NETPREFS_CHANGE_SERVER B_UTF8_ELLIPSIS,
								new BMessage(M_SERVER_DIALOG));
	fServerButton->ResizeToPreferred();
	fServerButton->MoveTo(kItemSpacing, fAlternates->Frame().bottom + 5);
	fNetDetailsBox->AddChild(fServerButton);

	BStringView* stringView4(new BStringView(boundsRect, NULL, S_NETPREFS_AUTOEXEC));
	stringView4->ResizeToPreferred();
	stringView4->MoveTo(fAlternates->Frame().left, fServerButton->Frame().bottom + 5);
	fNetDetailsBox->AddChild(stringView4);

	boundsRect = fNetDetailsBox->Frame();
	boundsRect.left += kItemSpacing;
	boundsRect.right -= (20 + B_V_SCROLL_BAR_WIDTH);
	boundsRect.top = stringView4->Frame().bottom + 5;
	boundsRect.bottom -= 65;

	fTextView = new InvokingTextView(boundsRect, NULL, new BMessage(M_NETPREFS_TEXT_INVOKE), this,
									 B_FOLLOW_ALL, B_WILL_DRAW);
	fExecScroller =
		new BScrollView(NULL, fTextView, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP, 0, false, true);
	fTextView->MakeEditable(true);
	fTextView->SetStylable(false);
	fNetDetailsBox->AddChild(fExecScroller);

	fLagCheckBox = new BCheckBox(boundsRect, NULL, S_NETPREFS_LAG_CHECK,
		new BMessage(M_NET_CHECK_LAG), B_FOLLOW_BOTTOM);
	fLagCheckBox->ResizeToPreferred();
	fLagCheckBox->MoveTo(kItemSpacing, fNetDetailsBox->InnerFrame().bottom
		- fLagCheckBox->Bounds().Height());
	fNetDetailsBox->AddChild(fLagCheckBox);

	fStartupBox = new BCheckBox(boundsRect, NULL, S_NETPREFS_STARTUP_CONN,
			new BMessage(M_CONNECT_ON_STARTUP), B_FOLLOW_BOTTOM);
	fStartupBox->ResizeToPreferred();
	fStartupBox->MoveTo(fNetDetailsBox->Frame().left + kItemSpacing,
			fMainNetBox->Bounds().bottom - (fStartupBox->Bounds().Height()
			+ kItemSpacing));
	fMainNetBox->AddChild(fStartupBox);

	// PERSONAL BOX
	boundsRect = fPersonalBox->InnerFrame();
	boundsRect.InsetBy(kItemSpacing, kItemSpacing);

	fNickDefaultsBox =
		new BCheckBox(boundsRect, NULL, S_NETPREFS_USE_DEFAULTS, new BMessage(M_USE_NICK_DEFAULTS));
	fNickDefaultsBox->ResizeToPreferred();
	fPersonalBox->AddChild(fNickDefaultsBox);

	// Preferred nicks:
	BStringView* stringView5(new BStringView(boundsRect, NULL, S_NETPREFS_PREFNICK));
	stringView5->ResizeToPreferred();
	stringView5->MoveTo(fAlternates->Frame().left, fAlternates->Frame().top);
	fPersonalBox->AddChild(stringView5);

	fListView = new BListView(fExecScroller->Frame(), NULL, B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL);
	fListView->SetSelectionMessage(new BMessage(M_NICK_SELECTED));
	fListView->ResizeBy(-B_V_SCROLL_BAR_WIDTH, -(kItemSpacing * 3.5));
	fListView->MoveTo(fListView->Frame().left, stringView5->Frame().bottom + 5);
	fNickScroller = new BScrollView(NULL, fListView, B_FOLLOW_ALL, 0, false, true);
	fPersonalBox->AddChild(fNickScroller);

	// Add/Remove nick buttons
	boundsRect = fPersonalBox->InnerFrame();
	boundsRect.InsetBy(kItemSpacing, kItemSpacing);
	boundsRect.top = boundsRect.bottom - fServerButton->Bounds().Height();

	fNickAddButton = new BButton(boundsRect, NULL, S_NETPREFS_ADD_BUTTON B_UTF8_ELLIPSIS,
								 new BMessage(M_ADD_NICK), B_FOLLOW_BOTTOM);
	fNickAddButton->ResizeToPreferred();

	boundsRect = fNickAddButton->Frame();
	boundsRect.OffsetBy(boundsRect.Width() + kItemSpacing, 0);
	
	fNickRemoveButton = new BButton(boundsRect, NULL, S_NETPREFS_REMOVE_BUTTON,
		new BMessage(M_REMOVE_NICK), B_FOLLOW_BOTTOM);
	fNickRemoveButton->ResizeToPreferred();

	fPersonalBox->AddChild(fNickRemoveButton);
	fPersonalBox->AddChild(fNickAddButton);

	// Ident and Realname text controls
	boundsRect.OffsetBy(0, -boundsRect.Height());
	boundsRect.left = fNickScroller->Frame().left;
	boundsRect.right = fNickScroller->Frame().right;

	fRealName = new BTextControl(boundsRect, NULL,
		S_NETPREFS_REALNAME, NULL, NULL, B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT);
	fRealName->ResizeToPreferred();
	fRealName->ResizeTo(boundsRect.Width(), fRealName->Bounds().Height());
	fRealName->SetDivider(fRealName->StringWidth(fRealName->Label()) + 5);
	boundsRect = fRealName->Frame();

	boundsRect.OffsetBy(0, -(boundsRect.Height() + 5));
	fIdent = new BTextControl(boundsRect, NULL, S_NETPREFS_IDENT, NULL, NULL,
		B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT);
	fIdent->SetDivider(fRealName->Divider());

	fPersonalBox->AddChild(fIdent);
	fPersonalBox->AddChild(fRealName);
}

NetworkPrefsView::~NetworkPrefsView(void)
{
	BMessenger(fNickPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fNetPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fDupePrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fServerPrefs).SendMessage(B_QUIT_REQUESTED);
}

void NetworkPrefsView::AttachedToWindow(void)
{
	fNetworkMenu->Menu()->SetTargetForItems(this);
	fServerButton->SetTarget(this);
	fLagCheckBox->SetTarget(this);
	fStartupBox->SetTarget(this);
	fNickDefaultsBox->SetTarget(this);
	fNickAddButton->SetTarget(this);
	fNickRemoveButton->SetTarget(this);
	fRealName->SetTarget(this);
	fListView->SetTarget(this);
	dynamic_cast<BInvoker*>(fTextView)->SetTarget(this);

	fNickRemoveButton->SetEnabled(false);
	fLagCheckBox->SetEnabled(false);
	fIdent->SetTarget(this);
	dynamic_cast<BInvoker*>(fNetworkMenu->Menu()->ItemAt(0))->Invoke();
	BuildNetworkList();

	FrameResized(0, 0);

	// Correct size of fNickScroller
	BRect boundsRect = fNickScroller->Frame();
	boundsRect.bottom = fIdent->Frame().top - kItemSpacing;
	fNickScroller->ResizeTo(boundsRect.Width(), boundsRect.Height());

	boundsRect = fNickScroller->Bounds();
	boundsRect.right -= B_V_SCROLL_BAR_WIDTH;
	fListView->ResizeTo(boundsRect.Width(), boundsRect.Height());
}

void NetworkPrefsView::DetachedFromWindow(void)
{
	// save changes to active network
	// get autoexec commands

	SaveCurrentNetwork();
}

void NetworkPrefsView::BuildNetworkList(void)
{
	if (fNetworkMenu == NULL) return;

	BMessage msg;

	BMenu* menu(fNetworkMenu->Menu());

	for (int32 i = 0; (msg = vision_app->GetNetwork(i)), !msg.HasBool("error"); i++) {
		BMenuItem* item(new BMenuItem(msg.FindString("name"), new BMessage(M_CHOOSE_NETWORK)));
		menu->AddItem(item, 0);
		item->SetTarget(this);
	}
}

void NetworkPrefsView::SetConnectServer(const char* serverName)
{
	fConnectServer->SetText(serverName);
	fConnectServer->ResizeToPreferred();
}

void NetworkPrefsView::SetAlternateCount(uint32 altCount)
{
	if (altCount > 0) {
		BString text(S_NETPREFS_FALLBACK1);
		text << altCount;
		text += S_NETPREFS_FALLBACK2;
		text += (altCount > 1) ? S_NETPREFS_FALLBACK2_PLURAL : "";
		text += ".";
		fAlternates->SetText(text.String());
		fAlternates->ResizeToPreferred();
	} else
		fAlternates->SetText("");
}

void NetworkPrefsView::UpdateNetworkData(BMessage& msg)
{
	// enable network controls
	fStartupBox->SetEnabled(true);
	fServerButton->SetEnabled(true);
	fTextView->MakeEditable(true);
	fLagCheckBox->SetEnabled(true);
	SetConnectServer("<N/A>");
	SetAlternateCount(0);

	bool startup(false), lagcheck(true);
	if (msg.FindBool("connectOnStartup", &startup) == B_OK)
		fStartupBox->SetValue((startup) ? B_CONTROL_ON : B_CONTROL_OFF);
	else
		fStartupBox->SetValue(B_CONTROL_OFF);
	if (msg.FindBool("lagCheck", &lagcheck) == B_OK)
		fLagCheckBox->SetValue((lagcheck) ? B_CONTROL_ON : B_CONTROL_OFF);
	else
		fLagCheckBox->SetValue(B_CONTROL_OFF);

	const char* autoexec(NULL);
	if ((autoexec = msg.FindString("autoexec")) != NULL)
		fTextView->SetText(autoexec);
	else
		fTextView->SetText("");
	uint32 altCount(0);
	ssize_t size;
	const ServerData* data(NULL);
	for (int32 i = 0; msg.FindData("server", B_ANY_TYPE, i, reinterpret_cast<const void**>(&data),
								   &size) == B_OK;
		 i++) {
		if (data->state == 0)
			SetConnectServer(data->serverName);
		else if (data->state == 1)
			++altCount;
	}
	SetAlternateCount(altCount);
}

void NetworkPrefsView::UpdatePersonalData(BMessage& msg)
{
	BString curIdent("");
	BString curRname("");
	BString curNick("");
	int32 count(fListView->CountItems()), i(0);
	for (i = 0; i < count; i++) delete (fListView->RemoveItem((int32)0));

	if ((msg.HasBool("useDefaults") && msg.FindBool("useDefaults"))) {
		BMessage defaults(vision_app->GetNetwork("defaults"));
		defaults.FindString("ident", &curIdent);
		defaults.FindString("realname", &curRname);
		for (i = 0; defaults.FindString("nick", i, &curNick) == B_OK; i++)
			fListView->AddItem(new BStringItem(curNick.String()));
		fIdent->SetEnabled(false);
		fRealName->SetEnabled(false);
		fNickAddButton->SetEnabled(false);
		fNickRemoveButton->SetEnabled(false);
		fNickDefaultsBox->SetValue(B_CONTROL_ON);
	} else {
		msg.FindString("ident", &curIdent);
		msg.FindString("realname", &curRname);
		for (i = 0; msg.FindString("nick", i, &curNick) == B_OK; i++)
			fListView->AddItem(new BStringItem(curNick.String()));
		fIdent->SetEnabled(true);
		fRealName->SetEnabled(true);
		fNickAddButton->SetEnabled(true);
		fNickDefaultsBox->SetValue(B_CONTROL_OFF);
	}

	if (fActiveNetwork.what != VIS_NETWORK_DEFAULTS) fNickDefaultsBox->SetEnabled(true);

	if (curIdent.Length())
		fIdent->SetText(curIdent.String());
	else
		fIdent->SetText("");
	if (curRname.Length())
		fRealName->SetText(curRname.String());
	else
		fRealName->SetText("");

	if (fListView->CountItems() == 0) fNickRemoveButton->SetEnabled(false);
}

void NetworkPrefsView::SetupDefaults(BMessage& msg)
{
	// disable appropriate controls
	fTextView->MakeEditable(false);
	fTextView->SetText("");
	fServerButton->SetEnabled(false);
	SetConnectServer("<N/A>");
	SetAlternateCount(0);
	fNickDefaultsBox->SetEnabled(false);
	fStartupBox->SetEnabled(false);
	fLagCheckBox->SetEnabled(false);
	// update personal data
	UpdatePersonalData(msg);
}

void NetworkPrefsView::SaveCurrentNetwork(void)
{
	if (fActiveNetwork.FindString("name") == NULL) return;

	// check real name and fIdent, update if needed
	if (fNickDefaultsBox->Value() == 0) {
		const char* curRname(fRealName->Text());
		if (curRname != NULL) {
			if (!fActiveNetwork.HasString("realname"))
				fActiveNetwork.AddString("realname", curRname);
			else
				fActiveNetwork.ReplaceString("realname", curRname);
		}

		const char* curIdent(fIdent->Text());
		if (curIdent != NULL) {
			if (!fActiveNetwork.HasString("ident"))
				fActiveNetwork.AddString("ident", curIdent);
			else
				fActiveNetwork.ReplaceString("ident", curIdent);
		}
	}

	// Not usually the best thing, but lets just do this until we
	// split the functionality into a function.
	BMessage m(M_NETPREFS_TEXT_INVOKE);
	MessageReceived(&m);

	const char* name(fActiveNetwork.FindString("name"));

	if (!strcmp(fActiveNetwork.FindString("name"), "defaults")) {
		vision_app->SetNetwork("defaults", &fActiveNetwork);
		return;
	}

	vision_app->SetNetwork(name, &fActiveNetwork);
}


void
NetworkPrefsView::FrameResized(float width, float)
{
	// Move/resize net details and personal boxes relative to main box:
	BRect bounds = fMainNetBox->InnerFrame();

	bounds.InsetBy(kItemSpacing, kItemSpacing);
	bounds.right /= 2;
	bounds.right - kItemSpacing;

	bounds.bottom = fStartupBox->Frame().top - kItemSpacing;

	fNetDetailsBox->MoveTo(bounds.LeftTop());
	fNetDetailsBox->ResizeTo(bounds.Width(), bounds.Height());

	bounds.OffsetBy(bounds.Width() + kItemSpacing, 0);

	fPersonalBox->MoveTo(bounds.LeftTop());
	fPersonalBox->ResizeTo(bounds.Width(), bounds.Height());

	// Resize Autoexec box relative to net details box:
	fExecScroller->LockLooper();

	bounds.bottom = fLagCheckBox->Frame().top - kItemSpacing;
	bounds.top = fExecScroller->Frame().top;

	float heightDelta = bounds.Height() - fExecScroller->Frame().Height();

	fExecScroller->ResizeBy(0, heightDelta);		
	fExecScroller->UnlockLooper();
}


void NetworkPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_NETWORK_DEFAULTS:
	{
		if (fActiveNetwork.HasString("name"))
			vision_app->SetNetwork(fActiveNetwork.FindString("name"), &fActiveNetwork);
		fActiveNetwork = vision_app->GetNetwork("defaults");
		fNetworkMenu->MenuItem()->SetLabel("Defaults");
		float width = StringWidth("Defaults");
		fNetworkMenu->ResizeTo(width + 30, 30);
		SetupDefaults(fActiveNetwork);
		fDupeItem->SetEnabled(false);
		fRemoveItem->SetEnabled(false);
		break;
	}

	case M_CHOOSE_NETWORK: {
		BMenuItem* item(NULL);
		msg->FindPointer("source", reinterpret_cast<void**>(&item));
		SaveCurrentNetwork();
		fActiveNetwork = vision_app->GetNetwork(item->Label());
		fNetworkMenu->MenuItem()->SetLabel(item->Label());
		float width = StringWidth(item->Label());
		fNetworkMenu->ResizeTo(width + 30, 30);
		UpdatePersonalData(fActiveNetwork);
		UpdateNetworkData(fActiveNetwork);
		if (BMessenger(fServerPrefs).IsValid()) fServerPrefs->SetNetworkData(&fActiveNetwork);
		fDupeItem->SetEnabled(true);
		fRemoveItem->SetEnabled(true);
	} break;

	case M_ADD_NEW_NETWORK: {
		if (msg->HasString("text")) {
			fNetPrompt = NULL;
			BString network(msg->FindString("text"));
			network.RemoveAll(" ");
			BMenu* menu(fNetworkMenu->Menu());
			for (int32 i = 0; i < menu->CountItems(); i++) {
				BMenuItem* item(menu->ItemAt(i));
				if (item && network == item->Label()) {
					dynamic_cast<BInvoker*>(item)->Invoke();
					return;
				}
			}
			BMessage newNet(VIS_NETWORK_DATA);
			newNet.AddString("name", network.String());
			vision_app->SetNetwork(network.String(), &newNet);
			BMenuItem* item(new BMenuItem(network.String(), new BMessage(M_CHOOSE_NETWORK)));
			menu->AddItem(item, 0);
			item->SetTarget(this);
			dynamic_cast<BInvoker*>(item)->Invoke();
		} else {
			fNetPrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 S_NETPREFS_NET_PROMPT, S_NETPREFS_ADDNET_TITLE, NULL, this,
								 new BMessage(M_ADD_NEW_NETWORK), NULL, false);
			fNetPrompt->Show();
		}
	} break;

	case M_REMOVE_CURRENT_NETWORK: {
		const char* name(fActiveNetwork.FindString("name"));
		vision_app->RemoveNetwork(name);
		BMenu* menu(fNetworkMenu->Menu());
		for (int32 i = 0; i < menu->CountItems(); i++) {
			BMenuItem* item(menu->ItemAt(i));
			if (!strcmp(item->Label(), name)) {
				delete menu->RemoveItem(i);
				fActiveNetwork.MakeEmpty();
				dynamic_cast<BInvoker*>(menu->ItemAt(0))->Invoke();
				break;
			}
		}
	} break;

	case M_DUPE_CURRENT_NETWORK: {
		if (msg->HasString("text")) {
			fDupePrompt = NULL;
			BString network(msg->FindString("text"));
			network.RemoveAll(" ");
			BMenu* menu(fNetworkMenu->Menu());
			for (int32 i = 0; i < menu->CountItems(); i++) {
				BMenuItem* item(menu->ItemAt(i));
				if (item && network == item->Label()) {
					dynamic_cast<BInvoker*>(item)->Invoke();
					return;
				}
			}
			BMessage newNet = fActiveNetwork;
			newNet.ReplaceString("name", network.String());
			vision_app->SetNetwork(network.String(), &newNet);
			BMenuItem* item(new BMenuItem(network.String(), new BMessage(M_CHOOSE_NETWORK)));
			menu->AddItem(item, 0);
			item->SetTarget(this);
			dynamic_cast<BInvoker*>(item)->Invoke();
		} else {
			fDupePrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 S_NETPREFS_NET_PROMPT, S_NETPREFS_DUPENET_TITLE, NULL, this,
								 new BMessage(M_DUPE_CURRENT_NETWORK), NULL, false);
			fDupePrompt->Show();
		}
	} break;

	case M_SERVER_DATA_CHANGED: {
		UpdateNetworkData(fActiveNetwork);
	} break;

	case M_SERVER_DIALOG: {
		BMessenger msgr(fServerPrefs);
		if (msgr.IsValid())
			fServerPrefs->Activate();
		else {
			fServerPrefs = new NetPrefServerWindow(this);
			fServerPrefs->SetNetworkData(&fActiveNetwork);
			fServerPrefs->Show();
		}
	} break;

	case M_NET_CHECK_LAG: {
		bool value = msg->FindInt32("be:value");
		if (fActiveNetwork.HasBool("lagCheck"))
			fActiveNetwork.ReplaceBool("lagCheck", value);
		else
			fActiveNetwork.AddBool("lagCheck", value);
	} break;

	case M_CONNECT_ON_STARTUP: {
		bool value = msg->FindInt32("be:value");
		if (fActiveNetwork.HasBool("connectOnStartup"))
			fActiveNetwork.ReplaceBool("connectOnStartup", value);
		else
			fActiveNetwork.AddBool("connectOnStartup", value);
	} break;

	case M_USE_NICK_DEFAULTS: {
		bool value = msg->FindInt32("be:value");
		if (fActiveNetwork.HasBool("useDefaults"))
			fActiveNetwork.ReplaceBool("useDefaults", value);
		else
			fActiveNetwork.AddBool("useDefaults", value);
		UpdatePersonalData(fActiveNetwork);
	} break;

	case M_NETPREFS_TEXT_INVOKE: {
		if (fActiveNetwork.HasString("autoexec"))
			fActiveNetwork.ReplaceString("autoexec", fTextView->Text());
		else
			fActiveNetwork.AddString("autoexec", fTextView->Text());
	} break;

	case M_ADD_NICK:
		if (msg->HasString("text")) {
			fNickPrompt = NULL;
			BString nick(msg->FindString("text"));
			nick.RemoveAll(" ");
			for (int32 i = 0; i < fListView->CountItems(); i++) {
				BStringItem* item((BStringItem*)fListView->ItemAt(i));
				if (item && nick == item->Text()) return;
			}
			fActiveNetwork.AddString("nick", nick.String());
			fListView->AddItem(new BStringItem(nick.String()));
		} else {
			fNickPrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 S_NETPREFS_ADDNICK_PROMPT, S_NETPREFS_ADDNICK_TITLE, NULL, this,
								 new BMessage(M_ADD_NICK), NULL, false);
			fNickPrompt->Show();
		}
		break;

	case M_REMOVE_NICK: {
		int32 current(fListView->CurrentSelection());
		if (current >= 0) {
			delete fListView->RemoveItem(current);
			fActiveNetwork.RemoveData("nick", current);
		}
	} break;

	case M_NICK_UP: {
		int32 current(fListView->CurrentSelection());
		BString nick1, nick2;
		nick1 = fActiveNetwork.FindString("nick", current);
		nick2 = fActiveNetwork.FindString("nick", current - 1);
		fListView->SwapItems(current, current - 1);
		fActiveNetwork.ReplaceString("nick", current - 1, nick1.String());
		fActiveNetwork.ReplaceString("nick", current, nick2.String());
		current = fListView->CurrentSelection();
		Window()->DisableUpdates();
		fListView->DeselectAll();
		fListView->Select(current);
		Window()->EnableUpdates();
	} break;

	case M_NICK_DOWN: {
		int32 current(fListView->CurrentSelection());
		BString nick1, nick2;
		nick1 = fActiveNetwork.FindString("nick", current);
		nick2 = fActiveNetwork.FindString("nick", current + 1);
		fListView->SwapItems(current, current + 1);
		fActiveNetwork.ReplaceString("nick", current + 1, nick1.String());
		fActiveNetwork.ReplaceString("nick", current, nick2.String());
		current = fListView->CurrentSelection();
		Window()->DisableUpdates();
		fListView->DeselectAll();
		fListView->Select(current);
		Window()->EnableUpdates();
	} break;

	case M_NICK_SELECTED: {
		int32 index(msg->FindInt32("index"));
		fNickRemoveButton->SetEnabled((bool)index >= 0);
	} break;

	default:
		BView::MessageReceived(msg);
		break;
	}
}
