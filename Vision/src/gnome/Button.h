// BButton-like class
//
// see License at the end of file

#ifndef FBUTTON__
#define FBUTTON__

#include "PlatformDefines.h"

#include <gtk/gtkbutton.h>

#include "Control.h"

class FButton : public FControl {
public:
	FButton(FRect, const char *, const char *, BMessage *, 
		uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
		uint32 = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~FButton();

	void MakeDefault(bool);

	void SetLabel(const char *);
	virtual void SetFont(const FFont *, uint32 = 0);
	
private:
	static void RealizeHook(GtkButton *, FButton *);	
	static void InvokeBinder(GtkButton *, FButton *);
	
	virtual void PrepareInitialSize(FRect);


	bool fDefault;

	typedef FControl inherited_;
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
