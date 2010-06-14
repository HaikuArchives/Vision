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
 *								 Jamie Wilkinson
 */

#ifndef _CLIENTWINDOW_H_
#define _CLIENTWINDOW_H_

#include <Window.h>
#include <String.h>

#define STATUS_SERVER							 0
#define STATUS_LAG										1
#define STATUS_NICK								 2
#define STATUS_USERS									3
#define STATUS_OPS									4
#define STATUS_MODES									5
#define STATUS_META								 6

class BMenu;
class BMenuBar;
class BMenuItem;
class BScrollView;
class BView;
class BMessageRunner;
class ClientWindowDock;
class NotifyList;
class NotifyListItem;
class ResizeView;
class ServerAgent;
class StatusView;
class TIconMenu;
class WindowList;
class WindowListItem;

class ClientWindow : public BWindow
{

	protected:
		BMenuBar							*fMenuBar;
		BMenu								 *fServer,
													*fEdit,
													*fWindow;
		TIconMenu						 *fApp;

	public:
													ClientWindow (BRect);
		virtual							 ~ClientWindow (void);
		virtual void					FrameMoved (BPoint);
		virtual void					FrameResized (float width, float height);
		virtual void					MessageReceived (BMessage *);
		virtual bool					QuitRequested (void);
		virtual void					ScreenChanged (BRect, color_space);
		virtual void					Show (void);
		
		// accessors that add/remove menus to the main Vision menu bar
		void									AddMenu (BMenu *);
		void									RemoveMenu (BMenu *);
		
		ServerAgent					 *GetTopServer (WindowListItem *) const;
		
		bool									ServerBroadcast (BMessage *) const;
		
		void									SetEditStates(bool);

		BView								 *bgView;
			
		BRect								 *AgentRect (void) const;
		WindowList						*pWindowList (void) const;
		NotifyList						*pNotifyList (void) const;
		ClientWindowDock			*pCwDock (void) const;
		StatusView						*pStatusView (void) const;
		BString							 joinStrings;	// used to keep track of channel
																				// keys on u2 ircds

	private:
		void									Init (void);
		
		bool									fShutdown_in_progress;
		bool									fWait_for_quits;
		
		bool									fAltw_catch;		
		
		BMessageRunner				*fAltwRunner;
		
		BRect								 *fAgentrect;
		
		StatusView						*fStatus;
		
		ClientWindowDock			*fCwDock;
		
		ResizeView						*fResize;
		
};

#endif
