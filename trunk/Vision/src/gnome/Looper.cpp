// BLooper-like class
//
// see License at the end of file

#include "Looper.h"
#include "Message.h"

#include <gtk/gtkmain.h>
#include "PThread.h"
#include <stdio.h>

// Note that unlike the orignal BLooper, we do not create a separate
// thread here by default -- this would be a problem on Gtk+ where
// all of the Gtk+ code needs to run in the main thread.

FLooper::FLooper(const char *name)
	:	FHandler(name),
		fLooperID(looperID++),
		fLocker(NULL),
		fDeletingSelf(false)
{
	fLooper = this;
	fDefaultHandler = this;
	looperList.insert(pair<FLooper *, int32>(this, fLooperID));
}
	
FLooper::~FLooper()
{
	looperList.erase(pair<FLooper *, int32>(this, fLooperID));
	delete fLocker;
}

void 
FLooper::Quit()
{
	if (!fDeletingSelf)
		delete this;
}

void 
FLooper::Run()
{
	ASSERT(!"implement me");
}

bool 
FLooper::IsValid(const FLooper *looper)
{
	for (set<pair<FLooper *, int32> >::iterator i = looperList.begin();
		i != looperList.end(); i++) {
		if ((*i).first == looper) {
			// we know for sure that looper is mapped memory,
			// OK to dereference it to match the unique identifier.
			// If fLooperID does not match we have a case of an 
			// aliased pointer to a new Looper structure.
			return (*i).second == looper->fLooperID;
		}
	}
	return false;
}

bool 
FLooper::Lock()
{
	if (!fLocker)
		fLocker = new BLocker();

	return fLocker->Lock();
}

void 
FLooper::Unlock()
{
	fLocker->Unlock();
}

bool 
FLooper::IsLocked()
{
	if (!fLocker)
		return false;
	
	return fLocker->IsLocked();
}


void 
FLooper::AddHandler(FHandler *handler)
{
	handler->fNextHandler = fDefaultHandler;
	handler->fLooper = this;
	fDefaultHandler = handler;
}

void 
FLooper::RemoveHandler(FHandler *handler)
{
	handler->fLooper = NULL;
	
	if (fDefaultHandler == handler)
		fDefaultHandler = handler->fNextHandler;
	else 
		for (FHandler *previous = fDefaultHandler;
			previous != NULL;
			previous = previous->fNextHandler)
			if (previous->fNextHandler == handler) {
				previous->fNextHandler = handler->fNextHandler;
				break;
			}

	handler->fNextHandler = NULL;
}


status_t 
FLooper::PostMessage(FMessage *message, FHandler *handler)
{
	return PostMessageSoon(message, handler);
}

status_t 
FLooper::PostMessage(uint32 what)
{
	FMessage message(what);
	return PostMessageSoon(&message, NULL);
}

status_t 
FLooper::PostMessage(uint32 what, FHandler *handler)
{
	FMessage message(what);
	return PostMessageSoon(&message, handler);
}

struct PostMessageSoonParams {
	FLooper *looper;
	FHandler *handler;
	FMessage *message;
};

static gboolean 
PostMessageSoonBinder(void *castToParams)
{
	PostMessageSoonParams *params = (PostMessageSoonParams *)castToParams;
	if (FLooper::IsValid(params->looper))
		params->looper->ReceiveMessage(params->message, params->handler);

	delete params->message;
	delete params;	
	return false;
}

status_t 
FLooper::PostMessageSoon(FMessage *message, FHandler *handler)
{
	PostMessageSoonParams *params =  new PostMessageSoonParams;
	params->looper = this;
	params->handler = handler;
	params->message = new FMessage(*message);
	gtk_idle_add(PostMessageSoonBinder, params);
	
	return B_OK;
}

void 
FLooper::DispatchMessage(FMessage *message, FHandler *handler)
{
	handler->MessageReceived(message);
}

void 
FLooper::AddCommonFilter(FMessageFilter *filter)
{
	fCommonFilters.AddItem(filter);
}

void 
FLooper::RemoveCommonFilter(FMessageFilter *filter)
{
	fCommonFilters.RemoveItem(filter);
}

void 
FLooper::ReceiveMessage(FMessage *message, FHandler *handler)
{
	fCurrentMessage.push(message);

	if (!handler)
		handler = fDefaultHandler;

	if (!handler)
		handler = this;

	// apply common filters if any
	for (;;) {
		FHandler *oldHandler = handler;
		handler = handler->RunFilters(&fCommonFilters, message);
		// keep going while handling gets redirected
		if (oldHandler == handler || handler == NULL)
			break;
	}

	// apply filters if any
	if (handler)
		for (;;) {
			FHandler *oldHandler = handler;
			handler = handler->RunFilters(message);
			// keep going while handling gets redirected
			if (oldHandler == handler || handler == NULL)
				break;
		}

	if (handler)
		DispatchMessage(message, handler);

	fCurrentMessage.pop();
}

FMessage *
FLooper::CurrentMessage()
{
	if (fCurrentMessage.empty())
		return NULL;
	return fCurrentMessage.top();
}

struct NotifyLooperInMainThreadParams {
	FLooper *looper;
	FunctionObject *functor;
};

static gboolean 
NotifyLooperInMainThreadBinder(void *castToParams)
{
	NotifyLooperInMainThreadParams *params =
		(NotifyLooperInMainThreadParams *)castToParams;
	
	if (FLooper::IsValid(params->looper))
		params->functor->operator()();
	
	delete params->functor;	
	delete params;	
	return false;
}

status_t 
FLooper::NotifyLooperInMainThread(FLooper *looper, FunctionObject *functor)
{
	NotifyLooperInMainThreadParams *params =  new NotifyLooperInMainThreadParams;
	params->looper = looper;
	params->functor = functor;
	gtk_idle_add(NotifyLooperInMainThreadBinder, params);
	return B_OK;
}

struct SynchronizeLooperInMainThreadParams {
	FLooper *looper;
	FunctionObject *functor;

	PThreadSemaphore lock;
};

static gboolean
SynchronizeLooperInMainThreadBinder(void *castToParams)
{
	SynchronizeLooperInMainThreadParams *params =
		(SynchronizeLooperInMainThreadParams *)castToParams;

	if (FLooper::IsValid(params->looper))
		params->functor->operator()();
	
	params->lock.Release();
	return false;
}

status_t 
FLooper::SynchronizeLooperInMainThread(FLooper *looper, FunctionObject *functor)
{
	SynchronizeLooperInMainThreadParams params;
	
	params.looper = looper;
	params.functor = functor;
	
	params.lock.Acquire();
	
	gtk_idle_add(SynchronizeLooperInMainThreadBinder, &params);

	params.lock.Acquire();
	return B_OK;
}


int32 FLooper::looperID = 0;
set<pair<FLooper *, int32> > FLooper::looperList;

/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler

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
