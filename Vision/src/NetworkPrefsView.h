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
class TSpeedButton;
struct ServerData;

class NetworkPrefsView : public BView
{
	public:

							NetworkPrefsView (BRect, const char *);
	virtual					~NetworkPrefsView (void);
	virtual void			MessageReceived (BMessage *);
	virtual void			AttachedToWindow (void);
	virtual void			DetachedFromWindow (void);

	private:
	
	void					UpdateNetworkData (BMessage &);
	void					UpdatePersonalData (BMessage &);
	void					SetupDefaults (BMessage &);
	void					BuildNetworkList (void);
	void					SaveCurrentNetwork();
	void					SetPrimaryServer(const char *serverName);
	void					SetAlternateCount(uint32 altCount);
	
	BMenuField				*fNetworkMenu;
	BScrollView				*fExecScroller,
							*fNickScroller;
	
	BBox					*fMainNetBox,
							*fNetDetailsBox,
							*fPersonalBox;
	
	BButton					*fServerButton,
							*fNickAddButton,
							*fNickRemoveButton;
	
	TSpeedButton			*fNickUpButton,
							*fNickDnButton;
	
	BCheckBox				*fNickDefaultsBox,
							*fLagCheckBox,
							*fStartupBox;
				
	BTextView				*fTextView;
	BListView				*fListView;
	
	BTextControl			*fIdent,
							*fRealName;
	
	BMessage				fActiveNetwork;
	PromptWindow			*fNickPrompt;
	PromptWindow			*fNetPrompt;
	PromptWindow			*fDupePrompt;
	BMenuItem				*fRemoveItem;
	BMenuItem				*fDupeItem;
	NetPrefServerWindow 	*fServerPrefs;
	BStringView				*fConnectServer,
							*fAlternates;
};

#endif
