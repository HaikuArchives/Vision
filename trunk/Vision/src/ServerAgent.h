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
 * Contributor(s): Wade Majors <guru@startrek.com>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */

#ifndef _SERVERAGENT_H_
#define _SERVERAGENT_H_

#ifdef GNOME_BUILD
#  include "gnome/Rect.h"
#  include "gnome/CString.h"
#  include "gnome/List.h"
#  include "gnome/Locker.h"
#elif BEOS_BUILD
#  include <Rect.h>
#  include <String.h>
#  include <List.h>
#  include <Locker.h>
#endif

#include "ClientAgent.h"

class BNetEndpoint;

class ServerAgent : public ClientAgent
{
  public:

                                ServerAgent (
                                  const char *,  // id_
                                  const char *,  // port
                                  bool identd_,  // enable identd?
                                  const char *,  // connect commands
                                  BRect);        // frame
    virtual                     ~ServerAgent (void);


	virtual void				MessageReceived (BMessage *);
	virtual void				AttachedToWindow (void);
    virtual void                Pulse (void);
    void						PostActive (BMessage *);


	void						Broadcast (BMessage *);
	void						RepliedBroadcast (BMessage *);
	status_t                    NewTimer (const char *, int32, int32);
    
  private:
    virtual void                Init (void);
	void						SendData (const char *);
	void						ParseLine (const char *);
	bool						ParseEvents (const char *);
	bool						ParseENums (const char *, const char *);
	void						ParseCTCP (BString theNick, BString theTarget, BString theMsg);
	void						ParseCTCPResponse (BString theNick, BString theMsg);
	
 	ClientAgent					*Client (const char *);
	ClientAgent					*ActiveClient (void);

	void						DisplayAll (const char *, const rgb_color * = 0, const BFont * = 0);
	BString						FilterCrap (const char *);
	
	BLocker						endPointLock;

	static int32				ServerSeed;
	
	const BString				lnick1,
									lnick2,
									lport,		
									lname,
									lident;

	int32							nickAttempt;		// going through list
	
	int                         ircdtype;
	
	BString						myNick;
	BString						quitMsg;
	BString						myLag;

	bool						isConnected,		// were done connecting
									isConnecting,		// in process
									reconnecting,		// we're reconnecting
									hasWarned,			// warn about quitting
									isQuitting,			// look out, going down
									checkingLag;		// waiting for a lag_check reply
	int32						retry,				// what retry # we're on
								retryLimit,			// connect retry limit	
								lagCheck,			// system_time()
								lagCount;			// passes made waiting

    BNetEndpoint                *lEndpoint;

	static BLocker				identLock;

	char							*send_buffer;		// dynamic buffer for sending
	size_t						send_size;			// size of buffer

	char							*parse_buffer;		// dynamic buffer for parsing
	size_t						parse_size;			// size of buffer

	thread_id					loginThread;		// thread that receives
	thread_id					identThread;

	BList							clients;			// agents this server "owns"

	rgb_color					ctcpReqColor,
									ctcpRpyColor,
									whoisColor,
									errorColor,
									quitColor,
									joinColor,
									noticeColor,
									wallColor;
	BString						*events;
	
	BString                     serverHostName;
//	uint32						localAddress;
	
	
	
	bool						initialMotd,
									identd,
									hostnameLookup;
	BString						cmds;
	BString						localAddress,
								localIP;
	uint32						localuIP;
	int32 s; 				// socket
	
	BList                       timers;
    
    static int32				Establish (void *);
    static int32                Timer (void *);

};

const uint32 M_GET_ESTABLISH_DATA			= 'saed'; // used by Establish()
const uint32 M_SERVER_DISCONNECT			= 'sasd'; // we got disconnected
const uint32 M_PARSE_LINE					= 'sapl'; // new data from server
const uint32 M_LAG_CHECK                    = 'salc'; // send lag check to server
const uint32 M_REJOIN_ALL					= 'sara';

#endif
