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
 */

#ifndef _CLIENTWINDOWDOCK_H_
#define _CLIENTWINDOWDOCK_H_

#include <View.h>
#include <StringView.h>

class NotifyList;
class WindowList;
class BScrollView;

class AgentDockHeaderString : public BStringView
{
	public:
													 AgentDockHeaderString (BRect, const char *);
		virtual								~AgentDockHeaderString (void);
		
		virtual void					 MouseMoved (BPoint, uint32, const BMessage *);
		virtual void					 MouseDown (BPoint);
		virtual void					 MouseUp (BPoint);
};

class AgentDockHeader : public BView
{
	public:
													 AgentDockHeader (BRect, const char *, uint32);
		virtual								~AgentDockHeader (void);

	private:
		
		AgentDockHeaderString	*fHeaderView;

};

class AgentDockWinList : public BView
{
	public:
													AgentDockWinList (BRect);
		virtual							 ~AgentDockWinList (void);
		
		WindowList						*pWindowList (void) const;

	private:
		WindowList						*fWinList;
		BScrollView					 *fWinListScroll;
		
		AgentDockHeader			 *fAHeader;

};

class AgentDockNotifyList : public BView
{
	public:
													AgentDockNotifyList (BRect);
		virtual							 ~AgentDockNotifyList (void);
		
		NotifyList						*pNotifyList (void) const; 
		
		virtual void					AllAttached(void);

	private:
		
		AgentDockHeader			 *fAHeader;
		NotifyList						*fNotifyList;
		BScrollView					 *fNotifyScroll;

};

class ClientWindowDock : public BView
{
	public:
														ClientWindowDock (BRect);
		virtual								 ~ClientWindowDock (void);
		
		void										AddWinList (void);
		void										AddNotifyList (void);
		WindowList							*pWindowList (void) const;
		NotifyList							*pNotifyList (void) const;
		
		virtual void						AllAttached (void);
		virtual void						MessageReceived (BMessage *);
			
	private:
		
		BRect									 fWorkingFrame;
		
		AgentDockWinList				*fWinListAgent;
		AgentDockNotifyList		 *fNotifyAgent;
		bool										fNotifyExpanded;
};


#endif
