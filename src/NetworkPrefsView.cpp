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
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <ListItem.h>
#include <ListView.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MessageFormat.h>
#include <SeparatorView.h>
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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NetworkPrefsView"

class InvokingTextView : public BTextView, public BInvoker
{
public:
	InvokingTextView(const char*, BMessage*, BHandler*);
	virtual ~InvokingTextView();

	virtual void KeyDown(const char*, int32);
};

InvokingTextView::InvokingTextView(const char* name, BMessage* msg, BHandler* target)
	: BTextView(name),
	  BInvoker(msg, target)
{
}

InvokingTextView::~InvokingTextView()
{
}

void InvokingTextView::KeyDown(const char* bytes, int32 numBytes)
{
	BTextView::KeyDown(bytes, numBytes);
	Invoke();
}

NetworkPrefsView::NetworkPrefsView(const char* name)
	: BView(name, B_WILL_DRAW | B_FRAME_EVENTS),
	  fNickPrompt(NULL),
	  fNetPrompt(NULL),
	  fDupePrompt(NULL),
	  fServerPrefs(NULL)
{
	AdoptSystemColors();

	BMenu* menu(new BMenu(B_TRANSLATE("Networks")));
	menu->AddItem(new BMenuItem(B_TRANSLATE("Defaults"), new BMessage(M_NETWORK_DEFAULTS)));
	menu->AddSeparatorItem();
	menu->AddItem(
		new BMenuItem(B_TRANSLATE("Add new" B_UTF8_ELLIPSIS), new BMessage(M_ADD_NEW_NETWORK)));
	menu->AddItem(fRemoveItem =
					  new BMenuItem(B_TRANSLATE("Remove current"), new BMessage(M_REMOVE_CURRENT_NETWORK)));
	menu->AddItem(fDupeItem = new BMenuItem(B_TRANSLATE("Duplicate current" B_UTF8_ELLIPSIS),
											new BMessage(M_DUPE_CURRENT_NETWORK)));
	fNetworkMenu = new BMenuField("NetList", NULL, menu);
	const float width = StringWidth(B_TRANSLATE("Defaults"));
	fNetworkMenu->SetExplicitSize(BSize(width + 30, B_SIZE_UNSET));

	// Create the Views
	BSeparatorView* fTitleView = new BSeparatorView(B_HORIZONTAL, B_FANCY_BORDER);
	fTitleView->SetLabel(fNetworkMenu, false);
	fTitleView->SetAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_UNSET));

	BSeparatorView* fTitlePaddingView = new BSeparatorView(B_HORIZONTAL, B_FANCY_BORDER);
	float padding = be_control_look->ComposeSpacing(B_USE_SMALL_SPACING) - 5;
	fTitlePaddingView->SetExplicitSize(BSize(padding, B_SIZE_UNSET));

	fNetDetailsBox = new BBox("NetDetailsBox");
	fNetDetailsContainerBox = new BView("NetDetailsContainerBox", 0);
	fNetDetailsContainerBox->SetLayout(new BGroupLayout(B_VERTICAL, 0));
	fNetDetailsBox->AddChild(fNetDetailsContainerBox);
	fNetDetailsBox->SetLabel(B_TRANSLATE("Network details"));

	fPersonalBox = new BBox("PersonalBox");
	fPersonalContainerBox = new BView("PersonalContainerBox", 0);
	fPersonalContainerBox->SetLayout(new BGroupLayout(B_VERTICAL, 0));
	fPersonalBox->AddChild(fPersonalContainerBox);
	fPersonalBox->SetLabel(B_TRANSLATE("Personal details"));

	BStringView* stringView1(new BStringView(NULL,
		B_TRANSLATE("Connects to server:")));

	fConnectServer = new BStringView(NULL, B_EMPTY_STRING);

	fAlternates = new BStringView(NULL, B_EMPTY_STRING);

	fServerButton = new BButton(NULL, B_TRANSLATE("Change servers" B_UTF8_ELLIPSIS),
								new BMessage(M_SERVER_DIALOG));

	BStringView* stringView4(new BStringView(NULL, B_TRANSLATE("Automatically execute:")));


	fTextView = new InvokingTextView(NULL, new BMessage(M_NETPREFS_TEXT_INVOKE), this);
	fExecScroller =
		new BScrollView(NULL, fTextView, 0, false, true);
	fTextView->MakeEditable(true);
	fTextView->SetStylable(false);

	fLagCheckBox = new BCheckBox(NULL, B_TRANSLATE("Enable lag checking"),
		new BMessage(M_NET_CHECK_LAG));

	fStartupBox = new BCheckBox(NULL, B_TRANSLATE("Auto-connect to this network"),
			new BMessage(M_CONNECT_ON_STARTUP));

	fNickDefaultsBox =
		new BCheckBox(NULL, B_TRANSLATE("Use defaults"), new BMessage(M_USE_NICK_DEFAULTS));

	// Preferred nicks:
	BStringView* stringView5(new BStringView(NULL, B_TRANSLATE("Preferred nicks:")));


	fListView = new BListView(NULL, B_SINGLE_SELECTION_LIST);
	fListView->SetSelectionMessage(new BMessage(M_NICK_SELECTED));

	fNickScroller = new BScrollView(NULL, fListView, 0, false, true);

	// Create buttons with fixed size
	font_height fontHeight;
	GetFontHeight(&fontHeight);
	const int16 buttonHeight = int16(fontHeight.ascent + fontHeight.descent + 12);
		// button size determined by font size
	BSize btnSize(buttonHeight, buttonHeight);

	fNickAddButton = new BButton("plus", "+", new BMessage(M_ADD_NICK));
	fNickAddButton->SetExplicitSize(btnSize);
	fNickRemoveButton = new BButton("minus", "-", new BMessage(M_REMOVE_NICK));
	fNickRemoveButton->SetExplicitSize(btnSize);

	BString text(B_TRANSLATE("Real name:"));
	text.Append(" ");
	fRealName = new BTextControl(NULL,
		text.String(), NULL, NULL);

	text = B_TRANSLATE("Ident:");
	text.Append(" ");

	fIdent = new BTextControl(NULL, text.String(), NULL, NULL);

	BLayoutBuilder::Group<>(fNetDetailsContainerBox, B_VERTICAL)
				.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
					.Add(stringView1)
					.Add(fConnectServer)
					.Add(fAlternates)
				.End()
				.Add(fServerButton)
				.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
					.Add(stringView4)
					.Add(fExecScroller)
				.End()
				.Add(fLagCheckBox)
		.SetInsets(B_USE_WINDOW_SPACING)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL, 0)
		.Add(fTitlePaddingView)
		.Add(fTitleView)
		.End()
		.SetInsets(0, B_USE_WINDOW_SPACING, 0, B_USE_WINDOW_SPACING)
			.AddGroup(B_VERTICAL, B_USE_DEFAULT_SPACING)
				.SetInsets(B_USE_SMALL_SPACING, 0, B_USE_SMALL_SPACING, 0)
				.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
					.Add(fNetDetailsBox)
					.Add(fPersonalBox)
				.End()
				.Add(fStartupBox)
			.End()
	.End();
	const int16 buttonSpacing = 1;
	BLayoutBuilder::Group<>(fPersonalContainerBox, B_VERTICAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fNickDefaultsBox)
			.AddStrut(B_USE_DEFAULT_SPACING)
			.AddGrid(0.0, 0.0)
				.AddTextControl(fIdent, 0, 0)
				.AddTextControl(fRealName, 0, 1)
			.End()
			.AddStrut(B_USE_DEFAULT_SPACING)
			.AddGroup(B_VERTICAL, B_USE_HALF_ITEM_SPACING)
				.Add(stringView5)
				.Add(fNickScroller)
			.End()
			.AddGroup(B_HORIZONTAL, 0, 0.0)
				// Add and Remove buttons
				.AddGroup(B_VERTICAL, 0, 0.0)
					.AddGroup(B_HORIZONTAL, 0, 0.0)
						.Add(new BSeparatorView(B_VERTICAL))
						.AddGroup(B_VERTICAL, 0, 0.0)
							.AddGroup(B_HORIZONTAL, buttonSpacing, 0.0)
								.SetInsets(buttonSpacing)
								.Add(fNickAddButton)
								.Add(fNickRemoveButton)
							.End()
							.Add(new BSeparatorView(B_HORIZONTAL))
						.End()
						.Add(new BSeparatorView(B_VERTICAL))
					.End()
					.End()
					.AddGlue()
				.End()

	.End();
}

NetworkPrefsView::~NetworkPrefsView()
{
	BMessenger(fNickPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fNetPrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fDupePrompt).SendMessage(B_QUIT_REQUESTED);
	BMessenger(fServerPrefs).SendMessage(B_QUIT_REQUESTED);
}

void NetworkPrefsView::AttachedToWindow()
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
}

void NetworkPrefsView::DetachedFromWindow()
{
	// save changes to active network
	// get autoexec commands

	SaveCurrentNetwork();
}

void NetworkPrefsView::BuildNetworkList()
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

	BString text;
	static BMessageFormat format(B_TRANSLATE("{0, plural,"
		"one{with a fallback to one other.}"
		"other{with a fallback to # others.}}"));
		format.Format(text, altCount);

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
	SetConnectServer(B_TRANSLATE_COMMENT("<N/A>", "as in 'not available"));
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
	for (int32 i = 0; msg.FindData("server", B_ANY_TYPE, i,
		reinterpret_cast<const void**>(&data), &size) == B_OK;
		 i++) {
		if (size < sizeof(ServerData)) {
			printf("WARNING! Old ServerData format loaded.\n");
			// FIXME: We really need to get rid of ServerData
			const_cast<ServerData*>(data)->secure = false;
		}
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
	SetConnectServer(B_TRANSLATE_COMMENT("<N/A>", "as in 'not available"));
	SetAlternateCount(0);
	fNickDefaultsBox->SetEnabled(false);
	fStartupBox->SetEnabled(false);
	fLagCheckBox->SetEnabled(false);
	// update personal data
	UpdatePersonalData(msg);
}

void NetworkPrefsView::SaveCurrentNetwork()
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


void NetworkPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_NETWORK_DEFAULTS:
	{
		if (fActiveNetwork.HasString("name"))
			vision_app->SetNetwork(fActiveNetwork.FindString("name"), &fActiveNetwork);
		fActiveNetwork = vision_app->GetNetwork("defaults");
		fNetworkMenu->MenuItem()->SetLabel(B_TRANSLATE("Defaults"));
		float width = StringWidth(B_TRANSLATE("Defaults"));
		fNetworkMenu->SetExplicitSize(BSize(width + 30, B_SIZE_UNSET));
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
		fNetworkMenu->SetExplicitSize(BSize(width + 30, B_SIZE_UNSET));
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
			BString text(B_TRANSLATE("Network name:"));
			text.Append(" ");
			fNetPrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 text.String(), B_TRANSLATE("Add network"), NULL, this,
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
			BString text(B_TRANSLATE("Network name:"));
			text.Append(" ");
			fDupePrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 text.String(), B_TRANSLATE("Duplicate network"), NULL, this,
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
			BString text(B_TRANSLATE("Nickname:"));
			text.Append(" ");
			fNickPrompt =
				new PromptWindow(BPoint(Window()->Frame().left + Window()->Frame().Width() / 2,
										Window()->Frame().top + Window()->Frame().Height() / 2),
								 text.String(), B_TRANSLATE("Add nickname"), NULL, this,
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
		bool enabled = (index >= 0) && fNickAddButton->IsEnabled();
		fNickRemoveButton->SetEnabled(enabled);
	} break;

	default:
		BView::MessageReceived(msg);
		break;
	}
}
