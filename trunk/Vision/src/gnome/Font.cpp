// BFont-like class
//
// see License at the end of file

#include "PlatformDefines.h"

#include "Font.h"
#include "CString.h"

#include <gtk/gtkstyle.h>
#include <gtk/gtkwidget.h>

FFont *be_fixed_font;
FFont *be_plain_font;
FFont *be_bold_font;

FFont::FFont()
	:	fFont(gdk_font_load("-Misc-Fixed-Medium-R-Normal--10-100-75-75-C-60-ISO8859-1"))
{
	gdk_font_ref(fFont);
}

FFont::FFont(const char *name)
	:	fFont(gdk_font_load(name))
{
	gdk_font_ref(fFont);
}

FFont::~FFont()
{
	gdk_font_unref(fFont);
}

FFont::FFont(const FFont &font)
	:	fFont(font.fFont)
{
	gdk_font_ref(fFont);
}

const FFont & 
FFont::operator=(const FFont &font)
{
	fFont = font.fFont;
	gdk_font_ref(fFont);

	return *this;
}

bool 
FFont::operator==(const FFont &) const
{
	return true;
}

void 
FFont::GetHeight(font_height *result) const
{
	result->ascent = (float)fFont->ascent;
	result->descent = (float)fFont->descent;
	result->leading = 1.0f;
}

void 
FFont::SetSize(float)
{
}

float 
FFont::Size() const
{
	return fFont->ascent + fFont->descent;
}

void
FFont::GetEscapements(const char *text, int32 length, float *result) const
{
	ASSERT(length == 1);
	*result = (float)gdk_text_width(fFont, text, length) / Size();
}

float 
FFont::StringWidth(const char *string) const
{
	return (float)gdk_string_width(fFont, string);
}

float 
FFont::Truncate(String &result, float width, uint32 /*mode*/, bool *stringChanged) const
{
	int newWidth = 0;
	bool changed = false;
	int stringLength;

	for (;;) {
		stringLength = result.Length();
		if (changed && stringLength <= 3)
			break;
		newWidth = gdk_text_width(fFont, result.CStr(), stringLength);
		if (newWidth <= width)
			break;
		if (!changed) {
			result += "...";
			changed = true;
		} else {
			result[stringLength - 3] = '.';
			result.Truncate(stringLength - 1);
		}
	}
	if (stringChanged)
		*stringChanged = changed;

	return (float)newWidth;
}

void 
FFont::InitFonts()
{
	be_fixed_font = new FFont("-Misc-Fixed-Medium-R-Normal--10-100-75-75-C-60-ISO8859-1");
	be_plain_font = new FFont("-adobe-helvetica-medium-r-normal-*-*-100-*-*-p-*-*");
	be_bold_font = new FFont("-adobe-helvetica-bold-r-normal-*-*-100-*-*-p-*-*");
	
}

void 
FFont::Set(const FFont *font, GtkWidget *widget)
{
	gtk_widget_ensure_style(widget);

	GtkStyle *style = gtk_style_copy (gtk_widget_get_style (widget));
	
	gdk_font_ref (font->fFont);
	gdk_font_unref (style->font);
	style->font = font->fFont;
	
	gtk_widget_set_style (widget, style);
	gtk_style_unref (style);
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
