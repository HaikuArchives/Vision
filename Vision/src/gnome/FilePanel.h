// FilePanel
//
// See License at the end fo this file

#ifndef FILE_PANEL__
#define FILE_PANEL__

#include "PlatformDefines.h"
#include "Entry.h"
#include "Messenger.h"
#include "Message.h"
#include "StorageDefs.h"

enum FilePanelType {
	B_OPEN_PANEL,
	B_SAVE_PANEL
};

class FDirectory;
class FMessenger;
class FMessage;
class EntryRef;

struct _GtkWidget;
struct _GtkObject;

class FFilePanel {
public:
	FFilePanel(FilePanelType type, const FMessenger *, const EntryRef * = NULL,
		node_flavor mode = B_ANY_NODE, bool = true);

	virtual ~FFilePanel();

	void Show();
	void SetPanelDirectory(const FDirectory *);
	void SetSaveText(const char *);
	void SetMessage(FMessage *);
	void SetTitle();

	FFilePanel *Window();
	// fakeout magic -- we can't really return an FWindow here like BeOS,
	// just return self and provide the concievable calls that
	// a file panel would use on it's window
	// This will break for more tricky operations than setting
	// a title though
	void SetTitle(const char *);
private:

	void SetUp();

	void SaveParentDir();
	void RestoreParentDir();
	
	void Invoke();
	
	static void DestroyBinder(_GtkObject *, FFilePanel *);
	static void InvokeBinder(_GtkObject *, FFilePanel *);
	static void CancelBinder(_GtkObject *, FFilePanel *);

	FilePanelType fType;
	EntryRef fRef;
	FMessenger fTarget;
	FMessage fMessage;
	String fTitle;
	String fSaveText;
	_GtkWidget *fGtkFileSelection;
};

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
