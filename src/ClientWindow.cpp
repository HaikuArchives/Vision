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
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
#include <LayoutBuilder.h>
#include <ScrollView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <Rect.h>
#include <MessageRunner.h>
#include <TextControl.h>
#include <Roster.h>

#include <stdio.h>

#include "ChannelAgent.h"
#include "ClientAgent.h"
#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "IconMenu.h"
#include "ListAgent.h"
#include "Names.h"
#include "NetworkMenu.h"
#include "NotifyList.h"
#include "ServerAgent.h"
#include "SettingsFile.h"
#include "StatusView.h"
#include "Vision.h"
#include "WindowList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ClientWindow"

/*
  -- #beos was here --
  <Electroly> kurros has a l33t ass projector, I saw a picture of it just once
			  and I've been drooling ever since
  <T-ball> nice...
  <T-ball> I don't have room for a projector... :/
  <Brazilian> I have a monkey who draws on my wall really fast
*/

static const char* skTermSig = "application/x-vnd.Haiku-Terminal";

//////////////////////////////////////////////////////////////////////////////
/// Begin BWindow functions
//////////////////////////////////////////////////////////////////////////////

// small dynamic menu that sets the enabled/disabled states of its items on the fly when the user
// invokes it
// needed to correctly show the state of cut/copy/paste/select all

class DynamicEditMenu : public BMenu
{
public:
	DynamicEditMenu();
	virtual ~DynamicEditMenu();

	virtual void AllAttached();
	virtual void DetachedFromWindow();
};

DynamicEditMenu::DynamicEditMenu() : BMenu(B_TRANSLATE("Edit"))
{
}

DynamicEditMenu::~DynamicEditMenu()
{
}

void DynamicEditMenu::AllAttached()
{
	BMenu::AllAttached();
	vision_app->pClientWin()->SetEditStates(false);
}

void DynamicEditMenu::DetachedFromWindow()
{
	BMenu::DetachedFromWindow();
	for (int32 i = 0; i < CountItems(); i++) ItemAt(i)->SetEnabled(true);
}

ClientWindow::ClientWindow(BRect frame)
	: BWindow(frame, B_TRANSLATE_SYSTEM_NAME("Vision"),
		B_TITLED_WINDOW, B_ASYNCHRONOUS_CONTROLS)
{
	Init();
}

ClientWindow::~ClientWindow()
{

}

bool ClientWindow::QuitRequested()
{
	SaveSettings();
	fCwDock->SaveSettings();

	if (!fShutdown_in_progress) {
		fShutdown_in_progress = true;
		BMessage killMsg(M_CLIENT_QUIT);
		killMsg.AddBool("vision:winlist", true);
		killMsg.AddBool("vision:shutdown", fShutdown_in_progress);

		if (ServerBroadcast(&killMsg)) fWait_for_quits = true;

		if (fWait_for_quits) return false;
	}
	be_app_messenger.SendMessage(B_QUIT_REQUESTED);

	return true;
}

void ClientWindow::FrameMoved(BPoint origin)
{
	BWindow::FrameMoved(origin);
	vision_app->SetRect("windowDockRect", fCwDock->Bounds());
	vision_app->SetRect("clientWinRect", Frame());
}

void ClientWindow::FrameResized(float width, float height)
{
	BWindow::FrameResized(width, height);
	vision_app->SetRect("windowDockRect", fCwDock->Bounds());
	vision_app->SetRect("clientWinRect", Frame());
}

void ClientWindow::ScreenChanged(BRect screenframe, color_space mode)
{

	// user might have lowered resolution, may need to
	// move the window to a visible pos.

	if (Frame().top > screenframe.bottom) MoveTo(Frame().left, 110); // move up
	if (Frame().left > screenframe.right) MoveTo(110, Frame().top);  // move left

	BWindow::ScreenChanged(screenframe, mode);
}

void ClientWindow::AddMenu(BMenu* menu)
{
	if (menu != NULL) fMenuBar->AddItem(menu);
}

void ClientWindow::RemoveMenu(BMenu* menu)
{
	if (menu != NULL) fMenuBar->RemoveItem(menu);
}

void ClientWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	// this is somewhat annoying but these are needed in addition to the ones in the input filter
	// since the menu items
	// can't send the keyboard accelerators and only send the message ; annoying code duplication
	// but oh well
	case M_UP_CLIENT: {
		pWindowList()->Select(pWindowList()->CurrentSelection() - 1);
		pWindowList()->ScrollToSelection();
	} break;

	case M_DOWN_CLIENT: {
		pWindowList()->Select(pWindowList()->CurrentSelection() + 1);
		pWindowList()->ScrollToSelection();
	} break;

	case M_SMART_UP_CLIENT: {
		pWindowList()->ContextSelectUp();
	} break;

	case M_SMART_DOWN_CLIENT: {
		pWindowList()->ContextSelectDown();
	} break;

	case M_NETWORK_CLIENT: {
		pWindowList()->SelectServer();
	} break;

	case M_PREVIOUS_CLIENT: {
		pWindowList()->SelectLast();
	} break;

	case M_NETWORK_UP: {
		pWindowList()->MoveCurrentUp();
	} break;

	case M_NETWORK_DOWN: {
		pWindowList()->MoveCurrentDown();
	} break;

	case M_COLLAPSE_NETWORK: {
		pWindowList()->CollapseCurrentServer();
	} break;

	case M_EXPAND_NETWORK: {
		pWindowList()->ExpandCurrentServer();
	} break;

	case M_CW_UPDATE_STATUS: {
		WindowListItem* item(NULL), *superItem(NULL);
		int32 newstatus;

		if ((msg->FindPointer("item", reinterpret_cast<void**>(&item)) != B_OK) ||
			(msg->FindInt32("status", ((int32*)&newstatus)) != B_OK)) {
			printf(":ERROR: no valid pointer and int found in M_CW_UPDATE_STATUS, bailing...\n");
			return;
		}

		superItem = (WindowListItem*)pWindowList()->Superitem(item);

		if (superItem && (!superItem->IsExpanded())) {
			int32 srvstatus((superItem->SubStatus()));
			if (srvstatus < newstatus) superItem->SetSubStatus(newstatus);
		}

		if (!pWindowList()->FullListHasItem(item)) return;

		if (msg->HasBool("hidden")) {
			item->SetStatus(newstatus);
			return;
		}

		if (item->Status() >= newstatus)
			return;
		else
			item->SetStatus(newstatus);
	} break;

	case M_STATUS_CLEAR: {
		fStatus->Clear();
		SetEditStates(true);
	} break;

	case M_OBITUARY: {
		WindowListItem* agentitem;
		BView* agentview;
		if ((msg->FindPointer("agent", reinterpret_cast<void**>(&agentview)) != B_OK) ||
			(msg->FindPointer("item", reinterpret_cast<void**>(&agentitem)) != B_OK)) {
			printf(":ERROR: no valid pointers found in M_OBITUARY, bailing...\n");
			return;
		}

		pWindowList()->RemoveAgent(agentitem);

		if (fShutdown_in_progress && pWindowList()->CountItems() == 0)
			PostMessage(B_QUIT_REQUESTED);

	} break;

	case M_CW_ALTW: {
		if (!fAltw_catch && vision_app->GetBool("catchAltW")) {
			fAltw_catch = true;

			fAltwRunner = new BMessageRunner(this, new BMessage(M_CW_ALTW_RESET),
											 400000, // 0.4 seconds
											 1);
		} else
			PostMessage(B_QUIT_REQUESTED);
	} break;

	case M_CW_ALTW_RESET: {
		fAltw_catch = false;
		if (fAltwRunner) delete fAltwRunner;
	} break;

	case M_CW_ALTP: {
		pWindowList()->CloseActive();
	} break;

	case M_STATE_CHANGE: {
		ServerBroadcast(msg);
	} break;

	case M_MAKE_NEW_NETWORK: {
		BMessage network;
		msg->FindMessage("network", &network);
		BString netName(network.FindString("name"));
		pWindowList()->AddAgent(
			new ServerAgent(netName.String(), network), netName.String(),
			WIN_SERVER_TYPE,
			pWindowList()->CountItems() > 0 ? false : true); // grab focus if none present
	} break;

	case M_OPEN_TERM: {
		status_t result = be_roster->Launch(skTermSig, 0, NULL);
		if (result != B_OK) {
			BMessage errMsg(M_DISPLAY);
			BString errString("Launch failed! Error code: ");
			errString << result;
			errString += "\n";
			ClientAgent::PackDisplay(&errMsg, errString.String(), C_ERROR, C_BACKGROUND, F_TEXT);
			WindowListItem* item(dynamic_cast<WindowListItem*>(
				pWindowList()->ItemAt(pWindowList()->CurrentSelection())));
			if (item) {
				BMessenger(item->pAgent()).SendMessage(&errMsg);
			}
		}
	} break;

	case M_LIST_COMMAND: {
		WindowListItem* item(
			(WindowListItem*)pWindowList()->ItemAt(pWindowList()->CurrentSelection()));
		BView* view(NULL);
		if (item) view = item->pAgent();
		if ((view == NULL) || (dynamic_cast<ListAgent*>(view) != NULL)) break;
		dynamic_cast<ClientAgent*>(view)->ParseCmd("/LIST");
	} break;

	case M_JOIN_CHANNEL: {
		WindowListItem* item(
			(WindowListItem*)pWindowList()->ItemAt(pWindowList()->CurrentSelection()));
		BView* view(NULL);
		if (item) view = item->pAgent();
		if ((view == NULL) || (dynamic_cast<ListAgent*>(view) != NULL)) break;
		BString cmd;
		if (msg->FindString("channel", &cmd) < B_OK) break;
		cmd.Prepend("/JOIN ");
		dynamic_cast<ClientAgent*>(view)->ParseCmd(cmd.String());
		// XXX: should select the channel if already joined.
	} break;

	case M_NOTIFYLIST_UPDATE: {
		BObjectList<NotifyListItem>* nickList(NULL);
		BView* msgSource(NULL);
		int32 hasChanged(0);

		// whether we're the active one or not, no difference in state, ignore
		if (msg->HasInt32("change") && (hasChanged = msg->FindInt32("change")) == 0) break;

		msg->FindPointer("source", reinterpret_cast<void**>(&msgSource));
		msg->FindPointer("list", reinterpret_cast<void**>(&nickList));

		WindowListItem* item(
			(WindowListItem*)pWindowList()->ItemAt(pWindowList()->CurrentSelection()));
		if (item != NULL) {
			if (item->pAgent() == msgSource)
				pNotifyList()->UpdateList(nickList);
			else {
				item = (WindowListItem*)(pWindowList()->Superitem(item));
				if ((item != NULL) && (item->pAgent() == msgSource))
					pNotifyList()->UpdateList(nickList);
			}
			pWindowList()->BlinkNotifyChange(hasChanged, (ServerAgent*)msgSource);
		}

	} break;

	default:
		BWindow::MessageReceived(msg);
	}
}

ServerAgent* ClientWindow::GetTopServer(WindowListItem* request) const
{
	ServerAgent* target(NULL);
	if (pWindowList()->FullListHasItem(request)) {
		// if the item has no super, it is a server agent, return it.
		if (pWindowList()->Superitem(request) == NULL)
			target = dynamic_cast<ServerAgent*>(request->pAgent());
		else
			target = dynamic_cast<ServerAgent*>(
				((WindowListItem*)pWindowList()->Superitem(request))->pAgent());
	}
	return target;
}

void ClientWindow::SetEditStates(bool retargetonly)
{
	WindowListItem* item(
		dynamic_cast<WindowListItem*>(pWindowList()->ItemAt(pWindowList()->CurrentSelection())));

	if (item != NULL) {
		ClientAgent* agent(dynamic_cast<ClientAgent*>(item->pAgent()));
		if (agent != NULL) agent->SetEditStates(fEdit, retargetonly);
	}
}

WindowList* ClientWindow::pWindowList() const
{
	return fCwDock->pWindowList();
}

NotifyList* ClientWindow::pNotifyList() const
{
	return fCwDock->pNotifyList();
}

ClientWindowDock* ClientWindow::pCwDock() const
{
	return fCwDock;
}

StatusView* ClientWindow::pStatusView() const
{
	return fStatus;
}

/*
  -- #beos was here --
  <AnEvilYak> regurg, you don't like my monkey??
  <regurg> Oh, I don't like your monkey?
  <AnEvilYak> regurg: no :( *sob*
  <regurg> Why 'no'?
  <AnEvilYak> regurg: you tell me
  <regurg> What makes you think I tell you?
*/

bool ClientWindow::ServerBroadcast(BMessage* outmsg_) const
{
	bool reply(false);

	for (int32 i(0); i < pWindowList()->CountItems(); i++) {
		WindowListItem* aitem((WindowListItem*)pWindowList()->ItemAt(i));
		if (aitem->Type() == WIN_SERVER_TYPE) {
			dynamic_cast<ServerAgent*>(aitem->pAgent())->fMsgr.SendMessage(outmsg_);
			reply = true;
		}
	}

	return reply;
}

void ClientWindow::Show()
{
	// nothing yet
	BWindow::Show();
}

//////////////////////////////////////////////////////////////////////////////
/// End BWindow functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin Private Functions
//////////////////////////////////////////////////////////////////////////////

void ClientWindow::Init()
{
	SetSizeLimits(330, 2000, 150, 2000);

	fShutdown_in_progress = false;
	fWait_for_quits = false;
	fAltw_catch = false;

	AddShortcut('W', B_COMMAND_KEY, new BMessage(M_CW_ALTW));
	AddShortcut('Q', B_COMMAND_KEY, new BMessage(M_CW_ALTW));

	BRect frame(Bounds());
	fMenuBar = new BMenuBar("menu_bar");

	BMenuItem* item;
	BMenu* menu;
	// Server menu

	menu = new BMenu("App");

	menu->AddItem(
		item = new BMenuItem(B_TRANSLATE("About Vision"), new BMessage(B_ABOUT_REQUESTED)));
	item->SetTarget(vision_app);
	menu->AddItem(
		item = new BMenuItem(B_TRANSLATE("Help"), new BMessage(M_HELP_SHOW)));
	item->SetTarget(vision_app);
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Preferences" B_UTF8_ELLIPSIS), new BMessage(M_PREFS_SHOW)));
	item->SetTarget(vision_app);

	menu->AddSeparatorItem();
	menu->AddItem(
		item = new BMenuItem(B_TRANSLATE("Open Terminal"), new BMessage(M_OPEN_TERM), 'T', B_OPTION_KEY));
	menu->AddSeparatorItem();
	menu->AddItem(item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED)));
	item->SetTarget(vision_app);

	fApp = new TIconMenu(menu);
	fMenuBar->AddItem(fApp);

	fServer = new BMenu(B_TRANSLATE("Server"));
	fServer->AddItem(menu = new NetworkMenu(B_TRANSLATE("Connect to" B_UTF8_ELLIPSIS), M_CONNECT_NETWORK,
											BMessenger(vision_app)));

	fServer->AddItem(item = new BMenuItem(B_TRANSLATE("Setup" B_UTF8_ELLIPSIS),
										  new BMessage(M_SETUP_SHOW), '/', B_SHIFT_KEY));
	item->SetTarget(vision_app);
	fServer->AddSeparatorItem();
	fServer->AddItem(
		item = new BMenuItem(B_TRANSLATE("List channels" B_UTF8_ELLIPSIS), new BMessage(M_LIST_COMMAND), 'L'));
	fMenuBar->AddItem(fServer);

	// Edit menu
	fEdit = new DynamicEditMenu();

	fEdit->AddItem(item = new BMenuItem(B_TRANSLATE("Cut"), new BMessage(B_CUT), 'X'));
	fEdit->AddItem(item = new BMenuItem(B_TRANSLATE("Copy"), new BMessage(B_COPY), 'C'));
	fEdit->AddItem(item = new BMenuItem(B_TRANSLATE("Paste"), new BMessage(B_PASTE), 'V'));
	fEdit->AddItem(
		item = new BMenuItem(B_TRANSLATE("Select all"), new BMessage(B_SELECT_ALL), 'A', B_OPTION_KEY));
	fMenuBar->AddItem(fEdit);

	// Window menu
	fWindow = new BMenu(B_TRANSLATE("Window"));

	fWindow->AddItem(item = new BMenuItem(B_TRANSLATE("Up"), new BMessage(M_UP_CLIENT), B_UP_ARROW));
	fWindow->AddItem(
		item = new BMenuItem(B_TRANSLATE("Down"), new BMessage(M_DOWN_CLIENT), B_DOWN_ARROW));

	// bowser muscle memory
	AddShortcut(',', B_COMMAND_KEY, new BMessage(M_UP_CLIENT));
	AddShortcut('.', B_COMMAND_KEY, new BMessage(M_DOWN_CLIENT));

	fWindow->AddItem(item = new BMenuItem(B_TRANSLATE("Smart up"), new BMessage(M_SMART_UP_CLIENT),
										  B_UP_ARROW, B_SHIFT_KEY));
	fWindow->AddItem(item = new BMenuItem(B_TRANSLATE("Smart down"), new BMessage(M_SMART_DOWN_CLIENT),
										  B_DOWN_ARROW, B_SHIFT_KEY));
	fWindow->AddItem(item =
						 new BMenuItem(B_TRANSLATE("Network window"), new BMessage(M_NETWORK_CLIENT), '/'));
	fWindow->AddItem(item = new BMenuItem(B_TRANSLATE("Previous window"), new BMessage(M_PREVIOUS_CLIENT),
										  '0', B_SHIFT_KEY));
	AddShortcut(B_INSERT, B_SHIFT_KEY, new BMessage(M_PREVIOUS_CLIENT));
	fWindow->AddSeparatorItem();
	fWindow->AddItem(
		item = new BMenuItem(B_TRANSLATE("Move network up"), new BMessage(M_NETWORK_UP), 'U', B_SHIFT_KEY));
	fWindow->AddItem(
		item = new BMenuItem(B_TRANSLATE("Move network down"), new BMessage(M_NETWORK_DOWN), 'D', B_SHIFT_KEY));
	fWindow->AddItem(
		item = new BMenuItem(B_TRANSLATE("Collapse network"), new BMessage(M_COLLAPSE_NETWORK), B_LEFT_ARROW));
	fWindow->AddItem(
		item = new BMenuItem(B_TRANSLATE("Expand network"), new BMessage(M_EXPAND_NETWORK), B_RIGHT_ARROW));
	fWindow->AddSeparatorItem();
	fWindow->AddItem(item = new BMenuItem(B_TRANSLATE("Close this sub-window"), new BMessage(M_CW_ALTP), 'P'));

	item->SetTarget(this);
	fMenuBar->AddItem(fWindow);

	// add objects
	bgView = new BCardView("cards");
	bgView->AdoptSystemColors();

	fStatus = new StatusView(frame);
	fStatus->AddItem(new StatusItem("irc.elric.net", 0), true);

	fCwDock = new ClientWindowDock();

	BView* backgroundView = new BView("Background", 0);
	BView* mainView = new BView("MainView", 0);
	BLayoutBuilder::Group<>(mainView,B_VERTICAL,0)
			.SetInsets(0, -1, -1, -1)
			.Add(bgView);

	BLayoutBuilder::Group<>(backgroundView, B_HORIZONTAL,0)
		.SetInsets(0, 0, 0, 0)
		.AddSplit(B_HORIZONTAL, 0)
		.GetSplitView(&fSplitView)
			.Add(fCwDock, 1)
			.Add(mainView, 9);

	BLayoutBuilder::Group<>(this,B_VERTICAL,0)
		.Add(fMenuBar)
		.AddGroup(B_VERTICAL,0)
			.SetInsets(0, 0, 0, 0)
			.Add(backgroundView)
			.Add(fStatus)
		.End();

	fSplitView->SetItemWeight(0, vision_app->GetFloat("weight_WindowDock"), false);
	fSplitView->SetItemWeight(1, vision_app->GetFloat("weight_BgView"), false);
	fSplitView->SetItemCollapsed(0, vision_app->GetBool("collapsed_WindowDock"));
	fSplitView->SetItemCollapsed(1, vision_app->GetBool("collapsed_BgView"));
}

void ClientWindow::SaveSettings()
{
	vision_app->SetFloat("weight_WindowDock", fSplitView->ItemWeight((int32)0));
	vision_app->SetFloat("weight_BgView", fSplitView->ItemWeight((int32)1));
	vision_app->SetBool("collapsed_WindowDock", fSplitView->IsItemCollapsed((bool)0));
	vision_app->SetBool("collapsed_BgView", fSplitView->IsItemCollapsed((bool)1));
}
//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////
