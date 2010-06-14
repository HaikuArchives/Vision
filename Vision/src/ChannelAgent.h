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
 
#ifndef _CHANNELAGENT_H_
#define _CHANNELAGENT_H_

#include <Rect.h>
#include <String.h>
#include "ClientAgent.h"
#include "ObjectList.h"

class ChannelOptions;
class BScrollView;
class ServerWindow;
class NamesView;
class ResizeView;

const int32 MAX_RECENT_NICKS = 5;

class ChannelAgent : public ClientAgent
{
	public:

														ChannelAgent (
															const char *,			 // id 
															const char *,			 // serverName
															int,								// ircdtype
															const char *,			 // nick
															BMessenger &, // sMsgr (ServerAgent pointer)
															BRect &);						 // frame
		virtual								 ~ChannelAgent (void);

		virtual void						AttachedToWindow (void);
		virtual void						MessageReceived (BMessage *);
		virtual void						Parser (const char *);
		virtual void						TabExpansion (void);
		virtual void						ChannelMessage (
																	const char *,
																	const char * = 0,
																	const char * = 0,
																	const char * = 0);
		virtual void						AddMenuItems (BPopUpMenu *);
		virtual void						Show (void);
		
		void										AddUser (const char *, const int32);
		bool										RemoveUser (const char *);
		int										 FindPosition (const char *) const;
		void										UpdateMode (char, char);
		void										ModeEvent (BMessage *);
		
		static int							AlphaSortNames (const BString *, const BString *);
		static int							SortNames (const void *, const void *);
		
		const NamesView				 *pNamesList() const;

		
	private:
		void										Init();
		void										RemoveNickFromList (BObjectList<BString> &, const char *);
		
		BString								 fChanMode,
															fChanLimit,
															fChanLimitOld,
															fChanKey,
															fChanKeyOld,
															fLastExpansion,
															fTopic;
		
		BObjectList<BString>		fRecentNicks,
															fCompletionNicks;

		int32									 fUserCount,
														fOpsCount;
														
		int										 fIrcdtype;

		NamesView							 *fNamesList;
		BScrollView						 *fNamesScroll;
		ChannelOptions					*fChanOpt;
		ResizeView							*fResize;

};



#endif
