// BPopUpMenu-like class
//
// see License at the end of file

#include "PopUpMenu.h"

#include <gtk/gtkmain.h>
#include <gtk/gtksignal.h>

FPopUpMenu::FPopUpMenu(const char *name, bool, bool)
	:	FMenu(name)
{
}

void 
FPopUpMenu::PopUpDone(GtkMenuShell *, void *)
{
	gtk_main_quit();
}

FMenuItem *
FPopUpMenu::Go(FPoint, bool, bool , bool async)
{
	if (async) {
		gtk_menu_popup(GtkObject(), NULL, NULL, NULL, NULL, 0, GDK_CURRENT_TIME);
		return NULL;
	}
	
	uint32 signalID = gtk_signal_connect((_GtkObject *)(GtkObject()),
		"deactivate", (GtkSignalFunc)&FPopUpMenu::PopUpDone, NULL);

	fInvokedItem = NULL;
	gtk_menu_popup(GtkObject(), NULL, NULL, NULL, NULL, 0, GDK_CURRENT_TIME);
	gtk_grab_add((_GtkWidget *)(GtkObject()));
	gtk_main();
	gtk_grab_remove((_GtkWidget *)(GtkObject()));

	gtk_signal_disconnect((_GtkObject *)(GtkObject()), signalID);

	return fInvokedItem;
}

void 
FPopUpMenu::Invoke(FMenuItem *menuItem)
{
	fInvokedItem = menuItem;
	FMenu::Invoke(menuItem);
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
