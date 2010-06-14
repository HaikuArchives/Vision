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

#ifndef _CLIENTAGENT_H_
#define _CLIENTAGENT_H_

#include <View.h>
#include <Messenger.h>

class BScrollView;
class VTextControl;

class BMenu;
class BPopUpMenu;
class ClientAgentInputFilter;
class RunView;
class Theme;
class HistoryList;
class AgentSettings;
class WindowListItem;

class ClientAgent : public BView
{
	public:
																// used by ServerAgent
																ClientAgent (
																	const char *,				 // id_	(window name)
																	const char *,				 // serverName_
																	const char *,				 // myNick_
																	BRect);								// frame
																ClientAgent (
																	const char *,				 // id_	(window name)
																	const char *,				 // serverName_
																	const char *,				 // myNick_
																	const BMessenger &,	 // sMsgr pointer
																	BRect);								// frame
																	
		virtual										 ~ClientAgent (void);

		virtual void								MessageReceived (BMessage *);
		virtual void								AttachedToWindow (void);
		virtual void								AllAttached (void);
		virtual void								DetachedFromWindow (void);
		virtual void								Show (void);

		virtual void								AddMenuItems (BPopUpMenu *) = 0;

		float											 ScrollPos(void) const;
		void												SetScrollPos(float);
		void												ScrollRange(float *, float *) const;
		void												SetServerName(const char *);
		void												SetEditStates(BMenu *, bool);
		
		bool												ParseCmd (const char *);
		virtual void								TabExpansion (void);
		static int32								DNSLookup (void *);
		static int32								ExecPipe (void *);

		virtual void								DroppedFile (BMessage *);
		
		const BString							 &Id (void) const;
		
		BMessenger									fMsgr,
																fSMsgr;
		
		virtual void								ChannelMessage (
																	const char *,
																	const char * = 0,
																	const char * = 0,
																	const char * = 0);

		static void								 PackDisplay (BMessage *,
																	const char *,
																	const int16 = 0,
																	const int16 = 0,
																	const int16 = 0);

		
		virtual void								ActionMessage (
																	const char *,
																	const char *);

		void												CTCPAction (BString theTarget, BString theMsg);
	bool						CancelMultilineTextPaste() const {	return fCancelMLPaste; }

		WindowListItem								 *fAgentWinItem;

		virtual void										Display (
																			const char *,
																			int16 = 0,
																			int16 = 0,
																			int16 = 0);
		
		
	private:
		void												Init (void);

		bool						fCancelMLPaste;
														
	protected:
		HistoryList								 *fHistory;
		RunView										 *fText;
		BScrollView								 *fTextScroll;
		VTextControl								*fInput;
		Theme											 *fActiveTheme;

		static const char							 *endl;


		friend class										ClientAgentInputFilter;
		friend class										ServerAgent;
 
																			
		void														UpdateStatus (int32);

		void														ParsemIRCColors (
																			const char *,
																			int16 = 0,
																			int16 = 0,
																			int16 = 0);

	 	static BString									FilterCrap (const char *, bool = false);

																			
		virtual void										Submit (const char *, bool = true, bool = true);

		static int32										TimedSubmit (void *);

		int32													 FirstKnownAs (
																			const BString &,
																			BString &,
																			bool *) const;
		int32													 FirstSingleKnownAs (
																			const BString &,
																			const BString &) const;

		virtual void										Parser (const char *);
		virtual bool										SlashParser (const char *);
		
		void														AddSend (BMessage *, const char *) const;
		void														AddSend (BMessage *, const BString &) const;
		void														AddSend (BMessage *, int32) const;

		BString												 fId;
		BString												 fServerName;
		BString												 fMyNick,
																			fMyLag;

		bool														 fTimeStampState,
																			 fCanNotify,
																			 fScrolling,
																			 fIsLogging;
																			 
		BRect														fFrame;
		friend class										 WindowList;

};

// constants for multiline paste handler
enum {
	PASTE_CANCEL = 0,
	PASTE_MULTI = 1,
	PASTE_SINGLE = 2,
	PASTE_MULTI_NODELAY = 3
};

#endif
