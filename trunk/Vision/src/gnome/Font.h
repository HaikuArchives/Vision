// BFont-like class
//
// see License at the end of file

#ifndef FFONT__
#define FFONT__

#include "PlatformDefines.h"

#include "CString.h"

#include <gdk/gdk.h>

struct font_height {
	float ascent;
	float descent;
	float leading;
};

const int32 B_BITMAP_SPACING = 1;
const int32 B_ISO_8859_2 = 1;

enum {
	B_TRUNCATE_BEGINNING,
	B_TRUNCATE_MIDDLE,
	B_TRUNCATE_END
};


const uint32 B_FONT_SIZE = 3;
const uint32 B_FONT_FAMILY_AND_STYLE = 3;

class CString;

typedef struct _GtkWidget GtkWidget;

class FFont {
public:
	FFont();
	FFont(const char *);
	FFont(const FFont &);
	~FFont();

	const FFont &operator=(const FFont &);
	bool operator==(const FFont &) const;

	void GetHeight(font_height *) const;

	void SetSize(float);
	float Size() const;

	float Truncate(String &, float width, uint32, bool *) const;
	
	// fakeout calls
	void GetEscapements(const char *, int32, float *) const;

	float StringWidth(const char *) const;

	void SetRotation(float)
		{}
	void SetSpacing(int32)
		{}
	void SetEncoding(int32)
		{}
	void SetFlags(int32)
		{}

	GdkFont *GdkObject()
		{ return fFont; }
	const GdkFont *GdkObject() const
		{ return fFont; }

	// Gtk - only
	static void Set(const FFont *, GtkWidget *);

	static void InitFonts();
private:
	GdkFont *fFont;
};

extern FFont *be_fixed_font;
extern FFont *be_plain_font;
extern FFont *be_bold_font;

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
