// Pthread wrapper classes
//
// see License at the end of file

#ifndef PTHREAD__
#define PTHREAD__

#include "PlatformDefines.h"
#include <pthread.h>

class PThreadMutex {
public:
	enum MutexType {
		kFast,
		kErrorChecking,
		kRecursive
	};
		
	PThreadMutex(MutexType type = kFast);
	~PThreadMutex();

	status_t Lock();
	status_t TryLock();
		// if mutex is locked, return immediately with EBUSY
	
	status_t Unlock();

private:
	pthread_mutex_t fMutex;
	
	friend class PThreadCondition;
};

class PThreadCondition {
public:
	PThreadCondition();
	~PThreadCondition();

	status_t Wait(PThreadMutex *);
	status_t Signal();
	
private:
	pthread_cond_t fCondition;
};

class PThreadSemaphore {
public:
	PThreadSemaphore();
	
	status_t Acquire();
	status_t Release();
	
private:
	PThreadMutex fMutex;	
};

class Benaphore {
public:
	Benaphore(const char * = "lock")
		:	count(1)
		{}
	
	~Benaphore()
		{
		}
	
	bool Lock()
		{
#ifdef HAVE_ATOMIC_ADD
			if (atomic_add(&count, -1) <= 0)
				return acquire_sem(semaphore) == B_OK;

			return true;
#else
			count++;
			return fSemaphore.Acquire() == B_OK;
#endif
		}
		
	void Unlock()
		{
#ifdef HAVE_ATOMIC_ADD
			if (atomic_add(&count, 1) < 0)
				release_sem(semaphore);
#else
			count--;
			fSemaphore.Release();
#endif
		}

	bool IsLocked() const
		{ return count <= 0; }

private:
	PThreadSemaphore fSemaphore;
	int32 count;
};

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
