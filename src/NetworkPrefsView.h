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

#ifndef _NETWORKPREFSVIEW_H
#define _NETWORKPREFSVIEW_H

#include <View.h>
#include <Message.h>

class BBox;
class BMenuItem;
class BMenuField;
class BButton;
class BScrollView;
class BCheckBox;
class BTextView;
class BListView;
class BTextControl;
class BStringView;
class PromptWindow;
class NetPrefServerWindow;
class BColumnListView;
struct ServerData;

class NetworkPrefsView : public BView
{
public:
	NetworkPrefsView(BRect, const char*);
	virtual ~NetworkPrefsView();
	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();
	virtual void FrameResized(float width, float height);

private:
	void SetConnectServer(const char*);
	void SetAlternateCount(uint32);
	void UpdateNetworkData(BMessage&);
	void UpdatePersonalData(BMessage&);
	void SetupDefaults(BMessage&);
	void BuildNetworkList();
	void SaveCurrentNetwork();
	BMenuField* fNetworkMenu;
	BScrollView* fExecScroller, *fNickScroller;

	BBox *fNetDetailsBox, *fPersonalBox;
	BView* fMainNetContainerBox, *fNetDetailsContainerBox, *fPersonalContainerBox;

	BButton* fServerButton, *fNickAddButton, *fNickRemoveButton;

	BCheckBox* fNickDefaultsBox, *fLagCheckBox, *fStartupBox;

	BTextView* fTextView;
	BListView* fListView;

	BTextControl* fIdent, *fRealName;

	BStringView* fConnectServer, *fAlternates;

	BMessage fActiveNetwork;
	PromptWindow* fNickPrompt;
	PromptWindow* fNetPrompt;
	PromptWindow* fDupePrompt;
	BMenuItem* fRemoveItem;
	BMenuItem* fDupeItem;
	NetPrefServerWindow* fServerPrefs;
};

#endif
