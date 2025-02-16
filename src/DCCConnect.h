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
 * Contributor(s): Rene Gollent
 *                 Wade Majors
 *                 Todd Lair
 */

#ifndef DCCCONNECT_H_
#define DCCCONNECT_H_

#include <File.h>
#include <Messenger.h>
#include <String.h>
#include <View.h>

#include <arpa/inet.h>
#include <sys/socket.h>

class BStatusBar;
class BStringView;
class StopButton;
class PauseButton;

class DCCConnect : public BView {
public:
	DCCConnect(const char*, const char*, const char*, const char*, const char*, const BMessenger&);
	virtual ~DCCConnect();

	virtual void AttachedToWindow();
	virtual void AllAttached();
	virtual void DetachedFromWindow();
	virtual void Draw(BRect);
	virtual void MessageReceived(BMessage*);
	static void UpdateBar(const BMessenger&, int, int, uint32, bool);
	static void UpdateStatus(const BMessenger&, const char*);

protected:
	virtual void Stopped();
	virtual void Lock();
	virtual void Unlock();

	StopButton* fStop;
	BMessenger fCaller;

	BString fNick, fFileName, fSize, fIp, fPort;

	BStatusBar* fBar;
	BStringView* fLabel;

	int32 fTotalTransferred;
	int32 fFinalRateAverage;

	thread_id fTid;
	bool fIsStopped;
};

class DCCReceive : public DCCConnect {
	friend class DCCConnect;

public:
	DCCReceive(
		const char*, const char*, const char*, const char*, const char*, const BMessenger&, bool);

	virtual ~DCCReceive();
	virtual void AttachedToWindow();
	static int32 Transfer(void*);

protected:
	bool fResume;
};

class DCCSend : public DCCConnect {
	friend class DCCConnect;

public:
	DCCSend(const char*, const char*, const char*, const BMessenger&);

	virtual ~DCCSend();
	virtual void AttachedToWindow();
	static int32 Transfer(void*);
	bool IsMatch(const char*, const char*) const;
	void SetResume(off_t);

protected:
	int64 fPos;
};

#endif
