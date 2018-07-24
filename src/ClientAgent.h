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

#ifndef _CLIENTAGENT_H_
#define _CLIENTAGENT_H_

#include <View.h>
#include <Messenger.h>

#include "Agent.h"

class BScrollView;
class BTextControl;

class BMenu;
class BPopUpMenu;
class ClientAgentInputFilter;
class RunView;
class Theme;
class HistoryList;
class AgentSettings;
class WindowListItem;

class ClientAgent : public BView, public Agent
{
public:
	// used by ServerAgent
	ClientAgent(const char*,	   // id_  (window name)
				const char*,	   // serverName_
				const char*);	   // myNick_

	ClientAgent(const char*,	   // id_  (window name)
				const char*,	   // serverName_
				const char*,	   // myNick_
				const BMessenger&);// sMsgr pointer

	virtual ~ClientAgent();
	// View methods
	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow();
	virtual void AllAttached();
	virtual void DetachedFromWindow();
	virtual void Show();
	// Agent methods
	virtual BView* View();
	virtual void ActivateInputBox();

	virtual void AddMenuItems(BPopUpMenu*) = 0;

	float ScrollPos() const;
	void SetScrollPos(float);
	void ScrollRange(float*, float*) const;
	void SetServerName(const char*);
	void SetEditStates(BMenu*, bool);

	bool ParseCmd(const char*);
	virtual void TabExpansion();
	static int32 DNSLookup(void*);
	static int32 ExecPipe(void*);

	virtual void DroppedFile(BMessage*);

	const BString& Id() const;

	BMessenger fMsgr, fSMsgr;

	virtual void ChannelMessage(const char*, const char* = 0, const char* = 0, const char* = 0);

	static void PackDisplay(BMessage*, const char*, const uint32 = 0, const uint32 = 0,
							const uint32 = 0);

	virtual void ActionMessage(const char*, const char*);

	void CTCPAction(BString theTarget, BString theMsg);
	bool CancelMultilineTextPaste() const { return fCancelMLPaste; }

private:
	void Init();

	bool fCancelMLPaste;

protected:
	HistoryList* fHistory;
	RunView* fText;
	BScrollView* fTextScroll;
	BTextControl* fInput;
	Theme* fActiveTheme;

	static const char* endl;

	friend class ClientAgentInputFilter;
	friend class ServerAgent;

	virtual void Display(const char*, uint32 = 0, uint32 = 0, uint32 = 0);

	void UpdateStatus(int32);

	void ParsemIRCColors(const char*, uint32 = 0, uint32 = 0, uint32 = 0);

	static BString FilterCrap(const char*, bool = false);

	virtual void Submit(const char*, bool = true, bool = true);

	static int32 TimedSubmit(void*);

	int32 FirstKnownAs(const BString&, BString&, bool*) const;
	int32 FirstSingleKnownAs(const BString&, const BString&) const;

	virtual void Parser(const char*);
	virtual bool SlashParser(const char*);

	void AddSend(BMessage*, const char*) const;
	void AddSend(BMessage*, const BString&) const;
	void AddSend(BMessage*, int32) const;

	BString fId;
	BString fServerName;
	BString fMyNick, fMyLag;

	bool fTimeStampState, fCanNotify, fScrolling, fIsLogging;

	friend class WindowList;
};

// constants for multiline paste handler
enum { PASTE_CANCEL = 0, PASTE_MULTI = 1, PASTE_SINGLE = 2, PASTE_MULTI_NODELAY = 3 };

#endif
