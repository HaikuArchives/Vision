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

#ifndef _LISTAGENT_H_
#define _LISTAGENT_H_

#include <Messenger.h>
#include <String.h>
#include <View.h>
#include <regex.h>

#include "Agent.h"
#include "ObjectList.h"

class BColumnListView;
class BColumn;
class BScrollView;
class BMenuItem;
class StatusView;
class WindowListItem;
class Theme;
class BRow;
class WindowList;

class ListAgent : public BView, public Agent {
public:
	ListAgent(const char*, BMessenger*);
	virtual ~ListAgent();
	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow();
	virtual void Show();
	virtual void Hide();
	virtual BView* View();

	BMessenger fMsgr;

private:
	void AddBatch();
	Theme* activeTheme;
	BMessenger* fSMsgr;
	BMenu* listMenu;
	BColumnListView* listView;
	BColumn *channelColumn, *usersColumn, *topicColumn;

	StatusView* status;

	BString filter, find, statusStr;
	regex_t re, fre;

	bool processing;

	BObjectList<BRow> hiddenItems, fBuildList;

	BMenuItem *mFilter, *mFind, *mFindAgain;
	friend class WindowList;
};

#endif
