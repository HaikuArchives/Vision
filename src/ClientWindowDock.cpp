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
 */
#include <LayoutBuilder.h>
#include <GroupLayout.h>
#include <ScrollView.h>
#include <StringView.h>

#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "NotifyList.h"
#include "Theme.h"
#include "Vision.h"
#include "WindowList.h"

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ClientWindowDock"

static float label_height()
{
	return 8 + ceilf(be_plain_font->Size());
}

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDock functions
//////////////////////////////////////////////////////////////////////////////

ClientWindowDock::ClientWindowDock()
	: BView("agentDock", B_WILL_DRAW | B_FRAME_EVENTS),
	  fNotifyExpanded(false)
{
	AdoptSystemColors();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddSplit(B_VERTICAL, 0)
			.Add(fWinListAgent = new AgentDockWinList())
			.Add(fNotifyAgent = new AgentDockNotifyList())
		.End();
}

ClientWindowDock::~ClientWindowDock()
{
	//
}

WindowList* ClientWindowDock::pWindowList() const
{
	return fWinListAgent->pWindowList();
}

NotifyList* ClientWindowDock::pNotifyList() const
{
	return fNotifyAgent->pNotifyList();
}

void ClientWindowDock::AllAttached()
{	// TODO Replace this with the actual size
	/*if (vision_app->GetBool("notifyExpanded")) {
		BMessenger dockMsgr(this);
		dockMsgr.SendMessage(M_NOTIFYLIST_RESIZE);
	}*/
}

void ClientWindowDock::MessageReceived(BMessage* msg)
{
	switch (msg->what) { /*
	case M_NOTIFYLIST_RESIZE: {
		if (fNotifyExpanded) {
			fNotifyAgent->ResizeTo(fNotifyAgent->Bounds().Width(), label_height() - 2);
			fNotifyAgent->MoveTo(0.0, Bounds().bottom - label_height() + 2);
			fWinListAgent->ResizeBy(
				0.0, (fNotifyAgent->Frame().top - fWinListAgent->Frame().bottom - 1.0));
			fNotifyExpanded = false;
		} else {
			fWinListAgent->ResizeBy(0.0, -1.0 * (fWinListAgent->Bounds().Height() / 3.0));
			fNotifyAgent->MoveTo(0.0, fWinListAgent->Frame().bottom + 1.0);
			fNotifyAgent->ResizeBy(0.0, Bounds().bottom - fNotifyAgent->Frame().bottom);
			fNotifyExpanded = true;
		}
		vision_app->SetBool("notifyExpanded", fNotifyExpanded);
		break;
	}
*/
	default:
		BView::MessageReceived(msg);
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDock functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockWinList functions
//////////////////////////////////////////////////////////////////////////////

AgentDockWinList::AgentDockWinList()
	: BView("agentDockWinList", B_WILL_DRAW)
{
	SetViewColor(vision_app->GetColor(C_WINLIST_BACKGROUND));
	SetLayout(new BGroupLayout(B_VERTICAL, 0));

	fAHeader = new AgentDockHeader(B_TRANSLATE("Window list"));
	AddChild(fAHeader);
	fWinList = new WindowList();
	fWinListScroll =
		new BScrollView("fWinListScroll", fWinList, 0, false, true, B_PLAIN_BORDER);
	AddChild(fWinListScroll);
}

AgentDockWinList::~AgentDockWinList()
{
	//
}

WindowList* AgentDockWinList::pWindowList() const
{
	return fWinList;
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockWinList functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockNotifyList functions
//////////////////////////////////////////////////////////////////////////////

AgentDockNotifyList::AgentDockNotifyList()
	: BView("agentDockNotifyList",  B_WILL_DRAW)
{
	SetLayout(new BGroupLayout(B_VERTICAL, 0));
	SetViewColor(vision_app->GetColor(B_PANEL_BACKGROUND_COLOR));

	fAHeader =	new AgentDockHeader(B_TRANSLATE("Notify list"));
	fNotifyList = new NotifyList();
	fNotifyScroll = new BScrollView("fNotifyListScroll", fNotifyList, 0, false, true,
									B_PLAIN_BORDER);
	AddChild(fAHeader);
	AddChild(fNotifyScroll);
}

AgentDockNotifyList::~AgentDockNotifyList()
{
	//
}

NotifyList* AgentDockNotifyList::pNotifyList() const
{
	return fNotifyList;
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockNotifyList functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////

/*
 * Class Purpose: Provides the visual header for AgentDock Agents
 */

AgentDockHeaderString::AgentDockHeaderString(const char* name)
	: BStringView("fHeaderView", name)
{
}

AgentDockHeaderString::~AgentDockHeaderString()
{
}

void AgentDockHeaderString::MouseMoved(BPoint where, uint32 transitcode, const BMessage* mmMsg)
{
	BStringView::MouseMoved(where, transitcode, mmMsg);
}

void AgentDockHeaderString::MouseDown(BPoint where)
{
/*	SetViewUIColor(B_MENU_BACKGROUND_COLOR, B_DARKEN_2_TINT);
	Parent()->SetViewUIColor(B_MENU_BACKGROUND_COLOR, B_DARKEN_2_TINT);*/
	BStringView::MouseDown(where);
}

void AgentDockHeaderString::MouseUp(BPoint where)
{ /*
	SetViewUIColor(B_MENU_BACKGROUND_COLOR, B_DARKEN_1_TINT);
	Parent()->SetViewUIColor(B_MENU_BACKGROUND_COLOR, B_DARKEN_1_TINT);

	// check if this header string belongs to notify list and send resize message if so
	BView* notifyList(Parent());
	if (notifyList && dynamic_cast<AgentDockHeader*>(notifyList) != NULL) {
		notifyList = notifyList->Parent();
		if (notifyList && dynamic_cast<AgentDockNotifyList*>(notifyList) != NULL) {
			BMessenger msgr(((AgentDockNotifyList*)notifyList)->pNotifyList());
			if (msgr.IsValid()) msgr.SendMessage(M_NOTIFYLIST_RESIZE);
		}
	}
*/
	BStringView::MouseUp(where);
}

AgentDockHeader::AgentDockHeader(const char* name)
	: BView("AgentDockHeader", B_WILL_DRAW)
{
	AdoptSystemColors();
	fHeaderView = new AgentDockHeaderString(name);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.Add(fHeaderView)
		.AddGlue()
		.End();
}

AgentDockHeader::~AgentDockHeader()
{
	// nothing
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////
