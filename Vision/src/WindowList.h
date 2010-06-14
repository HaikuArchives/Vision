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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 */

#ifndef _WINDOWLIST_H_
#define _WINDOWLIST_H_

#include <ListItem.h>
#include <OutlineListView.h>
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

class WindowListItem : public BListItem
{
	public:
																		WindowListItem (const char *,
																										int32,
																										int32,
																										BView *);
		virtual												 ~WindowListItem (void);
		BString												 Name (void) const;
		int32													 Type (void) const;
		int32													 Status (void) const;
		int32													 SubStatus (void) const;
		int32													 BlinkState (void) const;
		BView													 *pAgent (void) const;

		void														SetName (const char *);
		void														SetStatus (int32);
		void														SetSubStatus (int32);
		void														ActivateItem (void);

		virtual void										DrawItem (BView *,
																							BRect,
																							bool complete = false);
		void														SetNotifyBlinker(int32);
		void														SetBlinkState(int32);
																							
	private:
		BString												 fMyName;
		int32													 fMyStatus;
		int32													 fMyType;
		int32													 fSubStatus; // servers only -- status of collapsed children
		BView													 *fMyAgent;
		int32													 fBlinkState;
		int32													 fBlinkStateCount;
		BMessageRunner									*fBlinker;
};


class WindowList : public BOutlineListView
{
	 public:
																		WindowList (BRect);
		virtual												 ~WindowList (void);
		virtual void										AttachedToWindow (void);
		virtual void										DetachedFromWindow (void);
		virtual void										MouseDown (BPoint);
		virtual void										MessageReceived (BMessage *);
		virtual void 					SelectionChanged (void);
		virtual void										KeyDown (const char *, int32);
	
		void														ClearList (void);
		void														Activate (int32);
		void														CloseActive (void);
		void														SelectLast (void);
		void														CollapseCurrentServer (void);
		void														ExpandCurrentServer (void);
		
		void														BlinkNotifyChange (int32, ServerAgent *);
		
		int32													 GetServer (int32);
		void														SelectServer (void);
		
		void														ContextSelectUp (void);
		void														ContextSelectDown (void);
		
		void														MoveCurrentUp (void);
		void														MoveCurrentDown (void);
		
//		ClientAgent										 *Agent (int32, const char *);
		
		void														AddAgent (BView *, const char *, int32, bool);
		void														RemoveAgent (BView *, WindowListItem *);
		void														Expand (BListItem *);
		void														Collapse (BListItem *);
	
	private:

		BPopUpMenu											*fMyPopUp;
		WindowListItem									*fLastSelected;
		
		Theme													 *fActiveTheme;
		
		void														BuildPopUp (void);
		
		int16													 fLastButton,
																		fClickCount;
		
		BPoint													fLastClick;
		bigtime_t											 fLastClickTime;
		

		static int											SortListItems (const BListItem *, const BListItem *);
		
};

#endif
