// BLocker-like class
//
// see License at the end of file

#include "Locker.h"

FLocker::FLocker()
	:	fMutex(PThreadMutex::kRecursive),
		fLockCounter(0)
{
}


FLocker::~FLocker()
{
	while (IsLocked())
		if (fMutex.Unlock() != B_OK)
			break;
}

bool 
FLocker::Lock()
{
	status_t result = fMutex.Lock();
	if (result == B_OK)
		fLockCounter++;
	return result == B_OK;
}

void 
FLocker::Unlock()
{
	if (fMutex.Unlock() == B_OK)
		fLockCounter--;
}

bool 
FLocker::IsLocked() const
{
	return fLockCounter > 0;
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
