// Gnome-compatible clipboard handling.
//
// see License at the end of file

#ifndef GNOME_CLIPBOARD_HANDLER
#define GNOME_CLIPBOARD_HANDLER

#include "PlatformDefines.h"

#include "CString.h"
#include <gdk/gdktypes.h>

typedef struct _GtkWidget GtkWidget;
typedef struct _GtkSelectionData GtkSelectionData;
typedef struct _GdkEventSelection GdkEventSelection;

struct GtkSelectionHandlerClass;


class GnomeClipboardHandler {
public:
	GnomeClipboardHandler();
	~GnomeClipboardHandler();

	bool Empty() const;

	const char *Text() const;
	int32 Length() const;

	void Set(const char *, int32);
	void Clear();

	static void Get(GtkWidget *, GtkSelectionData *, uint, uint);
	static int ClearBinder(GtkWidget *, GdkEventSelection *);
	static void Received(GtkWidget *, GtkSelectionData *, uint);

	static void InitSelectionCommon(GtkWidget *, GdkAtom clipboardType);
	static void HandleGetCommon(GtkSelectionData *data, const char *text,
		uint32 length, uint info);

	enum ReceivedSelectionKind {
		kCText,
		kString,
		kUnknown
	};

	static void HandleReceivedCommon(GtkSelectionData *, String &, 
		ReceivedSelectionKind);

private:
	int Clear(GdkEventSelection *);

	void StartSelectionSteal();
	void StopSelectionSteal();
	static gboolean SelectionStealBinder(void *);
	void SelectionSteal();

	void ConvertOneSelectionType(const char *);

	void ClearPrivate();
	void SetPrivate(const char *, int32);
	
	GtkWidget *fOffscreenWidget;
	String fTopClipboard;
	uint32 fSelectionStealCallback;
	bool fSelectionOwner;

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
