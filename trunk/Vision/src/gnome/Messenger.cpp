// BMessenger-like class
//
// see License at the end of file

#include "Handler.h"
#include "Looper.h"
#include "Messenger.h"
#include "Message.h"

FMessenger::FMessenger()
	:	fLooper(NULL),
		fHandler(NULL)
{
}


FMessenger::FMessenger(const FMessenger &cloneThis)
	:	fLooper(cloneThis.fLooper),
		fHandler(const_cast<FHandler *>(cloneThis.fHandler))
{
}


FMessenger::FMessenger(const FHandler *handler, FLooper *looper)
	:	fLooper(looper),
		fHandler(const_cast<FHandler *>(handler))
{
	if (!looper)
		fLooper = handler->Looper();
}


FMessenger::~FMessenger()
{
}

bool 
FMessenger::IsTargetLocal() const
{
	return true;
}

BHandler *
FMessenger::Target(BLooper **looper) const
{
	if (!IsValid())
		return NULL;

	if (looper)
		*looper = fLooper;

	return fHandler;
}

bool 
FMessenger::LockTarget() const
{
	if (!IsValid())
		return false;

	return fLooper->Lock();
}

status_t 
FMessenger::LockTargetWithTimeout(bigtime_t ) const
{
	ASSERT(!"implement me");
	return B_OK;
}

status_t 
FMessenger::SendMessage(uint32 what, BHandler *replyTo) const
{
	ASSERT(!replyTo || !"implement me");

	FMessage message(what);
	return SendMessage(&message, replyTo);
}

status_t 
FMessenger::SendMessage(BMessage *message, BHandler *, bigtime_t ) const
{
	if (!IsValid())
		return B_ERROR;
	
	return fLooper->PostMessage(message, fHandler);
}

status_t 
FMessenger::SendMessage(BMessage *, BMessenger , bigtime_t ) const
{
	ASSERT(!"implement me");
	return B_OK;
}

status_t 
FMessenger::SendMessage(uint32 , BMessage *) const
{
	ASSERT(!"implement me");
	return B_OK;
}

status_t 
FMessenger::SendMessage(BMessage *, BMessage *, bigtime_t , bigtime_t ) const
{
	ASSERT(!"implement me");
	return B_OK;
}

FMessenger &
FMessenger::operator=(const FMessenger &copyThis)
{
	if (&copyThis != this) {
		fLooper = copyThis.fLooper;
		fHandler = copyThis.fHandler;
	}
	return *this;
}

bool 
FMessenger::operator==(const FMessenger &other) const
{
	return fLooper == other.fLooper && fHandler == other.fHandler;
}

bool 
FMessenger::IsValid() const
{
	if (!fLooper)
		return false;

	return FLooper::IsValid(fLooper);
}

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
