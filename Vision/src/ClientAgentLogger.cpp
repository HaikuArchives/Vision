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
 
#include <Path.h>
#include <Entry.h>
#include <Directory.h>
#include <Application.h>
#include <Roster.h>

#include "ClientAgentLogger.h"
#include "StringManip.h"
#include "Vision.h"

ClientAgentLogger::ClientAgentLogger (BString currentLog, BString server)
{
  logName = currentLog;
  serverName = server;
  isQuitting = false;
  isLogging = false;
  logThread = -1;
  StartLogging();
  newLine = false;
}

ClientAgentLogger::~ClientAgentLogger (void)
{
  // tell log thread it's time to die
  StopLogging();
  status_t result;
  // if the whole app's shutting down, wait for the logger to finish its work to
  // prevent possible race/crash conditions, otherwise terminate immediately and let
  // logThread finish up in the background
  if (isQuitting)
    wait_for_thread (logThread, &result);
}

void
ClientAgentLogger::StartLogging (void)
{
  // someone tried to call StartLogging while a logger was already active, do nothing.
  if (logThread != -1)
     return;

  // the file, BList and Locker are taken over by logThread as soon as SetupLogging
  // is done. From there it will take care of cleaning them up on destruct.
  logFile = new BFile();
  logBuffer = new BList();
  logBufferLock = new BLocker();
  isLogging = true;
  
  // semaphore used to synchronize the log thread and newly incoming log requests
  logSyncherLock = create_sem (0, "logSynchLock_sem");
  
  logThread = spawn_thread (AsyncLogger,
                            vision_app->GetThreadName (THREAD_L),
                            B_LOW_PRIORITY,
                            this);
  resume_thread (logThread);
  
}

void
ClientAgentLogger::StopLogging (void)
{
  isLogging = false;
  delete_sem(logSyncherLock);
  logThread = -1;
}

void
ClientAgentLogger::SetupLogging (void)
{
  // if the logFile doesn't exist already create the dirs
  // and initialize the file, otherwise just open the existing one
  // and add a starting session line
  if (logFile->InitCheck() == B_NO_INIT)
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
    
    // append timestamp in year/month/day format to filename if desired
    if (vision_app->GetBool ("log_filetimestamp"))
    {
      char tempDate[16];
      strftime (tempDate, 16, "_%Y%m%d", ptr);
      wName << tempDate;
    }

    wName << ".log";
    wName.ToLower();

    path.Append (wName.String());
 
    logFile->SetTo (path.Path(), B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);

    if (logFile->InitCheck() == B_OK)
    {
      char tempTime[96];
      if (logFile->Position() == 0) // new file
      {
        strftime (tempTime, 96, "Session Start: %a %b %d %H:%M %Y\n", ptr);
      }
      else
      {
        strftime (tempTime, 96, "\n\nSession Start: %a %b %d %H:%M %Y\n", ptr);
      }
      BString timeString(tempTime);
      logFile->Write (timeString.String(), timeString.Length());
      update_mime_info (path.Path(), false, false, true);
    }
    else
      logFile->Unset();
  }
  return;
}

void
ClientAgentLogger::Log (const char *data)
{
  if (!isLogging)
    return;
  
  // add entry to logfile for logThread to write asynchronously
  logBufferLock->Lock();
  BString *logString (new BString (data));
  if (newLine)
    logString->Prepend (TimeStamp().String());
  
  newLine = (logString->IFindFirst("\n") == B_ERROR) ? false : true;
  
  logBuffer->AddItem (logString);
  logBufferLock->Unlock();
  // unlock logThread so it can go to work
  release_sem (logSyncherLock);
}

int32
ClientAgentLogger::AsyncLogger (void *arg)
{
  ClientAgentLogger *logger ((ClientAgentLogger *)arg);

  BString *currentString (NULL);
  BLocker *myLogBufferLock ((logger->logBufferLock));
  BList *myLogBuffer ((logger->logBuffer));
  BFile *myLogFile ((logger->logFile));  

  // initialize the log file if it doesn't already exist
  logger->SetupLogging();

  // sit in event loop waiting for new data
  // loop will break when ~Logger deletes the semaphore
  while (acquire_sem (logger->logSyncherLock) == B_NO_ERROR)
  {
    myLogBufferLock->Lock();
    if (!myLogBuffer->IsEmpty())
    {
      // grab next string from list and write to file
      currentString = (BString *)myLogBuffer->RemoveItem (0L);
      myLogBufferLock->Unlock();
      if (myLogFile->InitCheck() != B_NO_INIT)
        myLogFile->Write (currentString->String(), currentString->Length());
      delete currentString;
    }
    else myLogBufferLock->Unlock();
  }

  // on shutdown empty out all remaining data (if any) and write to file
  while (!myLogBuffer->IsEmpty())
  {
    currentString = (BString *)myLogBuffer->RemoveItem(0L);
    if (myLogFile->InitCheck() != B_NO_INIT)
      myLogFile->Write (currentString->String(), currentString->Length());
    delete currentString;
  }
  
  // clean up the now unneeded BList and Locker
  delete myLogBuffer;
  delete myLogBufferLock;
    
  // write Session Close and close/clean up
  if (myLogFile->InitCheck() != B_NO_INIT)
  {
    time_t myTime (time (0));
    struct tm *ptr;
    ptr = localtime (&myTime);
    char tempTime[96];
    strftime (tempTime, 96, "Session Close: %a %b %d %H:%M %Y\n", ptr);
    off_t len = strlen (tempTime);
    myLogFile->Write (tempTime, len);
    myLogFile->Unset();
    delete myLogFile;
  }

  return 0;
}

