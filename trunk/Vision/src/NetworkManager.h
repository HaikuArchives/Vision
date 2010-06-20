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
 */
 
#ifndef _NETWORK_MANAGER_H_
#define _NETWORK_MANAGER_H_

#include <Locker.h>
#include <Looper.h>
#include <Messenger.h>

#include <poll.h>

#include <map>
#include <set>
#include <vector>


class NetworkData;

class NetworkManager : public BLooper
{
	public:
										NetworkManager(void);
		virtual							~NetworkManager(void);
		
		virtual void					MessageReceived(BMessage *);
		virtual bool					QuitRequested(void);
		
	private:
		// thread functions
		static int32					Overlord(void *);
		static int32					ConnectionHandler(void *);

		void							_SocketLock(void);
		void							_SocketUnlock(void);
		
		void							_HandleBind(const BMessage *data);
		void							_HandleAccept(int sock, uint32 index);
		void							_HandleConnect(const BMessage *data);
		void							_HandleSend(const BMessage *data);
		void							_HandleReceive(int sock, uint32 index);
		void							_HandleDisconnect(int sock, uint32 index);
		void							_CleanupSocket(int sock, uint32 index);
		int32							_IndexForSocket(int sock);
		
		std::map<int, BMessenger>		fSockets;
		bool							fShuttingDown;
		BLocker							fSocketLock;
		struct pollfd					fPollFDs[256];
		thread_id						fPollThread;
		std::set<int>					fListeners;
		
};

#endif // _NETWORK_MANAGER_H_
