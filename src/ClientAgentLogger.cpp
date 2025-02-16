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
 *                 Alan Ellis <alan@cgsoftware.org>
 */

#include <Application.h>
#include <Directory.h>
#include <Entry.h>
#include <FindDirectory.h>
#include <Mime.h>
#include <Path.h>
#include <Roster.h>

#include "ClientAgentLogger.h"
#include "Utilities.h"
#include "Vision.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ClientAgentLogger"

ClientAgentLogger::ClientAgentLogger(BString server)
{
	fServerName = server;
	fIsQuitting = false;
	fIsLogging = false;
	fLogThread = -1;
	fNewLine = false;
	fLogSetupDone = false;
	StartLogging();
}

ClientAgentLogger::~ClientAgentLogger()
{
	// tell log thread it's time to die
	StopLogging();
	status_t result;
	// if the whole app's shutting down, wait for the logger to finish its work to
	// prevent possible race/crash conditions, otherwise terminate immediately and let
	// fLogThread finish up in the background
	if (fIsQuitting)
		wait_for_thread(fLogThread, &result);

	for (filemap::iterator it = fLogFiles.begin(); it != fLogFiles.end(); ++it)
		CloseSession(it->second);
}

void
ClientAgentLogger::StartLogging()
{
	// someone tried to call StartLogging while a logger was already active, do nothing.
	if (fLogThread != -1)
		return;

	// the file, BList and Locker are taken over by fLogThread as soon as SetupLogging
	// is done. From there it will take care of cleaning them up on destruct.
	fLogBuffer = new BObjectList<BString>();
	fLogBufferLock = new BLocker();
	fIsLogging = true;

	// semaphore used to synchronize the log thread and newly incoming log requests
	fLogSyncherLock = create_sem(0, "logSynchLock_sem");

	BString name;
	vision_app->GetThreadName(THREAD_L, name);

	fLogThread = spawn_thread(AsyncLogger, name.String(), B_LOW_PRIORITY, this);
	if (fLogThread >= B_OK)
		resume_thread(fLogThread);
}

void
ClientAgentLogger::StopLogging()
{
	fIsLogging = false;
	delete_sem(fLogSyncherLock);
	fLogThread = -1;
}

void
ClientAgentLogger::RegisterLogger(const char* logName)
{
	// if it doesn't already exist
	// initialize the file, otherwise just open the existing one
	// and add a starting session line

	time_t myTime(time(0));
	struct tm ptr;
	localtime_r(&myTime, &ptr);

	BString wName(logName);	 // do some cleaning up
	wName.ReplaceAll("/", "_");
	wName.ReplaceAll("*", "_");
	wName.ReplaceAll("?", "_");

	// append timestamp in year/month/day format to filename if desired
	if (vision_app->GetBool("log_filetimestamp")) {
		char tempDate[16];
		strftime(tempDate, 16, "_%Y%m%d", &ptr);
		wName << tempDate;
	}

	wName << ".log";
	wName.ToLower();

	BPath filePath(fLogPath);
	filePath.Append(wName.String());

	if (filePath.InitCheck() != B_OK)
		return;

	BFile logFile(filePath.Path(), B_READ_WRITE | B_CREATE_FILE | B_OPEN_AT_END);

	if (logFile.InitCheck() == B_OK) {
		char tempTime[96];
		if (logFile.Position() == 0)  // new file
		{
			strftime(tempTime, 96, B_TRANSLATE("Session start: %a %b %d %H:%M %Y\n"), &ptr);
		} else {
			strftime(tempTime, 96, B_TRANSLATE("\n\nSession start: %a %b %d %H:%M %Y\n"), &ptr);
		}

		BString timeString(tempTime);
		logFile.Write(timeString.String(), timeString.Length());
		update_mime_info(filePath.Path(), false, false, true);
		BString filename(logName);
		fLogFiles[filename] = logFile;
	} else
		logFile.Unset();
}

void
ClientAgentLogger::UnregisterLogger(const char* name)
{
	BFile& logFile(fLogFiles[name]);
	if (logFile.InitCheck() == B_OK) {
		if (fIsLogging) {
			fLogBufferLock->Lock();
			for (int32 i = 0; i < fLogBuffer->CountItems();) {
				BString* currentLog(NULL);

				// read through buffer and find all strings that belonged to this file,
				// then write and remove them

				if (((currentLog = fLogBuffer->ItemAt(i)) != NULL)
					&& (currentLog->ICompare(name) == 0)) {
					delete fLogBuffer->RemoveItemAt(i);
					currentLog = fLogBuffer->RemoveItemAt(i);
					logFile.Write(currentLog->String(), currentLog->Length());
					delete currentLog;
				} else
					i += 2;
			}
			fLogBufferLock->Unlock();
		}

		CloseSession(logFile);
		BString logName(name);
		fLogFiles.erase(logName);
	}
}

void
ClientAgentLogger::CloseSession(BFile& logFile)
{
	// write Session Close and close/clean up
	if (logFile.InitCheck() != B_NO_INIT) {
		time_t myTime(time(0));
		struct tm ptr;
		localtime_r(&myTime, &ptr);
		char tempTime[96];
		strftime(tempTime, 96, B_TRANSLATE("Session close: %a %b %d %H:%M %Y\n"), &ptr);
		size_t len = strlen(tempTime);
		logFile.Write(tempTime, len);
		logFile.Unset();
	}
}

void
ClientAgentLogger::SetupLogging()
{
	if (fLogSetupDone)
		return;

	if (!vision_app->GetBool("log_enabled"))
		return;

	// create the dirs if they don't already exist
	BString visLogPath(vision_app->GetString("logBaseDir"));
	if (visLogPath.Length() == 0)
		visLogPath = "logs";
	else if (visLogPath[0] == '/')
		fLogPath.SetTo(visLogPath.String());
	else {
		if (find_directory(B_USER_SETTINGS_DIRECTORY, &fLogPath) == B_OK)
			fLogPath.Append("Vision");
		else
			return;
		fLogPath.Append(visLogPath.String());
	}

	if (create_directory(fLogPath.Path(), 0777) != B_OK)
		return;

	BDirectory dir(fLogPath.Path());
	if (dir.InitCheck() == B_OK) {
		BString sName(fServerName);
		sName.ToLower();
		dir.CreateDirectory(sName.String(), &dir);
		fLogPath.Append(sName.String());
		fLogSetupDone = true;
	}
}

void
ClientAgentLogger::Log(const char* loggername, const char* data)
{
	if (!fIsLogging)
		return;

	if ((loggername == NULL) || (data == NULL))
		return;

	// depending on log state at the time the logger was initialized,
	// the file may not yet exist. Attempt to create it if so.
	if (fLogFiles.find(loggername) == fLogFiles.end()) {
		RegisterLogger(loggername);
	}

	// add entry to logfile for fLogThread to write asynchronously
	fLogBufferLock->Lock();
	BString* pathString(new BString(loggername));
	BString* logString(new BString(data));

	if (fNewLine)
		logString->Prepend(TimeStamp().String());

	fNewLine = (logString->IFindFirst("\n") == B_ERROR) ? false : true;

	fLogBuffer->AddItem(pathString);
	fLogBuffer->AddItem(logString);
	fLogBufferLock->Unlock();
	// unlock fLogThread so it can go to work
	release_sem(fLogSyncherLock);
}

int32
ClientAgentLogger::AsyncLogger(void* arg)
{
	ClientAgentLogger* logger((ClientAgentLogger*)arg);

	BString* currentLogger(NULL);
	BString* currentString(NULL);
	BLocker* myLogBufferLock((logger->fLogBufferLock));
	BObjectList<BString>* myLogBuffer((logger->fLogBuffer));
	BFile* myLogFile;

	// sit in event loop waiting for new data
	// loop will break when ~Logger deletes the semaphore
	while (acquire_sem(logger->fLogSyncherLock) == B_NO_ERROR) {
		myLogBufferLock->Lock();
		if (!myLogBuffer->IsEmpty()) {
			if (!logger->fLogSetupDone)
				logger->SetupLogging();
			// grab next string from list and write to file
			currentLogger = myLogBuffer->RemoveItemAt(0L);
			currentString = myLogBuffer->RemoveItemAt(0L);
			myLogBufferLock->Unlock();
			myLogFile = &logger->fLogFiles[*currentLogger];
			if (myLogFile->InitCheck() != B_NO_INIT)
				myLogFile->Write(currentString->String(), currentString->Length());
			delete currentLogger;
			delete currentString;
		} else
			myLogBufferLock->Unlock();
	}

	// on shutdown empty out all remaining data (if any) and write to file
	while (!myLogBuffer->IsEmpty()) {
		currentLogger = (BString*)(myLogBuffer->RemoveItemAt(0L));
		currentString = (BString*)(myLogBuffer->RemoveItemAt(0L));
		myLogFile = &logger->fLogFiles[*currentLogger];
		if (myLogFile->InitCheck() != B_NO_INIT)
			myLogFile->Write(currentString->String(), currentString->Length());
		delete currentLogger;
		delete currentString;
	}

	// clean up the now unneeded BList and Locker
	delete myLogBuffer;
	delete myLogBufferLock;

	return 0;
}
