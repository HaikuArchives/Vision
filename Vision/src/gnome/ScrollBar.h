// BScrollBar-like class
//
// see License at the end of file

#ifndef SCROLLBAR__
#define SCROLLBAR__

#include "PlatformDefines.h"
#include "View.h"

const int32 B_V_SCROLL_BAR_WIDTH = 15;
const int32 B_H_SCROLL_BAR_HEIGHT = 15;

class FScrollBar : public FView {
public:
	FScrollBar(FRect, const char *, BView *, float min, float max,
		orientation);
	orientation Orientation() const;
	void SetRange(float, float);
	void SetSteps(float, float);

	float Value() const;
	void SetValue(float);
	
	float Proportion() const;
	void SetProportion(float ratio);

	static void ValueChangedCallback(void *, void *);

	static int32 VScrollBarWidth();
	static int32 HScrollBarHeight();

private:
	orientation fOrientation;
	BView *fTarget;
	bool fScrolling;
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
