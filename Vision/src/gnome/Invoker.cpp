// BInvoker-like class
//
// see License at the end of file

#include "PlatformDefines.h"

#include "Invoker.h"
#include "Looper.h"
#include "Message.h"

FInvoker::~FInvoker()
{
	delete fMessage;
}

void 
FInvoker::SetTarget(FHandler *target)
{
	fTarget = target;
}

void 
FInvoker::SetMessage(FMessage *message)
{
	delete fMessage;
	fMessage = message;
}

void 
FInvoker::Invoke(FMessage *message)
{
	if (!message)
		message = fMessage;
		
	if (message && fTarget && fTarget->Looper())
		fTarget->Looper()->PostMessage(message, fTarget);
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
