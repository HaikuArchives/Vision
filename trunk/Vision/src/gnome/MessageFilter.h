// BMessageFilter-like class
//
// see License at the end of file

#ifndef F_MESSAGE_FILTER_H__
#define F_MESSAGE_FILTER_H__

#include "PlatformDefines.h"

class FLooper;
class FHandler;
class FMessage;

const int32 B_ANY_DELIVERY = 1;
const int32 B_ANY_SOURCE = 1;

enum filter_result {
	B_DISPATCH_MESSAGE,
	B_SKIP_MESSAGE
};

class FMessageFilter {
public:
	FMessageFilter(int32, int32, int32 = 0)
		{}
	virtual ~FMessageFilter()
		{}

	virtual filter_result Filter(FMessage *message, FHandler **handler) = 0;

private:
	FLooper *fOwner;

friend class FHandler;
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
