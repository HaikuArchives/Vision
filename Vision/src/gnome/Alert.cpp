// BAlert-like class
//
// see License at the end of file

#include "Alert.h"
#include <libgnomeui/gnome-dialog.h>
#include <gtk/gtkbox.h>
#include <gtk/gtklabel.h>
#include <libgnomeui/gnome-uidefs.h>

static void
add_label_to_dialog (GnomeDialog *dialog, const char *message)
{
	GtkLabel *message_widget;

	if (message == NULL) {
		return;
	}
	
	message_widget = GTK_LABEL (gtk_label_new (message));
	gtk_label_set_line_wrap (message_widget, TRUE);
	gtk_label_set_justify (message_widget,
			       GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (dialog)->vbox),
			    GTK_WIDGET (message_widget),
			    TRUE, TRUE, GNOME_PAD);
}

FAlert::FAlert(const char *name, const char *label, const char *button1,
	const char *button2, const char *button3, button_width,	alert_type)
{
	const char *buttonTitles[4] = {button1, button2, button3};
	fDialog = gnome_dialog_newv (name, buttonTitles);
	gnome_dialog_set_close (GNOME_DIALOG(fDialog), true);
	add_label_to_dialog(GNOME_DIALOG(fDialog), label);
	gtk_widget_show_all(fDialog);
}

void 
FAlert::SetShortcut(int32, int32)
{
}

int32 
FAlert::Go()
{
	return gnome_dialog_run_and_close(GNOME_DIALOG(fDialog));
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
