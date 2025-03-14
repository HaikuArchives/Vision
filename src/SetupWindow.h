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
 *                 Todd Lair
 *                 Rene Gollent
 */

#ifndef _SETUPWINDOW_H_
#define _SETUPWINDOW_H_

#include <Window.h>

class BView;
class BButton;
class BMenuField;
class BMenu;
class BMessage;

class SetupWindow : public BWindow {
public:
	SetupWindow();
	virtual ~SetupWindow();
	virtual bool QuitRequested();
	virtual void MessageReceived(BMessage*);

private:
	void InitServerStartup();
	void BuildNetworkMenu();
	BButton *connectButton, *netPrefsButton, *prefsButton;
	BMenuField* netList;
};

#endif
