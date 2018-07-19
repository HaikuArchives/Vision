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

#ifndef _MESSAGEWINDOW_H_
#define _MESSAGEWINDOW_H_

#include <Rect.h>
#include <String.h>

#include "ClientAgent.h"

class MessageAgent : public ClientAgent
{
public:
	MessageAgent(const char*, const char*, const BMessenger&, const char*, const char*,
				 bool = false, bool = false, const char* = "", const char* = "");

	virtual ~MessageAgent();
	virtual void AllAttached();
	virtual void AddMenuItems(BPopUpMenu*);

	virtual void MessageReceived(BMessage*);
	virtual void Parser(const char*);
	virtual void DroppedFile(BMessage*);
	virtual void TabExpansion();
	virtual void ActionMessage(const char*, const char*);

	virtual void ChannelMessage(const char*, const char* = 0, const char* = 0, const char* = 0);

private:
	void Init();
	void DCCServerSetup();
	static status_t DCCIn(void*);
	static status_t DCCOut(void*);

	BString fChatAddy, fChatee, fDIP, fDPort;

	bool fDChat, fDInitiate, fDConnected;

	int fMySocket, fAcceptSocket;

	thread_id fDataThread;
};
#endif
