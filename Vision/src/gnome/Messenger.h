// BMessenger-like class
//
// see License at the end of file

#ifndef MESSENGER_H__
#define MESSENGER_H__

#include <PlatformDefines.h>

class BHandler;
class BLooper;
class BMessage;

class FMessenger {
public:	
	FMessenger();
	FMessenger(const FMessenger &);
	FMessenger(const FHandler *, FLooper * = NULL);
	~FMessenger();

	// Target
	bool IsTargetLocal() const;
	BHandler *Target(BLooper **looper) const;
	bool LockTarget() const;
	status_t LockTargetWithTimeout(bigtime_t timeout) const;

	// Message sending
	status_t SendMessage(uint32 command, BHandler *replyTo = NULL) const;
	status_t SendMessage(BMessage *, BHandler *replyTo = NULL,
		bigtime_t timeout = B_INFINITE_TIMEOUT) const;
	status_t SendMessage(BMessage *, BMessenger replyTo,
		bigtime_t timeout = B_INFINITE_TIMEOUT) const;

	status_t SendMessage(uint32 command, BMessage *reply) const;
	status_t SendMessage(BMessage *message, BMessage *reply,
		bigtime_t sendTimeout = B_INFINITE_TIMEOUT,
		bigtime_t replyTimeout = B_INFINITE_TIMEOUT) const;

	// Operators and misc
	FMessenger &operator=(const FMessenger &from);
	bool operator==(const FMessenger &other) const;

	bool IsValid() const;

private:
	FLooper *fLooper;
	FHandler *fHandler;
};

#endif

/*
License

Terms and Conditions

Copyright (c) 2000-2001, Pavel Cisler
Copyright (c) 2000-2001, Gene Ragan

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met: 

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE,
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE. 
*/
