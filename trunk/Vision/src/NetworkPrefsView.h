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
class TSpeedButton;
struct ServerData;

class NetworkPrefsView : public BView
{
	public:

					NetworkPrefsView (BRect, const char *);
	virtual			~NetworkPrefsView (void);
	virtual void	MessageReceived (BMessage *);
	virtual void	AttachedToWindow (void);
	virtual void	DetachedFromWindow (void);

	private:
	
	void			SetConnectServer (const char *);
	void			SetAlternateCount (uint32);
	void			UpdateNetworkData (BMessage &);
	void			UpdatePersonalData (BMessage &);
	void			SetupDefaults (BMessage &);
	void            BuildNetworkList (void);
	void            SaveCurrentNetwork();
	BMenuField *networkMenu;
	BScrollView *execScroller,
				*nickScroller;
	
	BBox *mainNetBox,
		*netDetailsBox,
		*personalBox;
	
	BButton *serverButton,
			*execButton,
			*nickAddButton,
			*nickRemoveButton;
	
	TSpeedButton *nickUpButton,
	             *nickDnButton;
	
	BCheckBox *nickDefaultsBox,
			  *startupBox;
			  
	BTextView *textView;
	BListView *listView;
	
	BTextControl *ident,
				*realName;
	
	BStringView *connectServer,
				*alternates;
	
	BMessage	activeNetwork;
	PromptWindow *nickPrompt;
	PromptWindow *netPrompt;
	PromptWindow *dupePrompt;
	BMenuItem   *removeItem;
	BMenuItem   *dupeItem;
	NetPrefServerWindow *serverPrefs;
};

#endif
