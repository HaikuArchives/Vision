// BBox-like class
//
// see License at the end of file

#include "Box.h"
#include "Color.h"

FBox::FBox(FRect frame, const char *name, uint32 followFlags,
	uint32 viewFlags, uint32 kind)
	:	FView(frame, name, followFlags, viewFlags),
		fKind(kind)
{
	SetViewColor(Color(210, 210, 210));
	SetLowColor(Color(210, 210, 210));
}

FBox::~FBox()
{
}

void 
FBox::Draw(FRect rect)
{
	FillRect(rect, B_SOLID_LOW);
	if (fKind == B_PLAIN_BORDER) {
		BRect bounds(Bounds());
		bounds.InsetBy(1, 1);
		rgb_color low = LowColor().ShiftBy(0.6);
		rgb_color high = LowColor().ShiftBy(1.4);
		
		BeginLineArray(4);
		AddLine(bounds.LeftTop(), bounds.RightTop(), low);
		AddLine(bounds.LeftTop(), bounds.LeftBottom(), low);
		AddLine(bounds.RightTop(), bounds.RightBottom(), high);
		AddLine(bounds.LeftBottom(), bounds.RightBottom(), high);
		EndLineArray();
	}
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
