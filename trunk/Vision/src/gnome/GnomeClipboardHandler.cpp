// Gnome-compatible clipboard handling.
//
// see License at the end of file

#include "GnomeClipboardHandler.h"
#include <gtk/gtkselection.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>
#include <gdk/gdk.h>
#include <gdk/gdktypes.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <gdk/gdki18n.h>
#include <X11/Xatom.h>

#include <stdio.h>

#undef CLIPBOARD_DEBUG

extern "C" {
#define GTK_TYPE_SELECTION_HANDLER (gtk_selection_handler_get_type ())
#define GTK_SELECTION_HANDLER(obj) (GTK_CHECK_CAST (obj, GTK_TYPE_SELECTION_HANDLER, GtkSelectionHandler))
#define GTK_SELECTION_HANDLER_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_SELECTION_HANDLER, GtkSelectionHandlerClass))
#define GTK_IS_SELECTION_HANDLER(obj) (GTK_CHECK_TYPE (obj, GTK_TYPE_SELECTION_HANDLER))
#define GTK_IS_SELECTION_HANDLER_CLASS(klass) (GTK_CHECK_CLASS_TYPE (klass, GTK_TYPE_SELECTION_HANDLER))

struct GtkSelectionHandler {
	GtkWindow widget;
	GnomeClipboardHandler *handler;
};

struct GtkSelectionHandlerClass {
	GtkWindowClass parent_class;
};

static GtkWidgetClass *parent_class = NULL;
static GdkAtom clipboardAtom = GDK_NONE;

static void gtk_selection_handler_class_init (GtkSelectionHandlerClass *);
static void gtk_selection_handler_init (GtkSelectionHandler *);


static GtkType
gtk_selection_handler_get_type()
{
	static GtkType selection_handler_type = 0;
	
	if (!selection_handler_type) {
		static const GtkTypeInfo selection_handler_info = {
			"GtkSelectionHandler",
			sizeof (GtkSelectionHandler),
			sizeof (GtkSelectionHandlerClass),
			(GtkClassInitFunc) gtk_selection_handler_class_init,
			(GtkObjectInitFunc) gtk_selection_handler_init,
			NULL, NULL, (GtkClassInitFunc) NULL
		};
	
		selection_handler_type = gtk_type_unique (gtk_window_get_type (),
			&selection_handler_info);
	}
	
	return selection_handler_type;
}


static void
gtk_selection_handler_class_init (GtkSelectionHandlerClass *klass)
{
	parent_class = (GtkWidgetClass *)gtk_type_class(gtk_window_get_type ());

	GtkWidgetClass *widget_class = (GtkWidgetClass*)klass;
	widget_class->selection_clear_event = &GnomeClipboardHandler::ClearBinder;
	widget_class->selection_received = GnomeClipboardHandler::Received;
	widget_class->selection_get = &GnomeClipboardHandler::Get;
}


static void
gtk_selection_handler_init (GtkSelectionHandler *selectionHandler)
{
	clipboardAtom = gdk_atom_intern("CLIPBOARD", false);
	GnomeClipboardHandler::InitSelectionCommon(GTK_WIDGET(selectionHandler), 
		clipboardAtom);
}

static GtkWidget *
gtk_selection_handler_new()
{
	GtkWidget *result = (GtkWidget *)gtk_type_new(GTK_TYPE_SELECTION_HANDLER);
	// make us a popup so that we don't show up in the panel, etc.
	GTK_WINDOW(result)->type = GTK_WINDOW_POPUP;
	return result;
}

}

GnomeClipboardHandler::GnomeClipboardHandler()
	:	fOffscreenWidget(gtk_selection_handler_new()),
		fSelectionStealCallback(0),
		fSelectionOwner(false)
{
	// allocate an offscreen widget
	gtk_widget_set_uposition(fOffscreenWidget, 1, 1);
	gtk_widget_set_usize(fOffscreenWidget, 1, 1);
	gtk_widget_show_all(fOffscreenWidget);
	gtk_window_reposition(GTK_WINDOW(fOffscreenWidget), -10000, -1000);

	// ToDo: 
	// make the window not show up in the panel
	GTK_SELECTION_HANDLER(fOffscreenWidget)->handler = this;
	StartSelectionSteal();
}

GnomeClipboardHandler::~GnomeClipboardHandler()
{
	StopSelectionSteal();
}

enum {
	kTargetString,
	kTargetText,
	kTargetCompoundText
};

const GtkTargetEntry kTargetTypes[] = {
	{ "STRING", 0, kTargetString },
	{ "TEXT", 0, kTargetText }, 
	{ "text/plain", 0, kTargetText }, 
	{ "COMPOUND_TEXT", 0, kTargetCompoundText }
};

void 
GnomeClipboardHandler::InitSelectionCommon(GtkWidget *widget, GdkAtom clipboardType)
{
	gtk_selection_add_targets(widget, clipboardType,
		kTargetTypes, sizeof(kTargetTypes) / sizeof(GtkTargetEntry));
}

int
GnomeClipboardHandler::ClearBinder(GtkWidget *widget, GdkEventSelection *event)
{	
	g_return_val_if_fail(widget != NULL, false);
	g_return_val_if_fail(GTK_IS_SELECTION_HANDLER (widget), false);
	g_return_val_if_fail(event != NULL, false);

	return GTK_SELECTION_HANDLER(widget)->handler->Clear(event);
}

int
GnomeClipboardHandler::Clear(GdkEventSelection *event)
{	
#ifdef CLIPBOARD_DEBUG
printf("selection clear\n");
#endif
	g_assert(fOffscreenWidget);

	if (!gtk_selection_clear(fOffscreenWidget, event))
		return false;
		
	if (event->selection == clipboardAtom) {
		fSelectionOwner = false;
		ClearPrivate();
		StartSelectionSteal();
		// set up a callback that will try to grab the selection
		// and steal it for us.
	}
	
	return true;
}

void 
GnomeClipboardHandler::StartSelectionSteal()
{
	fSelectionStealCallback = gtk_timeout_add(100, 
		&GnomeClipboardHandler::SelectionStealBinder, this);
}

void 
GnomeClipboardHandler::StopSelectionSteal()
{
	if (fSelectionStealCallback)
		gtk_timeout_remove(fSelectionStealCallback);
	fSelectionStealCallback = 0;
}


gboolean 
GnomeClipboardHandler::SelectionStealBinder(void *castToThis)
{
	GnomeClipboardHandler *self = (GnomeClipboardHandler *)castToThis;
	self->SelectionSteal();
	gtk_timeout_remove(self->fSelectionStealCallback);
	self->fSelectionStealCallback = gtk_timeout_add(1000,
		&GnomeClipboardHandler::SelectionStealBinder, self);
	
	return false;
}

void 
GnomeClipboardHandler::ConvertOneSelectionType(const char *type)
{
	gtk_selection_convert(fOffscreenWidget, clipboardAtom,
		gdk_atom_intern(type, false), GDK_CURRENT_TIME);
}

void 
GnomeClipboardHandler::SelectionSteal()
{
#ifdef CLIPBOARD_DEBUG
printf("trying to grab selection\n");
#endif
	ConvertOneSelectionType("COMPOUND_TEXT");
}


void 
GnomeClipboardHandler::HandleGetCommon(GtkSelectionData *data, const char *text,
	uint32 length, uint /*info*/)
{
	gtk_selection_data_set (data, GDK_SELECTION_TYPE_STRING,
		8, (const uchar *)text, length);
#if 0
	// this doesn't work with jcc
	if (info == kTargetString) {
		gtk_selection_data_set (data, GDK_SELECTION_TYPE_STRING,
			8, (const uchar *)text, length);
	} else if (info == kTargetText) {
		gtk_selection_data_set (data, gdk_atom_intern("TEXT", false),
			8, (const uchar *)text, length);
	} else if (info == kTargetCompoundText) {
		uchar *compoundText;
		GdkAtom encoding;
		int format;
		int newLength;
			
		gdk_string_to_compound_text(text, &encoding, &format, &compoundText, &newLength);
		gtk_selection_data_set(data, encoding, format, compoundText, newLength);
		gdk_free_compound_text(compoundText);
	}
#endif
}

void
GnomeClipboardHandler::Get(GtkWidget *widget, GtkSelectionData *data, 
	uint info, uint)
{	
	g_return_if_fail(widget != NULL);
	g_return_if_fail(GTK_IS_SELECTION_HANDLER (widget));
	

#ifdef CLIPBOARD_DEBUG
	printf("selection get\n");
#endif

	if (data->selection == GDK_SELECTION_PRIMARY) {
		TRESPASS();
		return;
	}
	
	HandleGetCommon(data, GTK_SELECTION_HANDLER(widget)->handler->Text(), 
		GTK_SELECTION_HANDLER(widget)->handler->Length(), info);
}

void 
GnomeClipboardHandler::HandleReceivedCommon(GtkSelectionData *data, 
	String &result, ReceivedSelectionKind kind)
{
	switch (kind) {
		case kString:
			// become the selection owner - we just turn around and offer
			// the selection we got as ours
			result.Assign((const char *)data->data, data->length);
			break;

		case kCText:
			{
				char **textRuns;	

				int count = gdk_text_property_to_text_list (data->type,
					data->format, data->data, data->length,
					&textRuns);
				
				for (int index = 0; index < count; index++)
					result += textRuns[index];


				if (count > 0)
					gdk_free_text_list(textRuns);
			}
			break;

		default:
			break;
	}
}

void
GnomeClipboardHandler::Received(GtkWidget *widget, GtkSelectionData *data,
	guint time)
{

	ReceivedSelectionKind type;
	
#ifdef CLIPBOARD_DEBUG
	printf("selection received\n");
#endif

	if (data->selection != clipboardAtom) 
		// we only handle CLIPBOARD here
		return;

	if (data->type == GDK_TARGET_STRING) {
		type = kString;

#ifdef CLIPBOARD_DEBUG
		printf("string\n");
#endif

	} else if (data->type == gdk_atom_intern ("COMPOUND_TEXT", false)
		|| data->type == gdk_atom_intern ("TEXT", false)) {
		type = kCText;

#ifdef CLIPBOARD_DEBUG
	if (data->type == gdk_atom_intern ("COMPOUND_TEXT", false))
		printf("text\n");
	else
		printf("compound text\n");
#endif

	} else {

#ifdef CLIPBOARD_DEBUG
	printf("unknown\n");
#endif

		type = kUnknown;
	}

	if (type == kUnknown || data->length < 0) {
		if (data->target != GDK_TARGET_STRING)
			gtk_selection_convert (widget, data->selection,
				GDK_TARGET_STRING, time);

#ifdef CLIPBOARD_DEBUG
		printf("unknown selection format\n");
#endif

		return;
	}
	
	GTK_SELECTION_HANDLER(widget)->handler->StopSelectionSteal();
	
	String result;
	HandleReceivedCommon(data, result, type);

	// become the selection owner - we just turn around and offer
	// the selection we got as ours
	GTK_SELECTION_HANDLER(widget)->handler->Set(result.CStr(), 
		result.Length());
}

bool 
GnomeClipboardHandler::Empty() const
{
	return fTopClipboard.Length() == 0;
}

const char *
GnomeClipboardHandler::Text() const
{
	return fTopClipboard.CStr();
}

int32 
GnomeClipboardHandler::Length() const
{
	return fTopClipboard.Length();
}

void 
GnomeClipboardHandler::Set(const char *text, int32 length)
{
	StopSelectionSteal();
	SetPrivate(text, length);
	fSelectionOwner = true;
	
	if (gdk_selection_owner_get(clipboardAtom) != fOffscreenWidget->window)
		gtk_selection_owner_set(fOffscreenWidget, clipboardAtom, GDK_CURRENT_TIME);
}

void 
GnomeClipboardHandler::Clear()
{
	StopSelectionSteal();
	ClearPrivate();
	fSelectionOwner = true;
	
	gtk_selection_owner_set(NULL, clipboardAtom, GDK_CURRENT_TIME);
}


void 
GnomeClipboardHandler::SetPrivate(const char *text, int32 length)
{
	fTopClipboard.Assign(text, length);
}

void 
GnomeClipboardHandler::ClearPrivate()
{
	fTopClipboard = "";
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
