// BHandler-like class
//
// see License at the end of file

#ifndef FHANDLER_H__
#define FHANDLER_H__

#include "PlatformDefines.h"

#include "CString.h"
#include "ObjectList.h"
#include "MessageFilter.h"

class FLooper;
class FMessage;

class FHandler {
public:
	FHandler(const char *name = NULL);
	virtual ~FHandler()
		{}

	void SetName(const char *);
	const char *Name() const;

	virtual void MessageReceived(FMessage *);

	FLooper *Looper() const;

	void AddFilter(FMessageFilter *);

protected:
	FHandler *RunFilters(ObjectList<FMessageFilter> *, FMessage *);
	FHandler *RunFilters(FMessage *);

	String fName;
	FLooper *fLooper;
	FHandler *fNextHandler;
	ObjectList<FMessageFilter> fFilters;
	
friend class FLooper;
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
