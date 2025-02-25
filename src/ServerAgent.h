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

#ifndef _SERVERAGENT_H_
#define _SERVERAGENT_H_

#include <map>

#include <List.h>
#include <Locker.h>
#include <Rect.h>
#include <String.h>

#include "ClientAgent.h"
#include "ObjectList.h"
#include "VisionBase.h"

class NotifyListItem;
class ClientAgentLogger;
class BMessageRunner;
class BSocket;
class ListAgent;
struct ServerData;

struct ResumeData {
	bigtime_t expire;
	off_t pos;
	BString nick, file, size, ip, port, path;
};

class ServerAgent : public ClientAgent {
public:
	ServerAgent(const char*,  // id_
		BMessage&);			  // frame
	virtual ~ServerAgent();

	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow();
	virtual void AllAttached();
	virtual void AddMenuItems(BPopUpMenu*);
	void PostActive(BMessage*);
	void Broadcast(BMessage*, bool = false);
	void RepliedBroadcast(BMessage*);
	status_t NewTimer(const char*, int32, int32);

	int IRCDType();

private:
	virtual void Init();
	void DCCChatDialog(BString, BString, BString);
	void DCCGetDialog(BString, BString, BString, BString, BString);
	void SendData(const char*);
	int AsyncSendData(const char*);
	void ParseLine(const char*);
	bool ParseEvents(const char*);
	bool ParseENums(const char*, const char*);
	void ParseCTCP(BString, BString, BString);
	void ParseCTCPResponse(BString, BString);

	static int SortNotifyItems(const NotifyListItem*, const NotifyListItem*);

	void HandleReconnect();
	static bool PrivateIPCheck(BString);
	const char* GetNextNick();
	const ServerData* GetNextServer();

	ClientAgent* Client(const char*);
	ClientAgent* ActiveClient();

	void DisplayAll(const char*, const uint32 = 0, const uint32 = 0, const uint32 = 0);

	void AddResumeData(BMessage*);

	void RemoveAutoexecChan(const BString&);
	void ParseAutoexecChans(const BString&);
	void CreateSocketThread();

	BLocker *fEndPointLock, *fSendLock;

	static int32 fServerSeed;

	BMessageRunner* fLagRunner;

	BString fLocalip,  // our local ip
		fMyNick,
		fReconNick,	 // used when reconnecting
		fQuitMsg, fMyLag;

	const BString fLname, fLident;

	// these are used to make Vision more dynamic to various
	// ircd and server configurations
	int fIrcdtype;

	BList fResumes;

	bool fLocalip_private,	// if localip is private
							// (set by PrivateIPCheck)
		fGetLocalIP,
		fIsConnected,		// were done connecting
		fIsConnecting,		// in process
		fReconnecting,		// we're reconnecting
		fHasWarned,			// warn about quitting
		fIsQuitting,		// look out, going down
		fCheckingLag,		// waiting for a lag_check reply
		fReacquiredNick,	// disconnected nick has been reacquired
		fEstablishHasLock;	// establish has taken ownership of
							// the endPointLock pointer

	int32 fRetry,	  // what retry # we're on
		fRetryLimit,  // connect retry limit
		fLagCheck,	  // system_time()
		fLagCount,	  // passes made waiting
		fNickAttempt;
	BSocket* fSocket;
	char fSend_buffer[2048];  // buffer for sending

	char fParse_buffer[2048];  // buffer for parsing

	volatile thread_id fSocketThread;  // thread that receives

	BObjectList<ClientAgent> fClients;	// agents this server "owns"

	BString* fEvents;
	BString fServerHostName;

	bool fInitialMotd, fIdentd;
	BString fCmds;

	BObjectList<BString> fStartupChannels, *fPendingSends;
	BObjectList<NotifyListItem> fNotifyNicks;
	//                                fIgnoreNicks;

	static status_t SocketThreadEntry(void*);
	status_t SocketThread();
	static int32 Timer(void*);

	ListAgent* fListAgent;

	BMessage fNetworkData;
	int32 fServerIndex, fNickIndex;
	ClientAgentLogger* fLogger;

	sem_id fSendSyncSem;  // synchronization semaphore for data sends
	ServerData fCurrentServer;
	std::map<BString, BString> fRemoteAwayMessages;
};

#endif
