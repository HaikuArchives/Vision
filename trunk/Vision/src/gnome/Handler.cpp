// BHandler-like class
//
// see License at the end of file

#include "PlatformDefines.h"
#include "Handler.h"
#include "MessageFilter.h"

FHandler::FHandler(const char *name)
	:	fName(name),
		fLooper(NULL),
		fNextHandler(NULL),
		fFilters(5, true)
	{}


void 
FHandler::SetName(const char *name)
{
	fName = name;
}

const char *
FHandler::Name() const
{
	return fName.CStr();
}


void 
FHandler::MessageReceived(FMessage *message)
{
	if (fNextHandler)
		fNextHandler->MessageReceived(message);
}

FLooper *
FHandler::Looper() const
{
	return fLooper;
}

void 
FHandler::AddFilter(FMessageFilter *filter)
{
	fFilters.AddItem(filter);
}

FHandler *
FHandler::RunFilters(ObjectList<FMessageFilter> *filters, FMessage *message)
{
	FHandler *result = this;
	int32 count = filters->CountItems();
	for (int32 index = 0; index < count; index++) {
		filters->ItemAt(index)->fOwner = Looper();
		if (filters->ItemAt(index)->Filter(message, &result) == B_SKIP_MESSAGE) {
			return NULL;
		}
	}
	return result;	
}

FHandler *
FHandler::RunFilters(FMessage *message)
{
	return RunFilters(&fFilters, message);
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
