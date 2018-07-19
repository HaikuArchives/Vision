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
 */

#ifndef _WINDOWLIST_H_
#define _WINDOWLIST_H_

#include <ControlLook.h>
#include <ListItem.h>
#include <OutlineListView.h>
#include <SplitView.h>
#include <String.h>

class BMessageRunner;
class BPopUpMenu;
class BMenu;
class BList;
class ServerAgent;
class ClientAgent;
class ClientWindow;
class BScrollView;
class Theme;

class AgentCard : public BSplitView {

public:	AgentCard(BView *agent);

		BView*	GetAgent() const;
		bool	IsChannelAgent() const;
private:

		BView* fAgent;

};

class WindowListItem : public BListItem
{
public:
	WindowListItem(const char*, int32, int32, AgentCard*);
	virtual ~WindowListItem();
	BString Name() const;
	int32 Type() const;
	int32 Status() const;
	int32 SubStatus() const;
	int32 BlinkState() const;
	BView* pAgent() const;
	AgentCard* pAgentCard() const;

	void SetName(const char*);
	void SetStatus(int32);
	void SetSubStatus(int32);
	void ActivateItem();

	virtual void DrawItem(BView*, BRect, bool complete = false);
	void SetNotifyBlinker(int32);
	void SetBlinkState(int32);

private:
	BString fMyName;
	int32 fMyStatus;
	int32 fMyType;
	int32 fSubStatus; // servers only -- status of collapsed children
	BView* fMyAgent;
	AgentCard* fAgentCard;
	int32 fBlinkState;
	int32 fBlinkStateCount;
	BMessageRunner* fBlinker;
};

class WindowList : public BOutlineListView
{
public:
	WindowList();
	virtual ~WindowList();
	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();
	virtual void MouseDown(BPoint);
	virtual void MessageReceived(BMessage*);
	virtual void SelectionChanged();
	virtual void KeyDown(const char*, int32);

	void ClearList();
	void Activate(int32);
	void CloseActive();
	void SelectLast();
	void CollapseCurrentServer();
	void ExpandCurrentServer();

	void BlinkNotifyChange(int32, ServerAgent*);

	int32 GetServer(int32);
	void SelectServer();

	void ContextSelectUp();
	void ContextSelectDown();

	void MoveCurrentUp();
	void MoveCurrentDown();

	//    ClientAgent                     *Agent (int32, const char *);

	void AddAgent(BView*, const char*, int32, bool);
	void RemoveAgent(WindowListItem*);
	void Expand(BListItem*);
	void Collapse(BListItem*);

	void SaveSplitSettings(AgentCard* agentCard);
	void ApplySplitSettings(AgentCard* agentCard);
protected:
	virtual void				DrawLatch(BRect itemRect, int32 level,
									bool collapsed, bool highlighted,
									bool misTracked);
	virtual	void				DrawItem(BListItem* item, BRect itemRect,
									bool complete = false);

private:
	BPopUpMenu* fMyPopUp;
	WindowListItem* fLastSelected;

	Theme* fActiveTheme;

	void BuildPopUp();

	int16 fLastButton, fClickCount;

	BPoint fLastClick;
	bigtime_t fLastClickTime;
	WindowListItem* fCurrentDrawItem;
	static int SortListItems(const BListItem*, const BListItem*);
};

#endif
