// BScrollView-like class
//
// see License at the end of file

#ifndef SCROLL_VIEW__
#define SCROLL_VIEW__

#include "PlatformDefines.h"
#include "ScrollBar.h"

typedef uint32 border_style;

const uint32 B_NO_BORDER = 1;
const uint32 B_FANCY_BORDER = 2;

class FScrollView : public FView {
public:
	FScrollView(const char *, BView *, uint32 = B_FOLLOW_LEFT,
		uint32 = 0, bool = false, bool = false,
		border_style = B_FANCY_BORDER);
	
	virtual FScrollBar *ScrollBar(orientation) const;
	

protected:
	virtual void Draw(FRect);
	virtual void MouseDown(BPoint point);
	virtual void MouseUp(BPoint point);
	virtual void MouseMoved(BPoint point, uint32, const BMessage *);

private:
	FScrollBar *fHorizontal;
	FScrollBar *fVertical;
	FPoint fTrackInitialPoint;
	FPoint fTrackInitialWindowSize;
	FPoint fTrackLastPoint;
	bool fTracking;
	BRect fResizeCornerRect;
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
