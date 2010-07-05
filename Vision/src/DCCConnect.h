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
 * Contributor(s): Rene Gollent
 *								 Wade Majors
 *								 Todd Lair
 */

#ifndef DCCCONNECT_H_
#define DCCCONNECT_H_

#include <File.h>
#include <String.h>
#include <View.h>

class BMessenger;
class BStatusBar;
class BStringView;
class StopButton;
class PauseButton;

class DCCConnect : public BView
{
	public:
									DCCConnect (
										const char *,
										const char *,
										const char *,
										const char *,
										const char *,
										const BMessenger &);
		virtual						 ~DCCConnect (void);

		virtual void				AttachedToWindow (void);
		virtual void				AllAttached (void);
		virtual void				DetachedFromWindow (void);
		virtual void				Draw (BRect);
		virtual void				MessageReceived (BMessage *);
		void						UpdateBar (int, bool);
		void						UpdateStatus (const char *);

	protected:
		virtual void				Stopped (void);
		virtual void				Lock (void);
		virtual void				Unlock (void);

		StopButton					*fStop;
		BMessenger					fCaller;

		BString						fNick,
									fFileName,
									fSize,
									fIp,
									fPort;

		BStatusBar					*fBar;
		BStringView					*fLabel;

		int32						fTotalTransferred;
		float						fFinalRateAverage;
		bool						fIsStopped;
		int32						fSocketID;
		BFile						fFile;
		bigtime_t					fTransferStartTime;
};

class DCCReceive : public DCCConnect
{
	friend class DCCConnect;
	public:
									DCCReceive (
										const char *,
										const char *,
										const char *,
										const char *,
										const char *,
										const BMessenger &,
										bool);
														
		virtual						~DCCReceive (void);
		virtual void				AttachedToWindow (void);
		virtual void				MessageReceived(BMessage *msg);

	protected:
		bool						fResume;
};

class DCCSend : public DCCConnect
{
	friend class DCCConnect;
	public:
									DCCSend (
										const char *,
										const char *,
										const char *,
										const BMessenger &);
														
		virtual						~DCCSend (void);
		virtual void				AttachedToWindow (void);
		virtual void				MessageReceived(BMessage *msg);
		bool						IsMatch (const char *, const char *) const;
		void						SetResume (off_t);

	protected:
		ssize_t						SendNextBlock(void);
		int64						fPos;
		int32						fServerID;
};

#endif
