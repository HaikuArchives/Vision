// BChecBox-like class
//
// see License at the end of file

#include "CheckBox.h"

#include <gtk/gtkcheckbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>

FCheckBox::FCheckBox(FRect frame, const char *name,
	const char *label, FMessage *message, uint32 followFlags,
	uint32 flags)
	:	FControl(gtk_check_button_new_with_label(label), frame,
			name, label, message, followFlags, flags)
{
	gtk_signal_connect(fObject, "toggled", GTK_SIGNAL_FUNC(&FCheckBox::InvokeBinder),
		this);
	SetFont(be_plain_font);
}

void
FCheckBox::InvokeBinder(GtkWidget *, FCheckBox *self)
{
	ASSERT(self);
	self->Invoke(self->Message());
}

bool 
FCheckBox::Value() const
{
	GtkToggleButton *button = &((GtkCheckButton *)GtkObject())->toggle_button;
	return gtk_toggle_button_get_active(button);
}

void 
FCheckBox::SetValue(bool on)
{
	GtkToggleButton *button = &((GtkCheckButton *)GtkObject())->toggle_button;
	gtk_toggle_button_set_active(button, on);
}

void 
FCheckBox::SetFont(const FFont *font, uint32 mode)
{
	inherited::SetFont(font, mode);
	if (GTK_BIN (AsGtkWidget())->child
		&& GTK_IS_LABEL(GTK_BIN(AsGtkWidget())->child))
		FFont::Set(font, GTK_BIN(AsGtkWidget())->child);		
}

void 
FCheckBox::PrepareInitialSize(FRect rect)
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
