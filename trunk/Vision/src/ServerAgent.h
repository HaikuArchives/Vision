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

#include <Rect.h>
#include <String.h>
#include <List.h>
#include <Locker.h>

#include "ClientAgent.h"

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
                                  const char *,  // id_
                                  BMessage &,
                                  BRect);        // frame
    virtual                     ~ServerAgent (void);


	virtual void				MessageReceived (BMessage *);
	virtual void				AttachedToWindow (void);
    void						PostActive (BMessage *);


	void						Broadcast (BMessage *);
	void						RepliedBroadcast (BMessage *);
	status_t                    NewTimer (const char *, int32, int32);
	
	int                         IRCDType (void);
	
  private:
    virtual void                Init (void);
    void                        DCCChatDialog (BString, BString, BString);
    void                        DCCGetDialog (BString, BString, BString, BString, BString);
    
	void						SendData (const char *);
	void						ParseLine (const char *);
	bool						ParseEvents (const char *);
	bool						ParseENums (const char *, const char *);
	void						ParseCTCP (BString theNick, BString theTarget, BString theMsg);
	void						ParseCTCPResponse (BString theNick, BString theMsg);

    void                        HandleReconnect (void);
	static bool                 PrivateIPCheck (const char *);
	const char                  *GetNextNick (void);
	const ServerData            *GetNextServer (void);
	
 	ClientAgent					*Client (const char *);
	ClientAgent					*ActiveClient (void);

	void						DisplayAll (const char *, const uint32 = 0, const uint32 = 0, const uint32 = 0);
	
	void                        AddResumeData (BMessage *);
	BLocker						*endPointLock,
								loginLock;

	static int32				ServerSeed;

    BMessageRunner              *lagRunner;
	
	BString                     localip;           // our local ip

	bool                        localip_private;    // if localip is private
	                                                // (set by PrivateIPCheck)
	bool                        getLocalIP;
	
	const BString					lname,
									lident;

	int32							nickAttempt;		// going through list
	
	
	// these are used to make Vision more dynamic to various
	// ircd and server configurations	
	int                         ircdtype;
	
	
	
	BString						myNick;
	BString                     reconNick; // used when reconnecting
	BString						quitMsg;
	BString                     myLag;
	
	BList                       resumes;

	bool						isConnected,		// were done connecting
									isConnecting,		// in process
									reconnecting,		// we're reconnecting
									hasWarned,			// warn about quitting
									isQuitting,			// look out, going down
									checkingLag,		// waiting for a lag_check reply
									reacquiredNick,     // disconnected nick has been reacquired
									establishHasLock;  // establish has taken ownership of
									                   // the endPointLock pointer
									                   
	int32						retry,				// what retry # we're on
								retryLimit,			// connect retry limit	
								lagCheck,			// system_time()
								lagCount;			// passes made waiting

    int32                       serverSocket;
	char						send_buffer[4096];		// dynamic buffer for sending

	char							*parse_buffer;		// dynamic buffer for parsing
	size_t						parse_size;			// size of buffer

	thread_id					loginThread;		// thread that receives

	BList							clients;			// agents this server "owns"

	BString						*events;
	
	BString                     serverHostName;
	
	
	
	bool						initialMotd,
									identd;
	BString						cmds;
	int32 s;  // socket
	
	BList                       timers;
    
    static int32				Establish (void *);
    static int32                Timer (void *);
    
    ListAgent                   *pListAgent;
    
    BMessage                    networkData;
    int32                       serverIndex,
                                nickIndex;
};

#endif
