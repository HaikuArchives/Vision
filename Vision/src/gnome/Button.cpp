// BChecBox-like class
//
// see License at the end of file

#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>

#include "Button.h"

static FRect
ButtonFrameAdjustFromBeOS(FRect frame)
{
	frame.InsetBy(-5, -7);
	return frame;
}

FButton::FButton(FRect frame, const char *name, const char *label, BMessage *message,
	uint32 followFlags, uint32 flags)
	:	FControl(gtk_button_new_with_label(label), ButtonFrameAdjustFromBeOS(frame),
			name, label, message, followFlags, flags),
		fDefault(false)
{
	fObject->flags |= GTK_CAN_DEFAULT;
	gtk_signal_connect(fObject, "clicked", GTK_SIGNAL_FUNC(&FButton::InvokeBinder),
		this);
	gtk_signal_connect (fObject, "realize", GTK_SIGNAL_FUNC(&FButton::RealizeHook),
		this);
	SetFont(be_plain_font);
}


FButton::~FButton()
{
}

void
FButton::InvokeBinder(GtkButton *, FButton *self)
{
	ASSERT(self);
	self->Invoke(self->Message());
}

void 
FButton::MakeDefault(bool on)
{
	fDefault = on;
	if (on) {
		gtk_widget_grab_default (AsGtkWidget());
	} else 
		gtk_widget_grab_default (NULL);
}

void 
FButton::SetLabel(const char *label)
{
	gtk_label_set_text (GTK_LABEL(GTK_BIN(AsGtkWidget())->child), label);
}

void 
FButton::SetFont(const FFont *font, uint32 mode)
{
	inherited_::SetFont(font, mode);
	if (GTK_BIN (AsGtkWidget())->child
		&& GTK_IS_LABEL(GTK_BIN(AsGtkWidget())->child))
		FFont::Set(font, GTK_BIN(AsGtkWidget())->child);		
}

void 
FButton::RealizeHook(GtkButton *, FButton *self)
{
	if (self->fDefault)
		gtk_widget_grab_default (self->AsGtkWidget());		
}

void 
FButton::PrepareInitialSize(FRect rect)
{
	gtk_widget_set_uposition(AsGtkWidget(), (int)rect.left, (int)rect.top);
	gtk_widget_set_usize(AsGtkWidget(), (int)rect.Width(), (int)rect.Height());
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
