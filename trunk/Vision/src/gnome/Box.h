// BBox-like class
//
// see License at the end of file

#ifndef FBOX__
#define FBOX__

#include "PlatformDefines.h"

#include "View.h"

const uint32 B_PLAIN_BORDER = 1;

class FBox : public FView {
public:
	FBox(FRect, const char *, uint32 followFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
	     uint32 viewFlags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE_JUMP,
	     uint32 border_style = 0);
	virtual ~FBox();
	
protected:
	virtual void Draw(FRect);
	
private:
	uint32 fKind;
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
