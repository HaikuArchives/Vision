// BLooper-like class
//
// see License at the end of file

#ifndef FLOOPER_H__
#define FLOOPER_H__

#include "PlatformDefines.h"

#include "Handler.h"
#include "ObjectList.h"
#include "FunctionObjectMessage.h"
#include "Locker.h"
#include <set>
#include <stack>

class FLooper : public FHandler {
public:	
	FLooper(const char * = NULL);
	virtual	~FLooper();

	void AddHandler(FHandler *);
	void RemoveHandler(FHandler *);

	status_t PostMessage(FMessage *, FHandler * = NULL);
	status_t PostMessage(uint32);
	status_t PostMessage(uint32, FHandler *);

	void AddCommonFilter(FMessageFilter *);
	void RemoveCommonFilter(FMessageFilter *);

	// non-BeOS only
	virtual void ReceiveMessage(FMessage *, FHandler *);

	FMessage *CurrentMessage();

	static bool IsValid(const FLooper *);
	
	virtual bool Lock();
	virtual void Unlock();
	virtual bool IsLocked();
	
	virtual void Quit();
	virtual void Run();

protected:
	virtual void DispatchMessage(FMessage *, FHandler *);

// updated gcc note - this should work, failed with gcc 2.91
// template <class Result, class Param1> Result CallInMainThread(Result (FLooper::*)(Param1), Param1);

	status_t PostMessageSoon(FMessage *, FHandler * = NULL);

public: // public because gcc had trouble with CallInMainThread being a member template
	static status_t NotifyLooperInMainThread(FLooper *, FunctionObject *);
	static status_t SynchronizeLooperInMainThread(FLooper *, FunctionObject *);

private:

	FHandler *fDefaultHandler;
	ObjectList<FMessageFilter> fCommonFilters;
	int32 fLooperID;
	FLocker *fLocker;

protected:
	stack<FMessage *> fCurrentMessage;
	bool fDeletingSelf;

private:
	static int32 looperID;
	static set<pair<FLooper *, int32> > looperList;
		// looper list is used to track loopers and validate an 
		// arbitrary pointer to be a valid looper.
};

template <class Looper, class Param1>
void
LooperCallInMainThread(Looper *looper, void (*call)(Looper *, Param1), Param1 param1)
{
	looper->NotifyLooperInMainThread(looper, new TwoParamFunctionObject<Looper *, Param1>(call, looper, param1));
}

template <class Looper, class Param1, class Param2>
void
LooperCallInMainThread(Looper *looper, void (*call)(Looper *, Param1, Param2), Param1 param1,
	Param2 param2)
{
	looper->NotifyLooperInMainThread(looper,
		new ThreeParamFunctionObject<Looper *, Param1, Param2>(call, looper, param1, param2));
}

template <class Looper, class Result, class Param1>
Result
LooperCallInMainThread(Looper *looper, Result (Looper::*call)(Param1), Param1 param1)
{
	SingleParamMemberFunctionObjectWithResult<Looper, Result, Param1> functor(call, looper, param1);
	looper->SynchronizeLooperInMainThread(looper, &functor);
	return functor.Result();
}


#if 0
template <class Result, class Param1>
Result 
FLooper::CallInMainThread(Result (FLooper::*call)(Param1), Param1 param1)
{
	SingleParamMemberFunctionObjectWithResult<FLooper, Result, Param1> functor(call, this, param1);
	CallLooperInMainThread(this, &functor);
	return functor.Result();
}
#endif

#endif

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
