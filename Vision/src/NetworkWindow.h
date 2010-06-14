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
 */

#ifndef _NETWORKWINDOW_H
#define _NETWORKWINDOW_H

#include <Window.h>

class BView;
class NetPrefsServerView;
class BHandler;

class NetworkWindow : public BWindow
{
	public:
															NetworkWindow ();
		virtual									 ~NetworkWindow (void);
		virtual bool							QuitRequested (void);
};

class NetPrefServerWindow : public BWindow
{
	public:
															NetPrefServerWindow (BHandler *);
		virtual									 ~NetPrefServerWindow (void);
		void											SetNetworkData (BMessage *);								
		virtual bool							QuitRequested (void);
	 
	private:
	
		NetPrefsServerView				*serverView;
};

#endif
