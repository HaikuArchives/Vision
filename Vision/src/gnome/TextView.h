// BTextView-like class
//
// see License at the end of file

#ifndef FTEXT_VIEW__
#define FTEXT_VIEW__

#include "PlatformDefines.h"

#include "View.h"
#include <stdio.h>

class _GtkWidget;
#include "Box.h"

class FTextView : public FView {
public:
	FTextView(FRect, const char *, BRect,
		uint32 resizeMode, 
		uint32 flags = B_WILL_DRAW | B_PULSE_NEEDED);

	virtual void SetText(const char *);
	const char *Text() const;

	void SetTabWidth(float);
	void SetStylable(bool);
	void MakeSelectable(bool);
	void MakeEditable(bool);
	void SetWordWrap(bool);

	int32 CountLines() const;
	float LineWidth(int32) const;
	float TextHeight(int32 from, int32 to) const;
	
	void AllowChar(uint32);
	void DisallowChar(uint32);

private:
	typedef FView inherited;
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
