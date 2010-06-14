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
 */

#ifndef _CLIENTAGENTLOGGER_H_
#define _CLIENTAGENTLOGGER_H_

#include <File.h>
#include <Locker.h>
#include <Path.h>
#include <String.h>

#include "ObjectList.h"

#include <map>

typedef std::map<BString, BFile> filemap;

class ClientAgentLogger
{
	public:
													 ClientAgentLogger (BString);
	 virtual								 ~ClientAgentLogger (void);
	 void										StartLogging (void);
	 void										RegisterLogger (const char *);
	 void										UnregisterLogger (const char *);		
	 void										Log (const char *, const char *);
	 void										StopLogging (void);
	 bool										fIsQuitting;
	 bool										fIsLogging;
	
	private:
	 void										SetupLogging (void);
	 void										CloseSession (BFile &);
	 static int32						AsyncLogger (void *);

	 thread_id							 fLogThread;
	 BString								 fServerName;
	 BPath									 fLogPath;
	 BObjectList<BString>		*fLogBuffer;
	 BLocker								 *fLogBufferLock;
	 sem_id									fLogSyncherLock;
	 bool										fNewLine;
	 filemap								 fLogFiles; 
};

#endif
