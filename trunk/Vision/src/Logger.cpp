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
 */
 
#include <Path.h>
#include <Entry.h>
#include <Directory.h>
#include <Application.h>
#include <Roster.h>

#include "Logger.h"
#include "Vision.h"

Logger::Logger (BString currentLog, BString server)
{
  logName = currentLog;
  serverName = server;
  SetupLogging();
  logSyncherLock = create_sem (0, "logSynchLock_sem");
  logThread = spawn_thread (AsyncLogger,
                            vision_app->GetThreadName (THREAD_L),
                            B_LOW_PRIORITY,
                            this);
  resume_thread (logThread);
}

Logger::~Logger (void)
{
  release_sem (logSyncherLock);
  thread_id logWait = logThread;
  logThread = -1;
  status_t result;
  wait_for_thread (logWait, &result);
  delete_sem (logSyncherLock);
  if (logFile.InitCheck() != B_NO_INIT)
  {
    time_t myTime (time (0));
    struct tm *ptr;
    ptr = localtime (&myTime);
    char tempTime[96];
    strftime (tempTime, 96, "Session Close: %a %b %d %H:%M %Y\n", ptr);
    off_t len = strlen (tempTime);
    logFile.Write (tempTime, len);
    logFile.Unset();
  }

}

void
Logger::SetupLogging (void)
{
  if (logFile.InitCheck() == B_NO_INIT)
  {
    time_t myTime (time (0));
    struct tm *ptr;
    ptr = localtime (&myTime);

    app_info ai;
    be_app->GetAppInfo (&ai);

    BEntry entry (&ai.ref);
    BPath path;
    entry.GetPath (&path);
    path.GetParent (&path);

    BDirectory dir (path.Path());
    dir.CreateDirectory ("logs", &dir);
    path.Append ("logs");
    dir.SetTo (path.Path());
    BString sName (serverName);
    sName.ToLower();
    dir.CreateDirectory (sName.String(), &dir);
    path.Append (sName.String());

    BString wName (logName);     // do some cleaning up
    wName.ReplaceAll ("/", "_");
    wName.ReplaceAll ("*", "_");
    wName.ReplaceAll ("?", "_");

    if (vision_app->GetBool ("log_filetimestamp"))
    {
      char tempDate[16];
      strftime (tempDate, 16, "_%Y%m%d", ptr);
      wName << tempDate;
    }

    wName << ".log";
    wName.ToLower();

    path.Append (wName.String());
 
    logFile.SetTo (path.Path(), B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);

    if (logFile.InitCheck() == B_OK)
    {
      char tempTime[96];
      if (logFile.Position() == 0) // new file
      {
        strftime (tempTime, 96, "Session Start: %a %b %d %H:%M %Y\n", ptr);
      }
      else
      {
        strftime (tempTime, 96, "\n\nSession Start: %a %b %d %H:%M %Y\n", ptr);
      }
      BString timeString(tempTime);
      logFile.Write (timeString.String(), timeString.Length());
      update_mime_info (path.Path(), false, false, true);
    }
    else
      logFile.Unset();
  }

  return;
}

void
Logger::Log (const char *data)
{
  logBufferLock.Lock();
  BString *logString (new BString (data));
  logBuffer.AddItem (logString);
  logBufferLock.Unlock();
  release_sem (logSyncherLock);
}

int32
Logger::AsyncLogger (void *arg)
{
  Logger *logger ((Logger *)arg);
  BString *currentString (NULL);
  BLocker *myLogBufferLock (&(logger->logBufferLock));
  BList *myLogBuffer (&(logger->logBuffer));
  BFile *myLogFile (&(logger->logFile));
  
  thread_id loggerThread (find_thread (NULL));

  while (acquire_sem (logger->logSyncherLock) == B_NO_ERROR)
  {
    if (loggerThread != logger->logThread)
      break;
  
    myLogBufferLock->Lock();
    if (!myLogBuffer->IsEmpty())
    {
      currentString = (BString *)myLogBuffer->RemoveItem (0L);
      myLogBufferLock->Unlock();
      if (myLogFile->InitCheck() != B_NO_INIT)
        myLogFile->Write (currentString->String(), currentString->Length());
      delete currentString;
    }
    else myLogBufferLock->Unlock();
  }

  while (!myLogBuffer->IsEmpty())
  {
    currentString = (BString *)myLogBuffer->RemoveItem(0L);
    if (myLogFile->InitCheck() != B_NO_INIT)
      myLogFile->Write (currentString->String(), currentString->Length());
    delete currentString;
  }

  return 0;
}

