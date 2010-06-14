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

#ifndef _SERVERENTRYVIEW_H
#define _SERVERENTRYVIEW_H

#include <View.h>
#include <Window.h>

#include "VisionBase.h"

class BButton;
class BHandler;
class BMessage;
class VTextControl;
class BMenuField;
class BCheckBox;

class ServerEntryWindow : public BWindow
{
	public:
		ServerEntryWindow (BHandler *, BMessage *, const ServerData*, int32);
		virtual ~ServerEntryWindow (void);
};

class ServerEntryView : public BView
{
	public:
															ServerEntryView (BRect, BHandler *, BMessage *, const ServerData *, int32);
		virtual									 ~ServerEntryView (void);
		virtual void							AttachedToWindow (void);
		virtual void							AllAttached (void);
		virtual void							MessageReceived (BMessage *);
	
	private:
		void											CheckDoneState (void);
		
		BMessage									*invocation;
		BHandler									*target;
		VTextControl							*serverName,
															*port,
															*passwordField;
		BMenuField								*statusField;
		BButton									 *okButton,
															*cancelButton;
		BCheckBox								 *usePassword;
		ServerData								currentServer;
};


#endif
