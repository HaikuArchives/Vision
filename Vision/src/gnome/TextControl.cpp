// BTextControl-like class
//
// see License at the end of file

#include "TextControl.h"
#include "Window.h"
#include "Message.h"

#include <gdk/gdkx.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkmain.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktext.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkhbox.h>

#include <stdio.h>

extern "C" {
#define GTK_TYPE_TEXT_CONTROL_BASE (gtk_text_control_base_get_type ())
#define GTK_TEXT_CONTROL_BASE(obj) (GTK_CHECK_CAST (obj, GTK_TYPE_TEXT_CONTROL_BASE, GtkSelectionHandler))
#define GTK_TEXT_CONTROL_BASE_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_TEXT_CONTROL_BASE, GtkSelectionHandlerClass))
#define GTK_IS_TEXT_CONTROL_BASE(obj) (GTK_CHECK_TYPE (obj, GTK_TYPE_TEXT_CONTROL_BASE))
#define GTK_IS_TEXT_CONTROL_BASE_CLASS(klass) (GTK_CHECK_CLASS_TYPE (klass, GTK_TYPE_TEXT_CONTROL_BASE))

struct GtkTextControlBase {
	GtkText text;
};

struct GtkTextControlBaseClass {
	GtkTextClass parent_class;
};

static GtkWidgetClass *parent_class = NULL;

static void gtk_text_control_base_class_init (GtkTextControlBaseClass *);
static void gtk_text_control_base_init (GtkTextControlBase *);

static GtkType
gtk_text_control_base_get_type()
{
	static GtkType text_control_base_type = 0;
	
	if (!text_control_base_type) {
		static const GtkTypeInfo text_control_base_info = {
			"GtkTextControlBase",
			sizeof (GtkTextControlBase),
			sizeof (GtkTextControlBaseClass),
			(GtkClassInitFunc)gtk_text_control_base_class_init,
			(GtkObjectInitFunc)gtk_text_control_base_init,
			NULL, NULL, (GtkClassInitFunc) NULL
		};
	
		text_control_base_type = gtk_type_unique (gtk_text_get_type (),
			&text_control_base_info);
	}
	
	return text_control_base_type;
}


static void
gtk_text_control_base_class_init (GtkTextControlBaseClass *klass)
{
	parent_class = (GtkWidgetClass *)gtk_type_class(gtk_text_get_type ());

	GtkWidgetClass *widget_class = (GtkWidgetClass *)klass;
	widget_class->key_press_event = &FTextControl::KeyPressHook;
}

static void
gtk_text_control_base_init (GtkTextControlBase *)
{
}

static GtkWidget *
gtk_text_control_base_new()
{
	return gtk_widget_new(GTK_TYPE_TEXT_CONTROL_BASE,
		"hadjustment", NULL, "vadjustment", NULL, NULL);
}

}

static FRect
TextControlFrameAdjustFromBeOS(FRect frame)
{
	frame.InsetBy(0, -1);
	return frame;
} 

FTextControl::FTextControl(FRect frame, const char *name, const char *label,
	const char *initialText, BMessage *message,
	uint32 resizeFlags, uint32 flags)
	:	FControl(gtk_hbox_new(true, 0),
			TextControlFrameAdjustFromBeOS(frame),
			name, label, message, resizeFlags, flags),
		fGetTextBuffer(NULL),
		fLabel(NULL),
		fModificationMessage(NULL)
{
	InitializeClass();


	GtkWidget *scrolledWindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledWindow),
		GTK_POLICY_NEVER, GTK_POLICY_NEVER);

	if (label && label[0]) {
		fLabel = gtk_label_new(label);
		gtk_box_pack_start(GTK_BOX(fObject), fLabel, false, false, 0);
		gtk_box_pack_end(GTK_BOX(fObject), scrolledWindow, true, true, 0);
	} else
		gtk_container_add (GTK_CONTAINER(fObject), scrolledWindow);

	fText = gtk_text_control_base_new();
	gtk_text_set_editable (GTK_TEXT(fText), true);
	gtk_container_add (GTK_CONTAINER (scrolledWindow), fText);

	// hacky way to force the font we want to take
	SetText("x");
	SetText(initialText ? initialText : "");

	gtk_signal_connect((struct _GtkObject *)fText, "changed",
		GTK_SIGNAL_FUNC(&FTextControl::ChangedBinder), this);
	
	SetFont(be_plain_font);
}

FTextControl::~FTextControl()
{
	g_free(fGetTextBuffer);
}

void 
FTextControl::SetText(const char *text)
{
	// delete existing text
	gtk_editable_delete_text(GTK_EDITABLE(fText), 0, TextLength());

	// insert new text
	Insert(text);
}

const char *
FTextControl::Text() const
{
	// gtk_editable_get_chars returns a copy of the text,
	// BTextControl doesn't. Need to cache here to support the
	// interface properly
	fGetTextBuffer = gtk_editable_get_chars(GTK_EDITABLE(fText), 0, -1);
	return fGetTextBuffer;
}

int32 
FTextControl::TextLength() const
{
	return gtk_text_get_length(GTK_TEXT(fText));
}

void 
FTextControl::Select(int32 start, int32 end)
{
	gtk_editable_select_region(GTK_EDITABLE(fText), start, end);
}

void 
FTextControl::Delete()
{
	gtk_editable_delete_selection(GTK_EDITABLE(fText));
	if (fModificationMessage)
		Invoke(fModificationMessage);
}

void 
FTextControl::Insert(const char *text)
{
	FFont font;
	GetFont(&font);
	gtk_text_insert(GTK_TEXT(fText), font.GdkObject(), NULL, NULL,
		text, strlen(text));

	g_free(fGetTextBuffer);
	fGetTextBuffer = NULL;
}

void 
FTextControl::ChangedBinder(GtkWidget *, FTextControl *self)
{
	if (self->fModificationMessage)
		self->Invoke(self->fModificationMessage);
}

void 
FTextControl::SetFont(const FFont *font, uint32 mode)
{
	inherited_::SetFont(font, mode);
	if (fLabel)
		FFont::Set(font, fLabel);		
	FFont::Set(font, fText);		
}

void 
FTextControl::SetDivider(float)
{
}

void 
FTextControl::SetModificationMessage(FMessage *message)
{
	delete fModificationMessage;
	fModificationMessage = message;
}

FMessage *
FTextControl::ModificationMessage() const
{
	return fModificationMessage;
}

void 
FTextControl::MakeFocus(bool on)
{
	if (on)
		gtk_widget_grab_focus(fText);
}

static gboolean
InvokeDefaultSoon(void *castToWindow)
{
	gtk_window_activate_default(GTK_WINDOW(castToWindow));
	return false;
}

int 
FTextControl::KeyPressHook(GtkWidget *widget, GdkEventKey *event) 
{
	FWindow *window;
	ASSERT(GnoBeObject(widget->parent->parent));
	FTextControl *self = GnoBeObject(widget->parent->parent);
	window = self->Window();

	ASSERT(window);

	if (event->keyval == GDK_Return) {
		GtkWidget *toplevel = gtk_widget_get_toplevel(
			self->AsGtkWidget());
		if (toplevel)
			gtk_idle_add(InvokeDefaultSoon, GTK_WINDOW(toplevel));
		return true;
	}

	if (event->keyval == GDK_Escape) {
		window->Close();
		return true;
	}

	// send the event through the message machinery to let
	// filters, etc. have their way with it, if needed
	return self->KeyPressEvent(event);
}

void 
FTextControl::KeyDown(const char *, int32)
{
	// got the message back no takers, just pass it on to the
	// widget
	GdkEventKey *event;
	if (Looper() && Looper()->CurrentMessage()
		&& Looper()->CurrentMessage()->FindPointer("gtk_event", (void **)&event) == B_OK)
		GTK_WIDGET_CLASS(parent_class)->key_press_event(fText, event);
}


void 
FTextControl::PrepareInitialSize(FRect rect)
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
