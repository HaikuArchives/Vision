// Pthread wrapper classes
//
// see License at the end of file

#include "PThread.h"


PThreadMutex::PThreadMutex(MutexType type)
{	
	switch (type) {
		case kFast:
			{
#if 0
				pthread_mutexattr_t mutexAttr = { PTHREAD_MUTEX_FAST_NP };
#endif
				// the RH 6.2 and RH 7 pthread.h headers are source-incompatible
				// for some dumb reason, just use NULL here and pray it will
				// be a fast mutex by default.
				pthread_mutex_init(&fMutex, NULL);
			}
			break;

		case kErrorChecking:
			{
				pthread_mutexattr_t mutexAttr = { PTHREAD_MUTEX_ERRORCHECK_NP };
				pthread_mutex_init(&fMutex, &mutexAttr);
			}
			break;

		case kRecursive:
			{
				pthread_mutexattr_t mutexAttr = { PTHREAD_MUTEX_RECURSIVE_NP };
				pthread_mutex_init(&fMutex, &mutexAttr);
			}
			break;
	}
}

PThreadMutex::~PThreadMutex()
{
	pthread_mutex_destroy(&fMutex);
}

status_t 
PThreadMutex::Lock()
{
	return pthread_mutex_lock(&fMutex);
}

status_t 
PThreadMutex::TryLock()
{
	return pthread_mutex_trylock(&fMutex);
}

status_t 
PThreadMutex::Unlock()
{
	return pthread_mutex_unlock(&fMutex);
}


PThreadCondition::PThreadCondition()
{
	pthread_cond_init(&fCondition, NULL);
}

PThreadCondition::~PThreadCondition()
{
	pthread_cond_destroy(&fCondition);
}

status_t 
PThreadCondition::Wait(PThreadMutex *mutex)
{
	return pthread_cond_wait(&fCondition, &mutex->fMutex);
}

status_t 
PThreadCondition::Signal()
{
	return pthread_cond_signal(&fCondition);
}

PThreadSemaphore::PThreadSemaphore()
	:	fMutex(PThreadMutex::kFast)
{
}

status_t 
PThreadSemaphore::Acquire()
{
	return fMutex.Lock();
}

status_t 
PThreadSemaphore::Release()
{
	return fMutex.Unlock();
}

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
