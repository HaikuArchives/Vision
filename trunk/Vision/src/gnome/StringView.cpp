
#include "StringView.h"
#include "Font.h"

#include "gtk/gtklabel.h"
#include "gtk/gtkbin.h"

FStringView::FStringView(BRect bounds, const char *name,
	const char *text, uint32 resizeFlags, uint32 flags)
	:	FView(gtk_label_new(text), bounds, name, resizeFlags, flags),
		fAlignment(B_LEFT)		
{
	SetFont(be_plain_font);
}


FStringView::~FStringView()
{
}

void 
FStringView::SetText(const char *text)
{
	gtk_label_set_text(GTK_LABEL(GTK_BIN(fObject)->child), text);
}

const char *
FStringView::Text() const
{
	return GTK_LABEL(GTK_BIN(fObject)->child)->label;
}

void 
FStringView::SetAlignment(alignment align)
{
	fAlignment = align;
	switch (align) {
		case B_LEFT:
			gtk_label_set_justify(GTK_LABEL(GTK_BIN(fObject)->child),
				GTK_JUSTIFY_LEFT);
			break;
		case B_CENTER:
			gtk_label_set_justify(GTK_LABEL(GTK_BIN(fObject)->child),
				GTK_JUSTIFY_CENTER);
			break;
		case B_RIGHT:
			gtk_label_set_justify(GTK_LABEL(GTK_BIN(fObject)->child),
				GTK_JUSTIFY_RIGHT);
			break;
	}
}

alignment 
FStringView::Alignment() const
{
	return fAlignment;
}

void 
FStringView::GetPreferredSize(float *, float *)
{
}

void 
FStringView::AttachedToWindow()
{
	FView::AttachedToWindow();
	SetLowColor(Parent()->LowColor());
	SetViewColor(Parent()->ViewColor());
}

void 
FStringView::Draw(BRect bounds)
{
	FView::Draw(bounds);
}

void 
FStringView::SetFont(const FFont *font, uint32)
{
	FFont::Set(font, AsGtkWidget());
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
