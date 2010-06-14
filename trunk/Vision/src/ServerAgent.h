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

#ifndef _SERVERAGENT_H_
#define _SERVERAGENT_H_

#include <map>

#include <Rect.h>
#include <String.h>
#include <List.h>
#include <Locker.h>

#include "ClientAgent.h"
#include "ObjectList.h"
#include "VisionBase.h"

class NotifyListItem;
class ClientAgentLogger;
class BMessageRunner;
class ListAgent;
struct ServerData;

struct ResumeData
{
	bigtime_t					expire;
	off_t							pos;
	BString						nick,
									file,
									size,
									ip,
									port,
									path;
};


class ServerAgent : public ClientAgent
{
	public:

																ServerAgent (
																	const char *,	// id_
																	BMessage &,
																	BRect);				// frame
		virtual										 ~ServerAgent (void);


		virtual void								MessageReceived (BMessage *);
		virtual void								AttachedToWindow (void);
		virtual void								AllAttached (void);
		virtual void								AddMenuItems (BPopUpMenu *);
		void												PostActive (BMessage *);
		void												Broadcast (BMessage *, bool = false);
		void												RepliedBroadcast (BMessage *);
		status_t										NewTimer (const char *, int32, int32);
	
		int												 IRCDType (void);
	
	private:
		virtual void								Init (void);
		void												DCCChatDialog (BString, BString, BString);
		void												DCCGetDialog (BString, BString, BString, BString, BString);
		void												SendData (const char *);
		void												ParseLine (const char *);
		bool												ParseEvents (const char *);
		bool												ParseENums (const char *, const char *);
		void												ParseCTCP (BString, BString, BString);
		void												ParseCTCPResponse (BString, BString);

		static int									SortNotifyItems (const NotifyListItem *, const NotifyListItem *);

		void												HandleReconnect (void);
		static bool								 PrivateIPCheck (const char *);
		const char									*GetNextNick (void);
		const ServerData						*GetNextServer (void);
	
		ClientAgent								 *Client (const char *);
		ClientAgent								 *ActiveClient (void);

		void												DisplayAll (const char *, const uint32 = 0, const uint32 = 0, const uint32 = 0);
	
		void												AddResumeData (BMessage *);
		
		void												RemoveAutoexecChan (const BString &);
		void												ParseAutoexecChans (const BString &);
		
		void												SendConnectionCreate(bigtime_t timeout = 0LL);

		static int32								fServerSeed;

		BMessageRunner							*fLagRunner;
	
	BString										 fLocalip,					 // our local ip
																fMyNick,
																fReconNick, // used when reconnecting
																fQuitMsg,
																fMyLag;
	
	
		const BString							 fLname,
															 fLident;

	// these are used to make Vision more dynamic to various
	// ircd and server configurations	
		int												 fIrcdtype;
	
	BList											 fResumes;
	
	int32											 fConnectionID;
	
		bool												fLocalip_private,		// if localip is private
																									// (set by PrivateIPCheck)
																	fGetLocalIP,
																	fIsConnecting,		// in process
																	fReconnecting,		// we're reconnecting
																	fConnected,
																	fHasWarned,			// warn about quitting
																	fIsQuitting,			// look out, going down
																	fCheckingLag,		// waiting for a lag_check reply
																	fReacquiredNick;		 // disconnected nick has been reacquired
																		 
		int32											 fRetry,				// what retry # we're on
																fRetryLimit,			// connect retry limit	
																fLagCheck,			// system_time()
																fLagCount,			// passes made waiting
																fNickAttempt;

		BObjectList<ClientAgent>		fClients;			// agents this server "owns"
		
		BString										 *fEvents;
		BString										 fServerHostName;

		bool												fInitialMotd,
																	fIdentd;
		BString										 fCmds;
	
		BObjectList<BString>				fStartupChannels,
																*fPendingSends;
		BObjectList<NotifyListItem> fNotifyNicks;
//																fIgnoreNicks;
		
		static int32								Timer (void *);
		
		ListAgent									 *fListAgent;
		
		BMessage										fNetworkData;
		int32											 fServerIndex,
																	fNickIndex;
		ClientAgentLogger					 *fLogger;
		
		ServerData									fCurrentServer;
		std::map<BString, BString>		fRemoteAwayMessages;
		BString										 fPartialBuffer;
};

#endif
