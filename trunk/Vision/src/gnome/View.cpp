// BView-like class
//
// see License at the end of file

#include "PlatformDefines.h"

#include "Bitmap.h"
#include "Region.h"
#include "ScrollView.h"
#include "ScrollBar.h"
#include "View.h"
#include "Window.h"
#include "Message.h"

#include <ctype.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <gdk/gdkrgb.h>
#include <gdk/gdkprivate.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <X11/Xlib.h>
#include <gtk/gtkfixed.h>
#include <typeinfo>

const int32 kMouseWheelScrollIncrement = 30;

extern "C" {
#define GTK_TYPE_FVIEW_BASE (gtk_fview_base_get_type ())
#define GTK_FVIEW_BASE(obj) (GTK_CHECK_CAST (obj, GTK_TYPE_FVIEW_BASE, GtkFViewBase))
#define GTK_FVIEW_BASE_CLASS(klass) (GTK_CHECK_CLASS_CAST (klass, GTK_TYPE_FVIEW_BASE, GtkFViewBaseClass))
#define GTK_IS_FVIEW_BASE(obj) (GTK_CHECK_TYPE (obj, GTK_TYPE_FVIEW_BASE))
#define GTK_IS_FVIEW_BASE_CLASS(klass) (GTK_CHECK_CLASS_TYPE (klass, GTK_TYPE_FVIEW_BASE))

struct GtkFViewBase {
	GtkContainer container;
};

struct GtkFViewBaseClass {
	GtkContainerClass parent_class;
};

static GtkWidgetClass *parent_class = NULL;

static void gtk_fview_base_class_init (GtkFViewBaseClass *);
static void gtk_fview_base_init (GtkFViewBase *);

static GtkType
gtk_fview_base_get_type()
{
	static GtkType fview_base_type = 0;
	
	if (!fview_base_type) {
		static const GtkTypeInfo fview_base_info = {
			"GtkFViewBase",
			sizeof (GtkFViewBase),
			sizeof (GtkFViewBaseClass),
			(GtkClassInitFunc)gtk_fview_base_class_init,
			(GtkObjectInitFunc)gtk_fview_base_init,
			NULL, NULL, (GtkClassInitFunc) NULL
		};
	
		fview_base_type = gtk_type_unique (gtk_container_get_type (),
			&fview_base_info);
	}
	
	return fview_base_type;
}


static void
gtk_fview_base_class_init (GtkFViewBaseClass *)
{
	parent_class = (GtkWidgetClass *)gtk_type_class(gtk_container_get_type ());
}

static void
gtk_fview_base_init (GtkFViewBase *)
{
}

}


FView::FView(FRect rect, const char *name, uint32 followFlags, uint32 flags)
	:	ObjectGlue<GtkWidget>(GTK_WIDGET(gtk_object_new(gtk_fview_base_get_type(), 0))),
		FHandler(name),
		fInitialRect(rect),
		fInitialAllocation(true),
		fFlags(flags),
		fFollowFlags(followFlags),
		fPenOffset(-1, -1),
		fForegroundGC(NULL),
		fBackgroundGC(NULL),
		fCopyGC(NULL),
		fInvertGC(NULL),
		fScratchGC(NULL),
		fClipRegion(NULL),
		fDrawingMode(B_OP_COPY),
		fGtkFocused(false),
		fEatNextMouseDown(false),
		fChildren(0, false)
{
	FViewCommon();
}

FView::FView(GtkWidget *widget, FRect rect, const char *name, uint32 followFlags, uint32 flags)
	:	ObjectGlue<GtkWidget>(GTK_WIDGET(widget)),
		FHandler(name),
		fInitialRect(rect),
		fInitialAllocation(true),
		fFlags(flags),
		fFollowFlags(followFlags),
		fPenOffset(-1, -1),
		fForegroundGC(NULL),
		fBackgroundGC(NULL),
		fCopyGC(NULL),
		fInvertGC(NULL),
		fScratchGC(NULL),
		fClipRegion(NULL),
		fDrawingMode(B_OP_COPY),
		fGtkFocused(false),
		fEatNextMouseDown(false),
		fChildren(0, false)
{
	FViewCommon();
}

void 
FView::FViewCommon()
{
	fObject->flags |= GTK_CAN_FOCUS;
	AsGtkWidget()->allocation.x = (int)fInitialRect.left;
	AsGtkWidget()->allocation.y = (int)fInitialRect.top;
	AsGtkWidget()->allocation.width = (int)fInitialRect.Width();
	AsGtkWidget()->allocation.height = (int)fInitialRect.Height();
	InitializeClass();
	gtk_signal_connect(fObject, "unmap", GTK_SIGNAL_FUNC(&FView::UnmapBinder),
		this);
	
	SetViewColor(Color(0, 0, 0));
	SetHighColor(Color(0, 0, 0));
	SetLowColor(Color(255, 255, 255));
}

FView::~FView()
{
	if (fFlags & B_PULSE_NEEDED) 
		gtk_timeout_remove (fIdleHandler);

	// uninstall keypress filters
	for (set<GdkWindow *>::iterator i = fInstalledFilters.begin();
		i != fInstalledFilters.end(); i++)
		gdk_window_remove_filter(*i, FView::KeyPressFilter, this);

	if (fCopyGC)
		gdk_gc_unref(fCopyGC);
	if (fBackgroundGC)
		gdk_gc_unref(fBackgroundGC);
	if (fInvertGC)
		gdk_gc_unref(fInvertGC);
	if (fScratchGC)
		gdk_gc_unref(fScratchGC);

	delete fClipRegion;
}

FRect 
FView::Frame() const
{
	return FRect(AsGtkWidget()->allocation.x, 
		AsGtkWidget()->allocation.y,
		AsGtkWidget()->allocation.x + AsGtkWidget()->allocation.width,
		AsGtkWidget()->allocation.y + AsGtkWidget()->allocation.height);
}

FRect 
FView::Bounds() const
{
	if (!IsRealized()) 
		return FRect(0, 0, fInitialRect.Width(), fInitialRect.Height()); 
	return FRect(0,	0, AsGtkWidget()->allocation.width, AsGtkWidget()->allocation.height);
}

uint32 
FView::ResizingMode() const
{
	return fFollowFlags;
}

uint32 
FView::Flags() const
{
	return fFlags;
}

rgb_color 
FView::ViewColor() const
{
	return fViewColor;
}

static GdkColor 
AsGdkColor(rgb_color color, const GtkWidget *widget)
{
	GdkColor result;
	GdkColormap *colormap = gtk_widget_get_colormap(const_cast<GtkWidget *>(widget));			

	result.red = ((gushort)color.red << 8) + (gushort)color.red;
	result.green = ((gushort)color.green << 8) + (gushort)color.green;
	result.blue = ((gushort)color.blue << 8) + (gushort)color.blue;

	gdk_color_alloc(colormap, &result);
	return result;
}

void 
FView::SetViewColor(rgb_color color)
{
	fViewColor = color;
	if (IsRealized()) {
		GdkColor color = AsGdkColor(fViewColor, AsGtkWidget());
		gdk_window_set_background(AsGtkWidget()->window, &color);
	}
}

void 
FView::SetViewColor(uchar r, uchar g, uchar b, uchar a)
{
	SetViewColor(Color(r, g, b, a));
}

rgb_color 
FView::LowColor() const
{
	return fLowColor;
}

void 
FView::SetLowColor(rgb_color color)
{
	fLowColor = color;
	if (IsRealized())
		gdk_rgb_gc_set_foreground(fBackgroundGC, color.AsGdkRgb());
}

void 
FView::SetLowColor(uchar r, uchar g, uchar b, uchar a)
{
	SetLowColor(Color(r, g, b, a));
}

rgb_color 
FView::HighColor() const
{
	return fHighColor;
}

void 
FView::SetHighColor(rgb_color color)
{
	fHighColor = color;
	if (IsRealized()) 
		gdk_rgb_gc_set_foreground(fCopyGC, color.AsGdkRgb());
}

void 
FView::SetHighColor(uchar r, uchar g, uchar b, uchar a)
{
	SetHighColor(Color(r, g, b, a));
}

void
FView::SetDrawingMode(drawing_mode mode)
{
	if (mode == fDrawingMode)
		return;

	switch (mode) {
		case B_OP_COPY:
		case B_OP_OVER:
			// for now pretending B_OP_OVER is just like
			// B_OP_COPY
			fForegroundGC = fCopyGC;
			break;

		case B_OP_INVERT:
			fForegroundGC = fInvertGC;
			break;

		case B_OP_ERASE:
		case B_OP_SELECT:
		case B_OP_ADD:
		case B_OP_SUBTRACT:
		case B_OP_BLEND:
		case B_OP_MIN:
		case B_OP_MAX:
		case B_OP_ALPHA:
			ASSERT(!"not implemented");
			break;
	}
	fDrawingMode = mode;
}

drawing_mode 
FView::DrawingMode() const
{
	return fDrawingMode;
}

inline FPoint
ViewAbsoluteLeftTop(const FView &view)
{
	ASSERT(view.Window());

	const FView *parent = view.Parent();
	if (!parent)
		return view.Window()->ConvertToScreen(view.Frame().LeftTop());

	return parent->ConvertToScreen(view.Frame().LeftTop());
}


FRect 
FView::ConvertToScreen(FRect rect) const
{
	rect.OffsetBy(ViewAbsoluteLeftTop(*this));
	return rect;
}

FRect 
FView::ConvertFromScreen(FRect rect) const
{
	FPoint delta = ViewAbsoluteLeftTop(*this);
	rect.OffsetBy(-delta.x, -delta.y);
	return rect;
}

FPoint 
FView::ConvertToScreen(FPoint point) const
{
	return point + ViewAbsoluteLeftTop(*this);
}

FPoint 
FView::ConvertFromScreen(FPoint point) const
{
	return point - ViewAbsoluteLeftTop(*this);
}

void 
FView::Realize()
{
	fObject->flags |= GTK_REALIZED;

	// According to Darin this needs to be done before using any
	// gdk_rgb calls and is trivial
	gdk_rgb_init();
	
	GdkWindowAttr attributes;

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = AsGtkWidget()->allocation.x;
	attributes.y = AsGtkWidget()->allocation.y;
	attributes.width = AsGtkWidget()->allocation.width;
	attributes.height = AsGtkWidget()->allocation.height;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.visual = gtk_widget_get_visual(AsGtkWidget());
	attributes.colormap = gtk_widget_get_colormap(AsGtkWidget());
	attributes.event_mask = gtk_widget_get_events(AsGtkWidget());
	attributes.event_mask |= 
		(GDK_EXPOSURE_MASK 
		| GDK_POINTER_MOTION_MASK
		| GDK_BUTTON_MOTION_MASK
		| GDK_BUTTON_PRESS_MASK 
		| GDK_BUTTON_RELEASE_MASK 
		| GDK_BUTTON_MOTION_MASK
		| GDK_ENTER_NOTIFY_MASK
		| GDK_LEAVE_NOTIFY_MASK
		| GDK_KEY_PRESS_MASK
		| GDK_KEY_RELEASE_MASK);


	AsGtkWidget()->window = gdk_window_new(
		gtk_widget_get_parent_window(AsGtkWidget()), 
		&attributes, 
		GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP);
	gdk_window_set_user_data (AsGtkWidget()->window, AsGtkWidget());

	fCopyGC = gdk_gc_new(AsGtkWidget()->window);
	fForegroundGC = fCopyGC;

	fBackgroundGC = gdk_gc_new(AsGtkWidget()->window);
	fInvertGC = gdk_gc_new(AsGtkWidget()->window);
	gdk_gc_set_function(fInvertGC, GDK_INVERT);

	if (fFlags & B_PULSE_NEEDED) {
		// using timeout here, gtk_idle_add seems to suck up a ton
		// of CPU
		bigtime_t pulseRate = 1000000;
		if (Window())
			pulseRate = Window()->PulseRate();
		fIdleHandler = gtk_timeout_add(pulseRate / 1000, &FView::PulseDispatcher, this);
	}

	// make the view color values propagate into the gcs we just set up
	SetLowColor(LowColor());
	SetHighColor(HighColor());

	GdkColor color = AsGdkColor(LowColor(), AsGtkWidget());
	gdk_window_set_background(AsGtkWidget()->window, &color);
	gdk_window_set_static_gravities(AsGtkWidget()->window, true);

	// set up a keypress filter so we can capture keycode values
	InstallKeypressFilterIfNeeded(AsGtkWidget()->window);

	AttachedToWindow();
	// would be nicer to call AttachedToWindow from AddedToWindow
	// we could maybe hook up the parent windows here instead?
}

void
FView::AddedToWindow(FWindow *window)
{
	ASSERT(fLooper == NULL);
	window->AddHandler(this);
}

void
FView::RemovedFromWindow(FWindow *window)
{
	ASSERT(fLooper != NULL);
	window->RemoveHandler(this);
}


void 
FView::AddViewsToWindow(FWindow *window)
{
	ASSERT(fLooper == NULL);
	AddedToWindow(window);
	EachListItem(&fChildren, &FView::AddedToWindow, window);
}

void 
FView::AddToWindow(FWindow *window)
{
	// ToDo:
	// once we get rid of gtkfixed as window background layout widget,
	// we need to call realized on this if the window is already showing
	AddViewsToWindow(window);
}

void 
FView::PrepareInitialSize(FRect rect)
{
	GtkWidget *widget = AsGtkWidget();
	
	widget->allocation.x = (int)rect.left;
	widget->allocation.y = (int)rect.top;
	widget->allocation.width = (int)rect.Width();
	widget->allocation.height = (int)rect.Height();
}

void 
FView::AddChild(FView *view)
{
	ASSERT(view->fLooper == NULL);
	
	fChildren.AddItem(view);

	GtkWidget *widget = view->AsGtkWidget();
	gtk_widget_set_parent (widget, AsGtkWidget());

	view->PrepareInitialSize(view->Frame());

	if (IsRealized())
		gtk_widget_realize(widget);
	
	if (IsVisible() && view->IsVisible()) {
		if (IsMapped())
			gtk_widget_map(widget);
		gtk_widget_queue_resize(AsGtkWidget());
	}

	if (fLooper) 
		// we are adding a view to a parent that is already in a window
		view->AddViewsToWindow(static_cast<FWindow *>(fLooper));
	
	if (IsVisible())
		gtk_widget_show_all(view->AsGtkWidget());	
}

void 
FView::RemoveSelf()
{
	FWindow *window = Window();

	if (Parent())
		Parent()->fChildren.RemoveItem(this);

	if (window) {
		EachListItem(&fChildren, &FView::RemovedFromWindow, window);
		RemovedFromWindow(window);
	}

	if (Parent())
		gtk_widget_unparent(AsGtkWidget());
	else if (window)
		window->RemoveChild(this);
}

void 
FView::MoveTo(FPoint newOrigin)
{
	FPoint delta = newOrigin - Frame().LeftTop();
	fInitialRect.left = newOrigin.x;
	fInitialRect.top = newOrigin.y;
	fInitialRect.right += delta.x;
	fInitialRect.bottom += delta.y;

	AsGtkWidget()->allocation.x = (int)fInitialRect.left;
	AsGtkWidget()->allocation.y = (int)fInitialRect.top;

	if (IsRealized()) {
		gdk_window_move_resize (AsGtkWidget()->window,
			AsGtkWidget()->allocation.x, AsGtkWidget()->allocation.y,
			AsGtkWidget()->allocation.width, AsGtkWidget()->allocation.height);
		if (!Parent())
			// top-level view
			Window()->TopLeveViewMoveTo(this, newOrigin);
	}
}

void 
FView::MoveBy(float x, float y)
{
	FPoint newOrigin = Frame().LeftTop();
	newOrigin.x += x;
	newOrigin.y += y;
	MoveTo(newOrigin);
}

void 
FView::ResizeTo(float w, float h)
{
	fInitialRect.right = fInitialRect.left + w;
	fInitialRect.bottom = fInitialRect.top + h;

	gtk_widget_set_usize(AsGtkWidget(), (int)w, (int)h);

	if (IsRealized()) {
		GtkAllocation allocation;
		allocation.x = (int)fInitialRect.left;
		allocation.y = (int)fInitialRect.top;
		allocation.width = (int)fInitialRect.Width();
		allocation.height = (int)fInitialRect.Height();
		gtk_widget_size_allocate(AsGtkWidget(), &allocation);
	} else {
		AsGtkWidget()->allocation.width = (int)w;
		AsGtkWidget()->allocation.height = (int)h;
	}
}


void 
FView::InstallKeypressFilterIfNeeded(GdkWindow *window)
{
	if (fInstalledFilters.find(window) == fInstalledFilters.end()) {
		fInstalledFilters.insert(window);
		gdk_window_add_filter(window, FView::KeyPressFilter, this);
	}
}

void 
FView::SetFont(const FFont *font, uint32)
{
	fFont = *font;
}

void 
FView::GetFont(FFont *font) const
{
	*font = fFont;
}

void
FView::SetFontSize(int32)
{
}

void 
FView::ConstrainClippingRegion(const FRegion *region)
{
	if (region == fClipRegion)
		return;

	delete fClipRegion;
	fClipRegion = NULL;

	if (region)
		fClipRegion = new FRegion(*region);
 
	if (!IsRealized())
		return;

	if (region) {
		GdkRectangle rect(region->Frame().GdkRect());
		rect.width += 1;
		rect.height += 1;
		gdk_gc_set_clip_rectangle(fCopyGC, &rect);
		gdk_gc_set_clip_rectangle(fBackgroundGC, &rect);
		gdk_gc_set_clip_rectangle(fInvertGC, &rect);
	} else {
		gdk_gc_set_clip_rectangle(fCopyGC, NULL);
		gdk_gc_set_clip_rectangle(fBackgroundGC, NULL);
		gdk_gc_set_clip_rectangle(fInvertGC, NULL);
	}
}

void 
FView::GetClippingRegion(FRegion *region)
{
	region->Set(Bounds());
}

void 
FView::FillRect(FRect rect, int32 mode)
{
	if (!IsRealized())
		return;

	// the following is correct, one has to add 1 to width and height to
	// get the same rectangle outline as StrokeRect. Apparently an X thing
	if (mode == B_SOLID_HIGH)
		gdk_draw_rectangle(AsGtkWidget()->window, 
			fForegroundGC, 1, (int)rect.left, (int)rect.top,
			(int)rect.Width() + 1, (int)rect.Height() + 1);
	else if (mode == B_SOLID_LOW)
		gdk_draw_rectangle(AsGtkWidget()->window,
			fBackgroundGC, 1, (int)rect.left, (int)rect.top,
			(int)rect.Width() + 1, (int)rect.Height() + 1);
}

void 
FView::StrokeRect(FRect rect)
{
	if (!IsRealized())
		return;

	gdk_draw_rectangle(AsGtkWidget()->window,
		fForegroundGC, 0, (int)rect.left, (int)rect.top,
		(int)rect.Width(), (int)rect.Height());
}

void 
FView::InvertRect(FRect rect)
{
	if (!IsRealized())
		return;

	gdk_draw_rectangle(AsGtkWidget()->window,
		fInvertGC, 1, (int)rect.left, (int)rect.top,
		(int)rect.Width() + 1, (int)rect.Height() + 1);
}

void 
FView::StrokePolygon(const BPoint *points, int32 count, bool)
{
	if (!IsRealized())
		return;

	GdkPoint *gdkPoints = new GdkPoint[count];
	for (int32 index = 0; index < count; index++) {
		gdkPoints[index].x = (int)points[index].x;
		gdkPoints[index].y = (int)points[index].y;
	}
	gdk_draw_polygon(AsGtkWidget()->window, fForegroundGC, 0, gdkPoints, count);

	delete [] gdkPoints;
}

void 
FView::StrokeLine(BPoint from, BPoint to, int32 )
{
	if (!IsRealized())
		return;

	gdk_draw_line(AsGtkWidget()->window, fForegroundGC,
		(int)from.x, (int)from.y, (int)to.x, (int)to.y);
}

void 
FView::BeginLineArray(int32)
{
}

void 
FView::AddLine(BPoint from, BPoint to, rgb_color color)
{
	if (!IsRealized())
		return;

	if (!fScratchGC)
		fScratchGC = gdk_gc_new(AsGtkWidget()->window);

	gdk_rgb_gc_set_foreground(fScratchGC, color.AsGdkRgb());

	gdk_draw_line(AsGtkWidget()->window, fScratchGC,
		(int)from.x, (int)from.y, (int)to.x, (int)to.y);
}

void 
FView::EndLineArray()
{
}

void 
FView::CopyBits(FRect from, FRect to)
{
	if (!IsRealized())
		return;

	gdk_draw_pixmap(AsGtkWidget()->window, fCopyGC, AsGtkWidget()->window,
		  (int)from.left, (int)from.top, (int)to.left, (int)to.top,
		  (int)from.Width(), (int)from.Height());
}

void 
FView::DrawBitmap(const FBitmap *bitmap, FRect from, FRect to)
{
	if (!IsRealized())
		return;

	from.right += 1;
	from.bottom += 1;
	to.right += 1;
	to.bottom += 1;
	bitmap->Draw(this, fClipRegion, from, to);
}

void 
FView::DrawBitmap(const FBitmap *bitmap)
{
	DrawBitmap(bitmap, bitmap->Bounds(), Bounds());
}

void 
FView::MovePenTo(FPoint to)
{
	fPenOffset = to;
}

void 
FView::MovePenTo(float x, float y)
{
	fPenOffset.x = x;
	fPenOffset.y = y;
}

void 
FView::DrawString(const char *text, int32 length)
{
	if (!IsRealized())
		return;

	gdk_draw_text(AsGtkWidget()->window, fFont.GdkObject(), fForegroundGC, 
		(int)fPenOffset.x, (int)fPenOffset.y, text, length);
}

void 
FView::DrawString(const char *text)
{
	DrawString(text, strlen(text));
}

float 
FView::StringWidth(const char *string) const
{
	return gdk_string_width(const_cast<FView *>(this)->fFont.GdkObject(), string);
}

static uint32
GdkToBeOSButtons(GdkModifierType buttons)
{
	uint32 result = 0;
	if (buttons & GDK_BUTTON1_MASK)
		result |= B_PRIMARY_MOUSE_BUTTON;
	if (buttons & GDK_BUTTON2_MASK)
		result |= B_SECONDARY_MOUSE_BUTTON;
	if (buttons & GDK_BUTTON3_MASK)
		result |= B_TERTIARY_MOUSE_BUTTON;

	return result;
}

void 
FView::GetMouse(FPoint *point, uint32 *buttons, bool)
{
	int x, y;
	GdkModifierType gdkButtons;
	gdk_window_get_pointer (AsGtkWidget()->window, &x, &y, &gdkButtons);
	point->x = x;
	point->y = y;
	*buttons = GdkToBeOSButtons(gdkButtons);
}

void 
FView::Invalidate(FRect rect)
{
	if (!IsRealized())
		return;

	FillRect(rect, B_SOLID_LOW);
	gtk_widget_queue_draw_area(AsGtkWidget(), (int)rect.left, (int)rect.top, 
		(int)rect.Width(), (int)rect.Height());
}

void 
FView::Invalidate()
{
	if (!IsRealized())
		return;

	FillRect(Bounds(), B_SOLID_LOW);
	gtk_widget_queue_draw(AsGtkWidget());
}

FWindow *
FView::Window() const
{
	if (!IsRealized() || !FLooper::IsValid(fLooper))
		// don't return a window until we get realized
		// (which calls AttachedToWindow)
		// 
		// Alternately, don't return a window if we are being
		// destroyed
		return NULL;

	return static_cast<FWindow *>(fLooper);
}

FView *
FView::Parent() const
{
	if (!AsGtkWidget()->parent)
		return NULL;

	return GnoBeObjectIfTypeMatches(AsGtkWidget()->parent);
}

FView *
FView::FindView(const char *name)
{
	if (strcmp(name, Name()) == 0)
		return this;
	
	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {
		FView *result = fChildren.ItemAt(index)->FindView(name);
		if (result)
			return result;
	}

	return NULL;
}

const FView *
FView::FindView(const char *name) const
{
	return const_cast<FView *>(const_cast<FView *>(this)->FindView(name));
}


gboolean 
FView::PulseDispatcher(void *castToThis)
{
	FView *self = (FView *)castToThis;
	
	if (self->Window()) {
		BMessage message(B_PULSE);
		self->Looper()->ReceiveMessage(&message, self);
	}

	// probably don't need to remove the handler here, should be expired
	gtk_timeout_remove (self->fIdleHandler);
	bigtime_t pulseRate = 1000000;
	if (self->Window())
		pulseRate = self->Window()->PulseRate();
	self->fIdleHandler = gtk_timeout_add(pulseRate / 1000, &FView::PulseDispatcher, self);

	return false;
}

void 
FView::MakeFocus(bool on)
{
	if (!fGtkFocused && on) {
		gtk_widget_grab_focus(AsGtkWidget());
		GtkWindow *window = GTK_WINDOW(gtk_widget_get_toplevel(AsGtkWidget()));
		if (window)
			gtk_window_set_focus (window, AsGtkWidget());
	}
}

bool 
FView::IsFocus() const
{
	return (fObject->flags & GTK_HAS_FOCUS) != 0;
}

int 
FView::FocusInEvent(GdkEventFocus *event)
{
	fObject->flags |= GTK_HAS_FOCUS;
	if (Window())
		gtk_widget_draw_focus(AsGtkWidget());

	// use the fGtkFocused flag to distinguish whether Gtk
	// already took care of focusing us
	fGtkFocused = true;
	if (Window())
		MakeFocus(true);
	fGtkFocused = false;
	fEatNextMouseDown = true;
	fEatNextMouseDownTime = system_time();
	InstallKeypressFilterIfNeeded(event->window);
	return false;
}

int 
FView::FocusOutEvent(GdkEventFocus *event)
{
	fObject->flags &= ~GTK_HAS_FOCUS;
	if (Window())
		gtk_widget_draw_focus(AsGtkWidget());
	fGtkFocused = true;
	if (Window())
		MakeFocus(false);
	fGtkFocused = false;
	InstallKeypressFilterIfNeeded(event->window);
	return false;
}


#if 0
// keyboard handling

static uint32
XToBeOSModifierKey(uint32 xKeyCode)
{
	switch (xKeyCode) {
		case kXLALT:
			return B_COMMAND_KEY | B_LEFT_COMMAND_KEY;
		case kXLCTL:
			return B_CONTROL_KEY | B_LEFT_CONTROL_KEY;
		case kXRCTL:
			return B_CONTROL_KEY | B_RIGHT_CONTROL_KEY;
		case kXRALT:
			return B_COMMAND_KEY | B_RIGHT_COMMAND_KEY;
		case kXLWIN:
			return B_OPTION_KEY | B_LEFT_OPTION_KEY;
		case kXRWIN:
			return B_OPTION_KEY | B_RIGHT_OPTION_KEY;
		case kXMENU:
			return B_MENU_KEY;
		case kXRTSH:
			return B_SHIFT_KEY | B_RIGHT_SHIFT_KEY;
		case kXLFSH:
			return B_SHIFT_KEY | B_LEFT_SHIFT_KEY;
		default:
			return 0;
	}
}
#endif

static bool
KeyTranslate(int32 keyval, int32 &raw_char, unsigned char &byte)
{
	switch (keyval) {
		case GDK_BackSpace:
			byte = B_BACKSPACE;
			break;
		case GDK_Return:
			byte = B_RETURN;
			break;
		case GDK_KP_Enter:
			byte = B_ENTER;
			break;
		case GDK_KP_Space:
			byte = B_SPACE;
			break;
		case GDK_KP_Tab:
			byte = B_TAB;
			break;
		case GDK_Escape:
			byte = B_ESCAPE;
			break;
		case GDK_Left:
			byte = B_LEFT_ARROW;
			break;
		case GDK_Right:
			byte = B_RIGHT_ARROW;
			break;
		case GDK_Up:
			byte = B_UP_ARROW;
			break;
		case GDK_Down:
			byte = B_DOWN_ARROW;
			break;
		case GDK_Insert:
			byte = B_INSERT;
			break;
		case GDK_Delete:
			byte = B_DELETE;
			break;
		case GDK_Home:
			byte = B_HOME;
			break;
		case GDK_End:
			byte = B_END;
			break;
		case GDK_Page_Up:
			byte = B_PAGE_UP;
			break;
		case GDK_Page_Down:
			byte = B_PAGE_DOWN;
			break;
		default:
			if (keyval < 256 && isalpha(keyval)) {
				byte = keyval;
				raw_char = tolower(keyval);
				return true;
			}
			return false;
	}

	raw_char = byte;
	return true;
}

static char
TransformToRaw(char ch)
{
	// ToDo:
	// needs to respect keymap

	if (isalpha(ch))
		return tolower(ch);

	switch (ch) {
		case '~':
			return '`';
		case '!':
			return '1';
		case '@':
			return '2';
		case '#':
			return '3';
		case '$':
			return '4';
		case '%':
			return '5';
		case '^':
			return '6';
		case '&':
			return '7';
		case '*':
			return '8';
		case '(':
			return '9';
		case ')':
			return '0';
		case '_':
			return '-';
		case '+':
			return '=';
		case '{':
			return '[';
		case '}':
			return ']';
		case '|':
			return '\\';
		case ':':
			return ';';
		case '"':
			return '\'';
		case '<':
			return ',';
		case '>':
			return '.';
		case '?':
			return '/';
		default:
			break;
	}
	return ch;
}

int
TranslateKeyCode (int)
{
	// ToDo:
	// Translate keycodes from Linux to BeOS mapping 
	// only needed for function keys and num keypad
	return 0;
}

int 
FView::KeyPressEvent(GdkEventKey *event)
{
	BMessage message(B_KEY_DOWN);
	message.AddInt64("when", system_time());
	message.AddInt32("modifiers", GnomeToBeOSModifiers(event->state));
	
	int32 rawKey;
	unsigned char byte;

	InstallKeypressFilterIfNeeded(event->window);
	// this needs translating
	message.AddInt32("key", TranslateKeyCode(lastKeyCode));

	if (KeyTranslate(event->keyval, rawKey, byte))
		message.AddInt8("byte", (int8)byte);
	else {
		for (int32 index = 0; index < event->length; index++) {
			message.AddInt8("byte", event->string[index]);
		}
		rawKey = TransformToRaw(event->string[0]);
	}

	message.AddPointer("gtk_event", event);
#if 0
	printf("keyval %d, raw %d, byte %d char %c, state %d\n", 
		event->keyval, rawKey, byte, event->string[0], event->state);
#endif
	if (rawKey > 0) {
		message.AddInt32("raw_char", rawKey);
		Looper()->ReceiveMessage(&message, this);
	}
	return true;
}

int 
FView::KeyReleaseEvent(GdkEventKey *event)
{	
	InstallKeypressFilterIfNeeded(event->window);
	return true;
}

GdkFilterReturn 
FView::KeyPressFilter(GdkXEvent *xEvent, GdkEvent *, gpointer)
{
	if (((XEvent *)xEvent)->type == KeyPress
		|| ((XEvent *)xEvent)->type == KeyRelease) {
		lastKeyCode = ((XKeyEvent *)xEvent)->keycode;
	}
	
	return GDK_FILTER_CONTINUE;
}

uint32 
modifiers()
{
	GdkModifierType modifiers;
	gdk_window_get_pointer (NULL, NULL, NULL, &modifiers);
	return GnomeToBeOSModifiers(modifiers);
}

// empty virtuals, overriden in subclasses

void 
FView::KeyDown(const char *, int32 )
{
	// empty
}

void 
FView::Draw(BRect)
{
	// empty
}

void 
FView::AttachedToWindow()
{
	// empty
}

void 
FView::DetachedFromWindow()
{
}

void 
FView::Pulse()
{
	// empty
}

void 
FView::MouseDown(BPoint)
{
	// empty
}

void 
FView::MouseUp(BPoint)
{
	// empty
}

void 
FView::MouseMoved(BPoint, uint32, const BMessage *)
{
	// empty
}

void 
FView::FrameResized(float, float)
{
	// empty
}


void 
FView::HandleMouseWheel(int32)
{
}

BScrollBar *
FView::ScrollBar(orientation which) const
{	
	for (const BView *parent = Parent(); parent; parent = parent->Parent()) {
		if (dynamic_cast<const FScrollView *>(parent))
			return dynamic_cast<const FScrollView *>(parent)->ScrollBar(which);
	}
	
	return NULL;
}

void 
FView::ScrollTo(FPoint point)
{
	BScrollBar *hScroll = ScrollBar(B_HORIZONTAL);
	if (hScroll)
		hScroll->SetValue(point.x);

	BScrollBar *vScroll = ScrollBar(B_VERTICAL);
	if (vScroll)
		vScroll->SetValue(point.y);

}

void 
FView::HScrollToCallback(float)
{
}

void 
FView::VScrollToCallback(float)
{
}

int 
FView::ButtonPressEvent(GdkEventButton *event)
{
	if (event->button == 4) {
		HandleMouseWheel(kMouseWheelScrollIncrement);
		return true;
	}
	if (event->button == 5) {
		HandleMouseWheel(-kMouseWheelScrollIncrement);
		return true;
	}

	if (event->type != GDK_BUTTON_PRESS)
		// ignore double and triple clicks,
		// figure them out ourselves
		return true;

	if (fEatNextMouseDown) {
		// this is a workaround for the problem where every first
		// click on a window first activates it and then re-sends the 
		// click event, ending in the frustruating behavior of text
		// widgets loosing their selections, etc.
		fEatNextMouseDown = false;
		if (system_time() - fEatNextMouseDownTime < 100000)
			// we got a mouse down right after a focus, must be
			// the broken case, don't issue a MouseDown callback
			return true;
	}

	FPoint point(event->x, event->y);
	MouseDown(point);
	return true;
}

int 
FView::ButtonReleaseEvent(GdkEventButton *event)
{
	FPoint point(event->x, event->y);
	MouseUp(point);
	return true;
}

int 
FView::MotionNotifyEvent(GdkEventMotion *event)
{
	FPoint point(event->x, event->y);
	MouseMoved(point, B_INSIDE_VIEW, NULL);
	return true;
}

int 
FView::EnterNotifyEvent(GdkEventCrossing *event)
{
	FPoint point(event->x, event->y);
	MouseMoved(point, B_ENTERED_VIEW, NULL);
	return true;
}

int 
FView::LeaveNotifyEvent(GdkEventCrossing *event)
{
	FPoint point(event->x, event->y);
	MouseMoved(point, B_EXITED_VIEW, NULL);
	return true;
}

void 
FView::WidgetDraw(GdkRectangle *area)
{
	if (!IsDrawable())
		return;

	Draw(*area);

	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {
		GdkRectangle intersection;
		if (gtk_widget_intersect (fChildren.ItemAt(index)->AsGtkWidget(), 
			area, &intersection))
			gtk_widget_draw (fChildren.ItemAt(index)->AsGtkWidget(), &intersection);
	}
}

int 
FView::ExposeEvent(GdkEventExpose *event)
{
	if (!IsDrawable())
		return false;

	FRegion invalidRegion;
	FRect invalRect(event->area);
	invalRect = invalRect & Bounds();
	invalidRegion.Set(invalRect);
	ConstrainClippingRegion (&invalidRegion);
	Draw(event->area);
	int32 count = fChildren.CountItems();

	// make sure expose events are propagated to items that don't get 
	// them by default (non-window items).
	for (int32 index = 0; index < count; index++) {
  		GdkEventExpose forwardedEvent;
  		forwardedEvent = *event;
		FView *view = fChildren.ItemAt(index);
		if (view->IsDrawable()
			&& (view->fObject->flags & GTK_NO_WINDOW)
			&& gtk_widget_intersect (view->AsGtkWidget(), 
				&event->area, &forwardedEvent.area))
			gtk_widget_event (view->AsGtkWidget(), (GdkEvent *)&forwardedEvent);
	}
	
	ConstrainClippingRegion (NULL);
	InstallKeypressFilterIfNeeded(event->window);
	return false;
}


void 
FView::SizeRequest(GtkRequisition *requisition)
{
	requisition->width = (int)Bounds().Width();
	requisition->height = (int)Bounds().Height();

	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {
		GtkRequisition requisition;
		gtk_widget_size_request	(fChildren.ItemAt(index)->AsGtkWidget(), &requisition);
	}
}

void 
FView::SizeAllocate(GtkAllocation *allocation)
{
	FPoint growDelta;
	
	growDelta.x = allocation->width - AsGtkWidget()->allocation.width;
	growDelta.y = allocation->height - AsGtkWidget()->allocation.height;


	AsGtkWidget()->allocation = *allocation;
	if (IsRealized())
		gdk_window_move_resize (AsGtkWidget()->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height);

	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {		
		if (fChildren.ItemAt(index)->IsVisible())
			FixedWidgetSizeAllocate((int)fChildren.ItemAt(index)->Frame().left,
				(int)fChildren.ItemAt(index)->Frame().top, 
				fChildren.ItemAt(index)->AsGtkWidget(),
				allocation,
				growDelta);
	}
	
	FrameResized(Bounds().Width(), Bounds().Height());
}

void 
FView::Map()
{
	fObject->flags |= GTK_MAPPED;

	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {		
		if (fChildren.ItemAt(index)->IsVisible())
			gtk_widget_map(fChildren.ItemAt(index)->AsGtkWidget());
	}
	gdk_window_show(AsGtkWidget()->window);
}

void 
FView::ShowAll()
{
	EachChildGtkWidget(gtk_widget_show_all);
	gtk_widget_show(AsGtkWidget());
}

void 
FView::HideAll()
{
	gtk_widget_hide(AsGtkWidget());
	EachChildGtkWidget(gtk_widget_hide_all);
}

void 
FView::EachChildGtkWidget(void (*function)(GtkWidget *))
{
	int32 count = fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {		
		(*function)(fChildren.ItemAt(index)->AsGtkWidget());
	}
}

// forall and remove are pure virtuals in GtkContainer,
// need to implement them here
void 
FView::ContainerForAll(GtkContainer *container, gboolean,
	GtkCallback callback, void *data)
{
	FView *self = GnoBeObject((GtkWidget *)container);
	ASSERT(self);

	// use a copy because forall is called when removing items
	// and ContainerRemove would screw up the iteration otherwise
	//ObjectList<FView> listCopy(self->fChildren);
	
	int32 count = self->fChildren.CountItems();
	for (int32 index = count - 1; index >= 0; index--) {		
		(callback)(self->fChildren.ItemAt(index)->AsGtkWidget(), data);
	}
}

void 
FView::ContainerAdd(GtkContainer *container, GtkWidget *widget)
{
	FView *self = GnoBeObject((GtkWidget *)container);
	ASSERT(self);

	gtk_widget_set_parent (widget, self->AsGtkWidget());

	if (self->IsRealized())
		gtk_widget_realize(widget);
	
	if (self->IsVisible() && (((_GtkObject *)widget)->flags & GTK_VISIBLE)) {
		if (self->IsMapped())
			gtk_widget_map(widget);
		gtk_widget_queue_resize(self->AsGtkWidget());
	}
}


void 
FView::ContainerRemove(GtkContainer *container, GtkWidget *widget)
{
	FView *self = GnoBeObject((GtkWidget *)container);
	ASSERT(self);

	int32 count = self->fChildren.CountItems();
	for (int32 index = 0; index < count; index++) {		
		if (self->fChildren.ItemAt(index)->AsGtkWidget() == widget) {
			gtk_widget_unparent (widget);
			self->fChildren.RemoveItemAt(index);
			break;
		}
	}
}

void
FView::FixedWidgetSizeAllocate(int x, int y, GtkWidget *widget, GtkAllocation *allocation,
	FPoint growDelta)
{
	FView *view = FView::GnoBeObject(widget);

	GtkRequisition child_requisition;
	GtkAllocation child_allocation;

	uint32 followMode = view->ResizingMode();
	gtk_widget_get_child_requisition(widget, &child_requisition);

	// This is a hack for now - to fix it I need to allow
	// subclasses of FView that supply their own gtkwidget to
	// specify which virtuals they override.
	FScrollBar *bar = dynamic_cast<FScrollBar *>(view);
	if (bar) {
		if (bar->Orientation() == B_VERTICAL)
			child_requisition.height = allocation->height
				- FScrollBar::HScrollBarHeight() - 6;
		else
			child_requisition.width = allocation->width
				- FScrollBar::VScrollBarWidth() - 6;
	}

	child_allocation.x = x;
	child_allocation.y = y;
	child_allocation.width = child_requisition.width;
	child_allocation.height = child_requisition.height;

	if (!view->fInitialAllocation) {
		if ((followMode & B_FOLLOW_LEFT) == 0 && (followMode & B_FOLLOW_RIGHT) != 0)
			// left and right edge follow parent's right edge
			child_allocation.x = x + (int)growDelta.x;

		if ((followMode & B_FOLLOW_LEFT) != 0 && (followMode & B_FOLLOW_RIGHT) != 0) 
			// left, right edge stay put
			child_allocation.width = widget->allocation.width + (int)growDelta.x;


		if ((followMode & B_FOLLOW_TOP) == 0 && (followMode & B_FOLLOW_BOTTOM) != 0)
			child_allocation.y = y + (int)growDelta.y;

		if ((followMode & B_FOLLOW_TOP) != 0 && (followMode & B_FOLLOW_BOTTOM) != 0)
			child_allocation.height = widget->allocation.height + (int)growDelta.y;
	}
	

	// don't call gtk_widget_size_allocate to avoid interference with
	// usize preset values
	gtk_signal_emit_by_name((_GtkObject *)widget, "size_allocate", &child_allocation);

	view->fInitialAllocation = false;
}

static const char invisible_cursor_bits[] = { 0 };
static const char invisible_cursor_mask_bits[] = { 0 };

// support for ObscureCursor

void 
FView::SetInvisibleCursor()
{
	XColor foreColor;
	foreColor.pixel = 0L;
	foreColor.red = foreColor.green = foreColor.blue = 0;
	foreColor.flags = DoRed | DoGreen | DoBlue;

	XColor backColor;
	backColor.pixel = 255L;
	backColor.red = backColor.green = backColor.blue = 65535;
	backColor.flags = DoRed | DoGreen | DoBlue;

	GdkWindowPrivate *window_private = (GdkWindowPrivate*) AsGtkWidget()->window;
  
	Pixmap sourcePixmap = XCreateBitmapFromData(window_private->xdisplay,
		window_private->xwindow, invisible_cursor_bits, 1, 1);
	assert(sourcePixmap != 0);

	Pixmap maskPixmap = XCreateBitmapFromData(window_private->xdisplay,
		window_private->xwindow, invisible_cursor_mask_bits, 1, 1);
	assert( maskPixmap != 0 );

	Cursor xcursor = XCreatePixmapCursor(window_private->xdisplay,
		sourcePixmap, maskPixmap, &foreColor, &backColor, 0, 0);

	XFreePixmap(window_private->xdisplay, sourcePixmap);
	XFreePixmap(window_private->xdisplay, maskPixmap);

	XDefineCursor (window_private->xdisplay, window_private->xwindow, xcursor);
}


// gtk object setup glue
void 
FView::InitializeClass()
{
	if (classInitialized)
		return;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->realize = &FView::RealizeBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->map = &FView::MapBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->show_all = &FView::ShowAllBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->hide_all = &FView::HideAllBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->draw = &FView::DrawBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->size_allocate = &FView::SizeAllocateBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->size_request = &FView::SizeRequestBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->expose_event = &FView::ExposeEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->key_press_event = &FView::KeyPressEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->key_release_event = &FView::KeyReleaseEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->focus_in_event = &FView::FocusInEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->focus_out_event = &FView::FocusOutEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->button_press_event = &FView::ButtonPressEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->button_release_event = &FView::ButtonReleaseEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->motion_notify_event = &FView::MotionNotifyEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->enter_notify_event = &FView::EnterNotifyEventBinder;
	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->leave_notify_event = &FView::LeaveNotifyEventBinder;

	GTK_CONTAINER_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->forall = &FView::ContainerForAll;
	GTK_CONTAINER_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->add = &FView::ContainerAdd;
	GTK_CONTAINER_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->remove = &FView::ContainerRemove;

	classInitialized = true;
}

void 
FView::RealizeBinder(GtkWidget *widget)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->Realize();
}

void 
FView::SizeAllocateBinder(GtkWidget *widget, GtkAllocation *allocation)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->SizeAllocate(allocation);
}

void 
FView::SizeRequestBinder(GtkWidget *widget, GtkRequisition *requisition)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->SizeRequest(requisition);
}

void 
FView::MapBinder(GtkWidget *widget)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->Map();
}

void 
FView::UnmapBinder(GtkWidget *widget)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->DetachedFromWindow();
}

void 
FView::ShowAllBinder(GtkWidget *widget)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->ShowAll();
}

void 
FView::HideAllBinder(GtkWidget *widget)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->HideAll();
}

void 
FView::DrawBinder(GtkWidget *widget, GdkRectangle *area)
{
	ASSERT(GnoBeObject(widget));
	GnoBeObject(widget)->WidgetDraw(area);
}

int 
FView::ExposeEventBinder(GtkWidget *widget, GdkEventExpose *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->ExposeEvent(event);
}

int 
FView::KeyPressEventBinder(GtkWidget *widget, GdkEventKey *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->KeyPressEvent(event);
}

int 
FView::KeyReleaseEventBinder(GtkWidget *widget, GdkEventKey *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->KeyReleaseEvent(event);
}

int 
FView::FocusInEventBinder(GtkWidget *widget, GdkEventFocus *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->FocusInEvent(event);
}

int 
FView::FocusOutEventBinder(GtkWidget *widget, GdkEventFocus *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->FocusOutEvent(event);
}

int
FView::ButtonPressEventBinder(GtkWidget *widget, GdkEventButton *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->ButtonPressEvent(event);
}

int
FView::ButtonReleaseEventBinder(GtkWidget *widget, GdkEventButton *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->ButtonReleaseEvent(event);
}

int
FView::MotionNotifyEventBinder(GtkWidget *widget, GdkEventMotion *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->MotionNotifyEvent(event);
}

int
FView::EnterNotifyEventBinder(GtkWidget *widget, GdkEventCrossing *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->EnterNotifyEvent(event);
}

int
FView::LeaveNotifyEventBinder(GtkWidget *widget, GdkEventCrossing *event)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->LeaveNotifyEvent(event);
}

uint32 BView::lastKeyCode = 0;
bool BView::filterInstalled = false;
bool BView::classInitialized = false;

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
