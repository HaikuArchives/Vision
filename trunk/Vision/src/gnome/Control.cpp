// BControl-like class
//
// see License at the end of file

#include "Control.h"
#include "Window.h"
#include <gtk/gtksignal.h>

FControl::FControl(GtkWidget *widget, FRect frame, const char *name,
	const char *, FMessage *message, uint32 resizeFlags,
	uint32 flags)
	:	FView(widget, frame, name, resizeFlags, flags)
{
	SetMessage(message);
	gtk_signal_connect (fObject, "realize", GTK_SIGNAL_FUNC(&FControl::RealizeHook),
		this);
}

bool
FControl::Value() const
{
	return true;
}

void 
FControl::SetValue(bool)
{
}

bool 
FControl::IsEnabled() const
{
	return (fObject->flags & GTK_SENSITIVE) != 0;
}

void 
FControl::SetEnabled(bool on)
{
	gtk_widget_set_sensitive(AsGtkWidget(), on);
}

void 
FControl::AttachedToWindow()
{
	if (!Target())
		SetTarget(Window());
}


void 
FControl::RealizeHook(GtkWidget *, FControl *control)
{
	control->AttachedToWindow();
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
