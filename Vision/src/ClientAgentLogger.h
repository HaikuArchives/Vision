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
 */

#ifndef _CLIENTAGENTLOGGER_H_
#define _CLIENTAGENTLOGGER_H_

#include <Locker.h>
#include <String.h>
#include <List.h>
#include <File.h>

class ClientAgentLogger
{
  public:
                       ClientAgentLogger (BString, BString);
                       ~ClientAgentLogger (void);
   void                StartLogging (void);    
   void                Log (const char *);
   void                StopLogging (void);
   bool                isQuitting;
   bool                isLogging;
  
  private:
   void                SetupLogging (void);
   static int32        AsyncLogger (void *);
   thread_id           logThread;
   BString             logName;
   BString             serverName;
   BList               *logBuffer;
   BLocker             *logBufferLock;
   sem_id              logSyncherLock;
   BFile               *logFile;
};

#endif