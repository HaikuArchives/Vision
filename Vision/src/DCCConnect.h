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

#include <View.h>
#include <String.h>
#include <File.h>

#ifdef BONE_BUILD
#include <sys/socket.h>
#include <arpa/inet.h>
#elif NETSERVER_BUILD
#include <socket.h>
#include <netdb.h>
#endif

class BMessenger;
class BStatusBar;
class BStringView;
class StopButton;
class PauseButton;

class DCCConnect : public BView
{
	StopButton			*stop;

	protected:

    BMessenger			caller;

	BString				nick,
							file_name,
							size,
							ip,
							port;
		
	BStatusBar			*bar;
	BStringView			*label;

	int32				totalTransferred;
	int32				finalRateAverage;

	thread_id			tid;
	bool					isStopped;

	virtual void		Stopped (void);
	virtual void		Lock (void);
	virtual void		Unlock (void);

	public:
							DCCConnect (
								const char *,
								const char *,
								const char *,
								const char *,
								const char *,
								const BMessenger &);
	virtual				~DCCConnect (void);

	virtual void		AttachedToWindow (void);
	virtual void		AllAttached (void);
	virtual void		DetachedFromWindow (void);
	virtual void		Draw (BRect);
	virtual void		MessageReceived (BMessage *);
	static void			UpdateBar (const BMessenger &, int, int, uint32, bool);
	static void			UpdateStatus (const BMessenger &, const char *);

};

class DCCReceive : public DCCConnect
{
	friend DCCConnect;
	protected:
	bool					resume;

	public:
							DCCReceive (
								const char *,
								const char *,
								const char *,
								const char *,
								const char *,
								const BMessenger &,
								bool);
	virtual				~DCCReceive (void);
	virtual void		AttachedToWindow (void);
	static int32		Transfer (void *);
};

class DCCSend : public DCCConnect
{
	friend DCCConnect;
	protected:
	int64				pos;

	public:
							DCCSend (
								const char *,
								const char *,
								const char *,
								const BMessenger &);
	virtual				~DCCSend (void);
	virtual void		AttachedToWindow (void);
	static int32		Transfer (void *);
	bool					IsMatch (const char *, const char *) const;
	void					SetResume (off_t);
	
};

uint32 const M_DCC_FINISH					= 'fnsh';

#endif
