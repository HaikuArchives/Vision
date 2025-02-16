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

#ifndef _CLIENTWINDOWDOCK_H_
#define _CLIENTWINDOWDOCK_H_

#include <StringView.h>
#include <View.h>

class NotifyList;
class WindowList;
class BScrollView;
class BSplitView;

class AgentDockHeaderString : public BStringView {
public:
	AgentDockHeaderString(const char*);
	virtual ~AgentDockHeaderString();

	virtual void MouseMoved(BPoint, uint32, const BMessage*);
	virtual void MouseDown(BPoint);
	virtual void MouseUp(BPoint);
};

class AgentDockHeader : public BView {
public:
	AgentDockHeader(const char*);
	virtual ~AgentDockHeader();

private:
	AgentDockHeaderString* fHeaderView;
};

class AgentDockWinList : public BView {
public:
	AgentDockWinList();
	virtual ~AgentDockWinList();

	WindowList* pWindowList() const;

private:
	WindowList* fWinList;
	BScrollView* fWinListScroll;

	AgentDockHeader* fAHeader;
};

class AgentDockNotifyList : public BView {
public:
	AgentDockNotifyList();
	virtual ~AgentDockNotifyList();

	NotifyList* pNotifyList() const;

private:
	AgentDockHeader* fAHeader;
	NotifyList* fNotifyList;
	BScrollView* fNotifyScroll;
};

class ClientWindowDock : public BView {
public:
	ClientWindowDock();
	virtual ~ClientWindowDock();

	WindowList* pWindowList() const;
	NotifyList* pNotifyList() const;

	virtual void AllAttached();
	virtual void MessageReceived(BMessage*);
	void SaveSettings();

private:
	AgentDockWinList* fWinListAgent;
	AgentDockNotifyList* fNotifyAgent;
	bool fNotifyExpanded;
	BSplitView* fSplitView;
};

#endif
