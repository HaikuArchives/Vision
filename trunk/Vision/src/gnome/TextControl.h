// BTextControl-like class
//
// see License at the end of file

#ifndef FTEXT_CONTROL__
#define FTEXT_CONTROL__

#include "PlatformDefines.h"

#include "Control.h"
#include "TextView.h"

typedef struct _GtkWidget GtkWidget;

class FTextControl : public FControl {
public:
	FTextControl(FRect, const char *, const char *, const char *,
		BMessage *, 
		uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP, 
		uint32 = B_WILL_DRAW | B_NAVIGABLE);
	virtual ~FTextControl();

	virtual void SetText(const char *);
	const char *Text() const;
	int32 TextLength() const;

	virtual void Select(int32, int32);

	virtual void Delete();
	virtual void Insert(const char *);

	virtual void MakeFocus(bool);

	static int KeyPressHook(GtkWidget *, GdkEventKey *);
	
	void SetDivider(float);
	
	void SetModificationMessage(FMessage *);
	FMessage *ModificationMessage() const;

	BTextView *TextView() const;
	
	virtual void SetFont(const FFont *, uint32 = 0);
	

protected:
	virtual void KeyDown(const char *bytes, int32 numBytes);
	
private:
	static void ChangedBinder(GtkWidget *, FTextControl *);
	static FTextControl *GnoBeObject(GtkWidget *gtkObject)
		{ return ::GnoBeObject<FTextControl, GtkWidget>(gtkObject); }

	virtual void PrepareInitialSize(FRect);

	GtkWidget *fText;
	mutable char *fGetTextBuffer;
	GtkWidget *fLabel;
	FMessage *fModificationMessage;

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
