// FilePanel
//
// See License at the end fo this file

#include "Directory.h"
#include "FilePanel.h"
#include "Messenger.h"
#include "Message.h"
#include "Path.h"

#include <gtk/gtkfilesel.h>
#include <gtk/gtksignal.h>


FFilePanel::FFilePanel(FilePanelType type, const FMessenger *target,
	const EntryRef *ref, node_flavor, bool)
	:	fType(type),
		fTarget(*target),
		fMessage(B_REFS_RECEIVED),
		fGtkFileSelection(NULL)
{
	if (ref)
		fRef = *ref;
	
	fTitle = fType == B_OPEN_PANEL ? "Open" : "Save";
}

void 
FFilePanel::SetUp()
{
	fGtkFileSelection = gtk_file_selection_new (fTitle.CStr());

	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(fGtkFileSelection));

	gtk_window_set_position(GTK_WINDOW(fGtkFileSelection), GTK_WIN_POS_MOUSE);

	gtk_signal_connect(GTK_OBJECT(fGtkFileSelection), "destroy",
		GTK_SIGNAL_FUNC(FFilePanel::DestroyBinder), this);

	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fGtkFileSelection)->ok_button),
		"clicked", GTK_SIGNAL_FUNC(FFilePanel::InvokeBinder), this);

	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fGtkFileSelection)->cancel_button),
		"clicked", GTK_SIGNAL_FUNC(FFilePanel::CancelBinder), this);
	
	RestoreParentDir();
}

void 
FFilePanel::DestroyBinder(GtkObject *, FFilePanel *self)
{
	self->fGtkFileSelection = NULL;
}

void 
FFilePanel::SaveParentDir()
{
	FEntry entry(gtk_file_selection_get_filename(
		GTK_FILE_SELECTION(fGtkFileSelection)));
	FEntry parent;
	if (entry.GetParent(&parent) != B_OK)
		return;
	
	parent.GetRef(&fRef);
}

void 
FFilePanel::RestoreParentDir()
{
	if (fRef.name == NULL)
		return;
	
	ASSERT(fGtkFileSelection);

	String path(fRef.Path());
	if (path.Length() < 1)
		return;

	if (path[path.Length() - 1] != '/')
		path += '/';
	gtk_file_selection_set_filename (GTK_FILE_SELECTION(fGtkFileSelection), path.CStr());
}

void 
FFilePanel::InvokeBinder(GtkObject *, FFilePanel *self)
{
	self->Invoke();
}

void 
FFilePanel::Invoke()
{
	EntryRef ref(gtk_file_selection_get_filename(
		GTK_FILE_SELECTION(fGtkFileSelection)));
	
	SaveParentDir();

	if (fType == B_SAVE_PANEL) {
		FEntry entry(&ref);
		FEntry parent;
		if (entry.GetParent(&parent) == B_OK) {
			EntryRef parentRef;
			parent.GetRef(&parentRef);
			char name[256];
			entry.GetName(name);
			fMessage.AddRef("directory", &parentRef);
			fMessage.AddString("name", name);
		}
	} else
		fMessage.AddRef("refs", &ref);

	fTarget.SendMessage(&fMessage);
	gtk_widget_destroy(fGtkFileSelection);
}


void 
FFilePanel::CancelBinder(GtkObject *, FFilePanel *self)
{
	self->SaveParentDir();
	gtk_widget_destroy(self->fGtkFileSelection);
}


FFilePanel::~FFilePanel()
{
	if (fGtkFileSelection)
		gtk_widget_destroy(fGtkFileSelection);
		
}

void 
FFilePanel::Show()
{
	if (!fGtkFileSelection)
		SetUp();

	gtk_widget_show(fGtkFileSelection);
}

void 
FFilePanel::SetPanelDirectory(const FDirectory *directory)
{
	EntryRef ref;
	if (directory->GetRef(&ref) != B_OK)
		return;
	
	fRef = ref;
	
	if (fGtkFileSelection)
		RestoreParentDir();
}

void 
FFilePanel::SetSaveText(const char *text)
{
	fSaveText = text;
//	if (fGtkFileSelection)
//		gtk_window_set_title (GTK_WINDOW (fGtkFileSelection), title);
}

void 
FFilePanel::SetMessage(FMessage *message)
{
	fMessage = *message;
}

FFilePanel *
FFilePanel::Window()
{
	return this;
}

void 
FFilePanel::SetTitle(const char *title)
{
	fTitle = title;
	if (fGtkFileSelection)
		gtk_window_set_title (GTK_WINDOW (fGtkFileSelection), title);
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
