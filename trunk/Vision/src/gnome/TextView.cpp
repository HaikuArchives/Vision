// BTextView-like class
//
// see License at the end of file

#include <gtk/gtktext.h>
#include "TextView.h"

FTextView::FTextView(FRect rect, const char *name, BRect, uint32 mode, uint32 flags)
	:	BView(gtk_text_new(NULL, NULL), rect, name, mode, flags)
{
	SetFont(be_plain_font);
}

void 
FTextView::SetText(const char *text)
{
	FFont font;
	GetFont(&font);
	
	gtk_text_insert(GTK_TEXT(GtkObject()), font.GdkObject(),
		NULL, NULL, text, -1);
}

const char *
FTextView::Text() const
{
	return "not yet implemented";
}

void 
FTextView::SetTabWidth(float)
{
}

void 
FTextView::SetStylable(bool)
{
}

void 
FTextView::MakeSelectable(bool)
{
}

void 
FTextView::MakeEditable(bool on)
{
	gtk_text_set_editable(GTK_TEXT(GtkObject()), on);
}

void 
FTextView::SetWordWrap(bool on)
{
	gtk_text_set_editable(GTK_TEXT(GtkObject()), on);
}

int32 
FTextView::CountLines() const
{
	return 10;
}

float 
FTextView::LineWidth(int32) const
{
	return 100;
}

float 
FTextView::TextHeight(int32, int32) const
{
	return 100;
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
