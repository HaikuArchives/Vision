// BWindow-like class
//
// see License at the end of file

#include "PlatformDefines.h"

#include <gdk/gdkx.h>
#include <gtk/gtkfixed.h>

#include "Application.h"
#include "Message.h"
#include "View.h"
#include "Window.h"
#include "Menu.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdio.h>
#include <glib.h>

extern "C" {

// lightweight override of GtkWindow, so that we can replace
// virtuals without affecting all the other instances of gtk window that we may use
#define GTK_TYPE_FWINDOW_BASE (gtk_fwindow_base_get_type ())
#define GTK_FWINDOW_BASE(obj) (GTK_CHECK_CAST (obj, GTK_TYPE_FWINDOW_BASE, GtkFWindowBase))
#define GTK_FWINDOW_BASE_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_FWINDOW_BASE, GtkFWindowBase))
#define GTK_IS_FWINDOW_BASE(obj) (GTK_CHECK_TYPE (obj, GTK_TYPE_FWINDOW_BASE))
#define GTK_IS_FWINDOW_BASE_CLASS(klass) (GTK_CHECK_CLASS_TYPE (klass, GTK_TYPE_FWINDOW_BASE))

struct GtkFWindowBase {
	GtkWindow window;
};

struct GtkFWindowBaseClass {
	GtkWindowClass parent_class;
};

static GtkWindowClass *parent_class = NULL;

static void gtk_fwindow_base_class_init (GtkFWindowBaseClass *);
static void gtk_fwindow_base_init (GtkFWindowBase *);

static GtkType
gtk_fwindow_base_get_type()
{
	static GtkType fwindow_base_type = 0;
	
	if (!fwindow_base_type) {
		static const GtkTypeInfo fwindow_base_info = {
			"GtkFWindowBase",
			sizeof (GtkFWindowBase),
			sizeof (GtkFWindowBaseClass),
			(GtkClassInitFunc)gtk_fwindow_base_class_init,
			(GtkObjectInitFunc)gtk_fwindow_base_init,
			NULL, NULL, (GtkClassInitFunc) NULL
		};
	
		fwindow_base_type = gtk_type_unique (gtk_window_get_type (),
			&fwindow_base_info);
	}
	
	return fwindow_base_type;
}


static void
gtk_fwindow_base_class_init (GtkFWindowBaseClass *)
{
	parent_class = (GtkWindowClass *)gtk_type_class(gtk_window_get_type ());
}

static void
gtk_fwindow_base_init (GtkFWindowBase *object)
{
	GTK_WINDOW(object)->type = GTK_WINDOW_TOPLEVEL;
}

}


static void
GtkFixedSizeAllocate(GtkWidget *widget, GtkAllocation *allocation)
{
	GtkFixed *fixed = GTK_FIXED(widget);

	FPoint growDelta;
	
	growDelta.x = allocation->width - widget->allocation.width;
	growDelta.y = allocation->height - widget->allocation.height;

	widget->allocation = *allocation;
	// resize the fixed background itself
	if (GTK_WIDGET_REALIZED(widget))
		gdk_window_move_resize (widget->window,
			allocation->x, allocation->y, 
			allocation->width, allocation->height);

	for (GList *p = fixed->children; p != NULL; p = p->next) {
		GtkFixedChild *child = (GtkFixedChild *)p->data;
		GtkWidget *widget = child->widget;
		if (GTK_WIDGET_VISIBLE (widget)) {

			// ToDo:
			// fetch the FView that wraps widget
			// based on it's follow flags, move it to respond to the
			// new size
			if (GTK_IS_MENU_BAR(widget)) {
				// For now just detect a menu bar and don't resize it
				GtkRequisition child_requisition;
				GtkAllocation child_allocation;
				gtk_widget_get_child_requisition(widget, &child_requisition);
				// vertically
				child_allocation.x = child->x;
				child_allocation.y = child->y;
				child_allocation.height = child_requisition.height;
				child_allocation.width = allocation->width - child->x;
				gtk_widget_size_allocate(widget, &child_allocation);

//	printf(" menu bar size allocate req %d %d alloc %d %d\n",
//		child_requisition.width, child_requisition.height,
//		allocation->width - child->x, child_requisition.height);
		
				widget->requisition.width = child_allocation.width;
			} else 
				FView::FixedWidgetSizeAllocate(child->x, child->y, widget, 
					allocation, growDelta);

		}
	}
}

// ToDo:
// get rid of fTopLevelView

FWindow::FWindow(FRect frame, const char *title, window_type type, uint32 flags) 
	:	ObjectGlue<GtkWindow>(GTK_WINDOW(gtk_object_new(gtk_fwindow_base_get_type(), 0))),
		fInitialFrame(frame),
		fTopLevelView(NULL),
		fMenuBar(NULL),
		fActive(false),
		fInitialAllocate(false),
		fMinX(10),
		fMinY(10),
		fMaxX(10000),
		fMaxY(10000),
		fMaxZoomWidth(10000),
		fMaxZoomHeight(10000),
		fPulseRate(1000000)
{
	// convert window_type to window_look/window_feel pair
	window_look look = B_DOCUMENT_WINDOW_LOOK;
	window_feel feel = B_NORMAL_WINDOW_FEEL;
	
	if (type == B_MODAL_WINDOW) {
		look = B_MODAL_WINDOW_LOOK;
		feel = B_MODAL_APP_WINDOW_FEEL;
	}
		
	FWindowCommon(title, look, feel, flags);
}

static GtkWindowType
GtkWindowKind(window_look look, window_feel)
{
	if (look == B_BORDERED_WINDOW_LOOK)
		// make the parent window not loose focus
		return GTK_WINDOW_POPUP;
	
	return GTK_WINDOW_TOPLEVEL;
}

FWindow::FWindow(FRect frame, const char *title, window_look look,
	window_feel feel, uint32 flags)
	:	ObjectGlue<GtkWindow>(GTK_WINDOW(gtk_object_new(gtk_fwindow_base_get_type(), 0))),
		fInitialFrame(frame),
		fTopLevelView(NULL),
		fMenuBar(NULL),
		fActive(false),
		fInitialAllocate(false),
		fPulseRate(1000000)
{
	GTK_WINDOW(fObject)->type = GtkWindowKind(look, feel);
	FWindowCommon(title, look, feel, flags);
}

void
FWindow::FWindowCommon(const char *title, window_look look, window_feel feel,
	uint32 flags)
{
	// initialize gtk object
	InitializeClass();
	
	fInitialLook = look;
	fInitialFeel = feel;

	//  We add a GtkFixed layout widget to the box so we can emulate 
	//  the behavior of a BWindow.
	fFixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(GtkObject()), GTK_WIDGET(fFixed));

	gtk_widget_set_usize(GTK_WIDGET(GtkObject()), (int)fInitialFrame.Width(),
		(int)fInitialFrame.Height());
	gtk_widget_set_uposition(GTK_WIDGET(GtkObject()), (int)fInitialFrame.left,
		(int)fInitialFrame.top);
	
	gtk_window_set_policy(GtkObject(), true, true, false);
		// shrink, grow, don't autoshrink
	SetTitle(title);
	SetFlags(flags);

	gtk_signal_connect (fObject, "realize", GTK_SIGNAL_FUNC(&FWindow::RealizeHook),
		this);
	
	be_app->WindowAdded(this);

	fIdleHandler = gtk_timeout_add(200, &FWindow::PulseDispatcher, this);
}

FWindow::~FWindow() 
{
	gtk_timeout_remove (fIdleHandler);
	if (fInitialFeel == B_MODAL_APP_WINDOW_FEEL || fInitialFeel == B_FLOATING_ALL_WINDOW_FEEL) {
		gdk_pointer_ungrab (GDK_CURRENT_TIME);
		gdk_keyboard_ungrab (GDK_CURRENT_TIME);
		gtk_grab_remove (AsGtkWidget());
	}
	be_app->WindowRemoved(this);
}

void
FWindow::TopLeveViewMoveTo(FView *view, FPoint to)
{
	// this is needed to update the GtkFixed structures when a
	// FView::Move moves a view that has already been attached.
	// Withou this the next window SizeAllocate moves the view to
	// it's old location.
	GtkWidget *widget = view->AsGtkWidget();

	for (GList *p = GTK_FIXED(fFixed)->children; p != NULL; p = p->next) {
		GtkFixedChild *child = (GtkFixedChild *)p->data;
		if (child->widget == widget) {
			child->x = (int)to.x;
			child->y = (int)to.y;
		}
	}
}

void 
FWindow::SetFlags(uint32 flags)
{
	gtk_window_set_policy(GtkObject(), (flags & B_NOT_RESIZABLE) == 0,
		(flags & B_NOT_RESIZABLE) == 0, true);
	// B_NOT_ZOOMABLE
}

void 
FWindow::Quit()
{
	FLooper::Quit();
}

void 
FWindow::Show()
{
	gtk_widget_show_all(AsGtkWidget());
}

void
FWindow::Hide()
{
	gtk_widget_hide_all(AsGtkWidget());
}

bool 
FWindow::IsHidden() const
{
	return (fObject->flags & GTK_VISIBLE) == 0;
}

bool
FWindow::IsActive() const
{
	return fActive;
}

void 
FWindow::Activate(bool on)
{
	if (on) {
		gdk_window_show(AsGtkWidget()->window);

		// doesn't seem to be a better way to do this without
		// an xlib call
		gdk_error_trap_push ();
		XSetInputFocus (GDK_DISPLAY (),
		      GDK_WINDOW_XWINDOW (AsGtkWidget()->window),
		      RevertToPointerRoot, 
		      GDK_CURRENT_TIME);
		gdk_flush();
		gdk_error_trap_pop();
	}
}

void 
FWindow::RealizeHook(GtkWidget *, FWindow *window)
{
	window->fInitialAllocate = true;
	switch (window->fInitialLook) {
		case B_BORDERED_WINDOW_LOOK:
			// plain vanilla window without any decor
			gdk_window_set_decorations(GTK_WIDGET(window->GtkObject())->window,
				(GdkWMDecoration)0);
			break;

		case B_MODAL_WINDOW_LOOK:
			// a lot of window manager themes don't support this mode
			gdk_window_set_decorations(GTK_WIDGET(window->GtkObject())->window,
				GDK_DECOR_BORDER);
			break;

		default:
			break;
	}
	
	switch (window->fInitialFeel) {
		case B_FLOATING_ALL_WINDOW_FEEL:
		case B_MODAL_APP_WINDOW_FEEL:
			gdk_pointer_grab(window->AsGtkWidget()->window, true,
				(GdkEventMask)(GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
				GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK |
				GDK_POINTER_MOTION_MASK),
				NULL, NULL, GDK_CURRENT_TIME);
			gdk_keyboard_grab(window->AsGtkWidget()->window, TRUE,
			      GDK_CURRENT_TIME);
			gtk_grab_add(window->AsGtkWidget());
			break;

		default:
			break;
	}
}

void
FWindow::AddChild(FMenuBar *menuBar)
{	
	fTopLevelView = NULL;
		// for now, since FMenuBar for now does not inherit from FView
	fMenuBar = menuBar;

	gtk_widget_set_usize(GTK_WIDGET(menuBar->GtkObject()), 0,
		(int)menuBar->Frame().Height());		
	gtk_fixed_put(GTK_FIXED(fFixed), GTK_WIDGET(menuBar->GtkObject()), 0, 0);
	menuBar->AddedToWindow(this);
}

void
FWindow::AddChild(FView *view)
{	
	fTopLevelView = view;
	gtk_fixed_put(GTK_FIXED(fFixed), view->AsGtkWidget(), (int)view->Frame().left, 
		(int)view->Frame().top);
	gtk_widget_set_usize(view->AsGtkWidget(), (int)view->Frame().Width(), 
		(int)view->Frame().Height());
	view->AddToWindow(this);

	if (!IsHidden())
		gtk_widget_show_all(view->AsGtkWidget());
}

void
FWindow::RemoveChild(FView *view)
{
	if (fTopLevelView == view)
		// ToDo: this is broken
		fTopLevelView = NULL;
	gtk_container_remove (GTK_CONTAINER(fFixed), view->AsGtkWidget());
//	ASSERT(view->AsGtkWidget()->parent == NULL);
}

FMenuBar *
FWindow::KeyMenuBar() const
{
	return fMenuBar;
}


FView *
FWindow::FindView(const char *name) const
{
	ASSERT(fTopLevelView);
	return fTopLevelView->FindView(name);
}

FView *
FWindow::ChildAt(int32 index) const
{
	if (index == 0)
		return fTopLevelView;

	ASSERT(!"implement me");
	return NULL;
}

void 
FWindow::WindowActivated(bool)
{
}

const char *
FWindow::Title() const
{
	return GtkObject()->title;
}

void 
FWindow::SetTitle(const char *title)
{
	gtk_window_set_title(GtkObject(), title);
}	

void 
FWindow::MoveTo(float x, float y)
{
	fInitialFrame.OffsetTo(x, y);
	if (!fInitialAllocate) 
		gtk_widget_set_uposition(GTK_WIDGET(GtkObject()), (int)fInitialFrame.left,
			(int)fInitialFrame.top);
	else
		gtk_window_reposition(GtkObject(), (int)x, (int)y);
}

void 
FWindow::MoveTo(FPoint where)
{
	MoveTo(where.x, where.y);
}

void 
FWindow::ResizeTo(float width, float height)
{
	//FPoint leftTop(Frame().LeftTop());

	fInitialFrame.SetWidth(width);
	fInitialFrame.SetHeight(height);
	gtk_widget_set_usize(GTK_WIDGET(GtkObject()), (int)width, (int)height);
	// Sometimes doing a set_usize will move the window origin too. Force
	// it back where it was.
	//gtk_window_reposition(GtkObject(), (int)leftTop.x, (int)leftTop.y);
}


BRect 
FWindow::Frame() const
{
	int x, y, w, h;

	if (!AsGtkWidget()->window)
		return fInitialFrame;

	gdk_window_get_root_origin(AsGtkWidget()->window, &x, &y);
	gdk_window_get_size(AsGtkWidget()->window, &w, &h);
	
	return BRect(x, y, x + w, y + h);
}

BRect 
FWindow::Bounds() const
{
	int w, h;

	if (!AsGtkWidget()->window)
		return BRect(0, 0, fInitialFrame.Width(), fInitialFrame.Height());

	gdk_window_get_size(AsGtkWidget()->window, &w, &h);
	
	return BRect(0, 0, w, h);
}

FPoint 
FWindow::ConvertToScreen(FPoint point) const
{
	return point + Frame().LeftTop();
}

FPoint 
FWindow::ConvertFromScreen(FPoint point) const
{
	return point - Frame().LeftTop();
}

void 
FWindow::DispatchMessage(FMessage *message, FHandler *handler)
{
	char bytes[256];
	int32 length;
	FView *view;
	
	switch (message->what) {
		case B_KEY_DOWN:
			view = dynamic_cast<FView *>(handler);
			if (!view) {
				// keyboard events going to a non-view, don't know
				// how to deal with that
				inherited_::DispatchMessage(message, handler);
				break;
			}
			// transform "byte" array into a C string
			for (length = 0; ; length++) {
				if (message->FindInt8("byte", length, 
					(int8 *)&bytes[length]) != B_OK)
					break;
			}
			if (!length)
				break;

			bytes[length + 1] = '\0';
			view->KeyDown(bytes, length);
			break;

		case B_PULSE:
			view = dynamic_cast<FView *>(handler);
			if (view)
				view->Pulse();
			break;
			
		default:
			inherited_::DispatchMessage(message, handler);
			break;
	}
}

void 
FWindow::SetPulseRate(bigtime_t rate)
{
	fPulseRate = rate;
}

bigtime_t 
FWindow::PulseRate() const
{
	return fPulseRate;
}


void 
FWindow::InitializeClass()
{
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->delete_event = &FWindow::DeleteEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->focus_in_event = &FWindow::FocusInEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->focus_out_event = &FWindow::FocusOutEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->size_allocate = &FWindow::SizeAllocateBinder;
}

int 
FWindow::DeleteEventBinder(GtkWidget *widget, GdkEventAny *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->DeleteEvent(event);
}

bool 
FWindow::QuitRequested()
{
	return true;
}

void 
FWindow::MessageReceived(FMessage *message)
{
	switch (message->what) {
		case B_QUIT_REQUESTED:
			if (QuitRequested())
				Quit();
			break;
		default:
			inherited_::MessageReceived(message);
			break;
	}
}

void 
FWindow::FrameResized(float, float)
{
}


int 
FWindow::FocusInEvent(GdkEventFocus *)
{
	if (IsHidden()) 
		// spurious callback
		return false;
		
	fActive = true;
	WindowActivated(true);

	if (GtkObject()->focus_widget 
		&& GtkObject()->focus_widget != AsGtkWidget() 
		&& (((::GtkObject *)GtkObject()->focus_widget)->flags & GTK_HAS_FOCUS) == 0) {
		GdkEventFocus focusEvent;

		focusEvent.type = GDK_FOCUS_CHANGE;
		focusEvent.window = GtkObject()->focus_widget->window;
		focusEvent.in = true;

		gtk_widget_event (GtkObject()->focus_widget, (GdkEvent *)&focusEvent);
	}
	return false;
}

int 
FWindow::FocusOutEvent(GdkEventFocus *)
{
	fActive = false;
	WindowActivated(false);

	if (GtkObject()->focus_widget 
		&& GtkObject()->focus_widget != AsGtkWidget() 
		&& (((::GtkObject *)GtkObject()->focus_widget)->flags & GTK_HAS_FOCUS) != 0) {

		GdkEventFocus focusEvent;
		focusEvent.type = GDK_FOCUS_CHANGE;
		focusEvent.window = GtkObject()->focus_widget->window;
		focusEvent.in = false;

		gtk_widget_event (GtkObject()->focus_widget, (GdkEvent*) &focusEvent);
    }
    return false;
}

void 
FWindow::GetSizeLimits(float *minx, float *maxx, float *miny, float *maxy)
{
	*minx = fMinX;
	*miny = fMinY;
	*maxx = fMaxX;
	*maxy = fMaxY;
}

void 
FWindow::SetSizeLimits(float minx, float maxx, float miny, float maxy)
{
	fMinX = minx;
	fMinY = miny;
	fMaxX = maxx;
	fMaxY = maxy;
	
	GdkGeometry geometry;
	geometry.min_width = (int)minx;
	geometry.min_height = (int)miny;
	geometry.max_width = (int)maxx;
	geometry.max_height = (int)maxy;

	gtk_window_set_geometry_hints(GTK_WINDOW(AsGtkWidget()), NULL, &geometry,
		(GdkWindowHints)(GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE));
}

void 
FWindow::SetZoomLimits(float width, float height)
{
	fMaxZoomWidth = width;
	fMaxZoomHeight = height;
}

void 
FWindow::SizeAllocate(GtkAllocation *allocation)
{	
	AsGtkWidget()->allocation = *allocation;	
	
	if (((struct _GtkObject *)fFixed)->flags & GTK_VISIBLE) {
		GtkAllocation fixedAllocation;
		fixedAllocation.x = GTK_CONTAINER(AsGtkWidget())->border_width;
		fixedAllocation.y = GTK_CONTAINER(AsGtkWidget())->border_width;
		fixedAllocation.width = allocation->width - fixedAllocation.x * 2;
		fixedAllocation.height = allocation->height - fixedAllocation.y * 2;

		if (fixedAllocation.width < 1)
			fixedAllocation.width = 1;
		if (fixedAllocation.height < 1)
			fixedAllocation.height = 1;

		GtkFixedSizeAllocate(fFixed, &fixedAllocation);
	}
	FrameResized(Bounds().Width(), Bounds().Height());
}

int 
FWindow::FocusInEventBinder(GtkWidget *widget, GdkEventFocus *event)
{
	if (!GnoBeObject(widget))
		return false;
	return GnoBeObject(widget)->FocusInEvent(event);
}

int 
FWindow::FocusOutEventBinder(GtkWidget *widget, GdkEventFocus *event)
{
	if (!GnoBeObject(widget))
		return false;
	return GnoBeObject(widget)->FocusOutEvent(event);
}

void 
FWindow::SizeAllocateBinder(GtkWidget *widget, GtkAllocation *allocation)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->SizeAllocate(allocation);
}

int 
FWindow::DeleteEvent (GdkEventAny *)
{
	if (!QuitRequested())
		// we don't want to quit yet.
		return 1;

	fDeletingSelf = true;
	Quit();
	fDeletingSelf = false;
	// don't delete anything here,
	// the gtk_object delete event will dispose the window properly
	return 0;
}

void 
FWindow::Close()
{
	PostMessage(B_QUIT_REQUESTED, this);
}

gboolean 
FWindow::PulseDispatcher(void *castToThis)
{
	FWindow *self = (FWindow *)castToThis;
	
	if (FLooper::IsValid(self)) 
		self->MenusBeginning();

	// probably don't need to remove the handler here, should be expired
	gtk_timeout_remove (self->fIdleHandler);
	self->fIdleHandler = gtk_timeout_add(200, &FWindow::PulseDispatcher, self);

	return false;
}

void 
FWindow::MenusBeginning()
{
	// empty
}

//--------------
//
//// gnome_win_hints_set_workspace

// gnome-core/applets/deskguide/gwmh.c
//


/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler
Copyright (c) 1999-2001, Gene Ragan

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
