// BView-like class
//
// see License at the end of file

#ifndef FVIEW__
#define FVIEW__

#include "PlatformDefines.h"

#include "Color.h"
#include "Font.h"
#include "GraphicDefs.h"
#include "Handler.h"

#include "ObjectGlue.h"
#include "ObjectList.h"

#include <gtk/gtkstyle.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkcontainer.h>
#include <gdk/gdkkeysyms.h>
#include <set>

#if 0
// having problems with gcc preprocessor/const handling ...
const uint32 B_FOLLOW_NONE = 0;
const uint32 B_FOLLOW_LEFT = 1;
const uint32 B_FOLLOW_RIGHT = 2;
const uint32 B_FOLLOW_TOP = 4;
const uint32 B_FOLLOW_BOTTOM = 8;
const uint32 B_FOLLOW_LEFT_RIGHT = B_FOLLOW_LEFT | B_FOLLOW_RIGHT;
const uint32 B_FOLLOW_TOP_BOTTOM = B_FOLLOW_TOP | B_FOLLOW_BOTTOM;
const uint32 B_FOLLOW_ALL_SIDES = B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM;
#else
#define B_FOLLOW_NONE 0
#define B_FOLLOW_LEFT 1
#define B_FOLLOW_RIGHT 2
#define B_FOLLOW_TOP 4
#define B_FOLLOW_BOTTOM 8
#define B_FOLLOW_LEFT_RIGHT (B_FOLLOW_LEFT | B_FOLLOW_RIGHT)
#define B_FOLLOW_TOP_BOTTOM (B_FOLLOW_TOP | B_FOLLOW_BOTTOM)
#define B_FOLLOW_ALL_SIDES (B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM)
#define B_FOLLOW_ALL B_FOLLOW_ALL_SIDES
#endif

typedef uint32 orientation;

const orientation B_VERTICAL = 1;
const orientation B_HORIZONTAL = 2;

class FWindow;
class FRegion;
class FBitmap;
class BScrollBar;
typedef struct _GtkFixedChild GtkFixedChild;


class FView : public ObjectGlue<GtkWidget>, public FHandler {
public:
	FView(FRect, const char *, uint32, uint32);
	virtual ~FView();
	
	FRect Frame() const;
	FRect Bounds() const;
	FPoint LeftTop() const
		{ return Bounds().LeftTop(); }

	uint32 ResizingMode() const;
	uint32 Flags() const;

	rgb_color ViewColor() const;
	virtual void SetViewColor(rgb_color);
	void SetViewColor(uchar, uchar, uchar, uchar = 255);

	rgb_color LowColor() const;
	virtual void SetLowColor(rgb_color);
	void SetLowColor(uchar, uchar, uchar, uchar = 255);

	rgb_color HighColor() const;
	virtual void SetHighColor(rgb_color);
	void SetHighColor(uchar, uchar, uchar, uchar = 255);

	void SetDrawingMode(drawing_mode);
	drawing_mode DrawingMode() const;

	FRect ConvertToScreen(FRect) const;
	FRect ConvertFromScreen(FRect) const;
	
	FPoint ConvertToScreen(FPoint) const;
	FPoint ConvertFromScreen(FPoint) const;
	
	void ConvertToParent(BRect *r) const;
	BRect ConvertToParent(BRect r) const;
	void ConvertToParent(BPoint *pt) const;
	BPoint ConvertToParent(BPoint pt) const;

	void Invalidate(FRect);
	void Invalidate();

	void GetMouse(FPoint *, uint32 *, bool unused = false);

	FWindow *Window() const;
	FView *Parent() const;
	const FView *FindView(const char *) const;
	FView *FindView(const char *);
	
	virtual void Draw(BRect);
	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();
	virtual void Pulse();
	virtual void KeyDown(const char *bytes, int32 numBytes);
	virtual void MouseDown(BPoint);
	virtual void MouseUp(BPoint);
	virtual void MouseMoved(BPoint, uint32, const BMessage *);
	virtual void FrameResized(float, float);
	virtual void MakeFocus(bool = true);
	bool IsFocus() const;

	virtual void SetFont(const FFont *, uint32 = 0);
	void GetFont(FFont *) const;
	void SetFontSize(int32);
	void GetFontHeight(font_height *fontHeight) const;

	void ConstrainClippingRegion(const FRegion *);
	void GetClippingRegion(FRegion *);

	void FillRect(FRect rect, int32 pattern = B_SOLID_HIGH);

	void StrokeRect(FRect);
	void InvertRect(FRect);
	void StrokePolygon(const BPoint *, int32, bool);
	void StrokeLine(BPoint, BPoint, int32 pattern = B_SOLID_HIGH);
	void StrokeLine(BPoint end, int32 pattern = B_SOLID_HIGH);

	void BeginLineArray(int32);
	void AddLine(FPoint, FPoint, rgb_color);
	void EndLineArray();
	
	void DrawBitmap(const FBitmap *);
	void DrawBitmap(const FBitmap *, FRect from, FRect to);
	
	void MovePenTo(FPoint);
	void MovePenTo(float, float);

	void DrawString(const char *, int32);
	void DrawString(const char *);
	void DrawString(const char *, BPoint point);
	float StringWidth(const char *) const;

	void Sync() {}

	void AddChild(FView *);
	void AddToWindow(FWindow *);
		// called by FWindow only

	void RemoveSelf();

	void MoveTo(FPoint);
	void MoveBy(float, float);
	void ResizeTo(float, float);

	virtual void ResizeToPreferred()
		{
#ifdef GTK_BRINGUP
		// unimplemented
#endif
		}
	
	virtual BScrollBar *ScrollBar(orientation) const;
	virtual void ScrollTo(FPoint);

	GtkWidget *AsGtkWidget()
		{ return GTK_WIDGET(fObject); }
		
	const GtkWidget *AsGtkWidget() const
		{ return reinterpret_cast<const GtkWidget *>(fObject); }

	void CopyBits(FRect from, FRect to);	// not originally a BeOS call

	static void FixedWidgetSizeAllocate(int x, int y, GtkWidget *, 
		GtkAllocation *, FPoint growDelta);
	
	virtual void HScrollToCallback(float);
	virtual void VScrollToCallback(float);

	void SetInvisibleCursor();
	
	void GetBlendingMode(source_alpha *srcAlpha, alpha_function *alphaFunc) const;
	void SetBlendingMode(source_alpha srcAlpha, alpha_function alphaFunc);
	
	bool LockLooper() const;
	void UnlockLooper() const;
	
	void PushState() const;
	void PopState() const;


protected:	
	FView(GtkWidget *, FRect, const char *, uint32, uint32);
		// for overrides that parallel the Gtk hierarchy 

	bool IsRealized() const
		{ return (fObject->flags & GTK_REALIZED) != 0; }
		// GTK_WIDGET_IS_REALIZED fails here because the
		// GTK_OBJECT macro confuses the compiler
	
	bool IsVisible() const
		{ return (fObject->flags & GTK_VISIBLE) != 0; }
	bool IsMapped() const
		{ return (fObject->flags & GTK_MAPPED) != 0; }
	bool IsDrawable() const
		{ return IsVisible() && IsMapped(); }

	virtual void HandleMouseWheel(int32 mouseMoveDelta);
		
private:
	void FViewCommon();
	void StopPulsingIfNeeded();
	static gboolean PulseDispatcher(void *);

	void InstallKeypressFilterIfNeeded(GdkWindow *);
	static GdkFilterReturn KeyPressFilter(GdkXEvent *, GdkEvent *, gpointer);

	static FView *GnoBeObject(GtkWidget *gtkObject)
		{ return ::GnoBeObject<FView, GtkWidget>(gtkObject); }
	static FView *GnoBeObjectIfTypeMatches(GtkWidget *gtkObject)
		{ return ::GnoBeObjectIfTypeMatches<FView, GtkWidget>(gtkObject); }

protected:
	// most of these should rarely get overriden since they map
	// to the corresponding BeOS virtual
	virtual void Realize();	
	virtual void Map();
	virtual void ShowAll();
	virtual void HideAll();
	virtual void SizeAllocate(GtkAllocation *);
	virtual void SizeRequest(GtkRequisition *);
	virtual void WidgetDraw(GdkRectangle *);
	virtual int ExposeEvent(GdkEventExpose *);
	virtual int KeyPressEvent(GdkEventKey *);
	virtual int KeyReleaseEvent(GdkEventKey *);
	virtual int FocusInEvent(GdkEventFocus *);
	virtual int FocusOutEvent(GdkEventFocus *);
	virtual int ButtonPressEvent(GdkEventButton *);
	virtual int ButtonReleaseEvent(GdkEventButton *);
	virtual int MotionNotifyEvent(GdkEventMotion *);
	virtual int EnterNotifyEvent(GdkEventCrossing *);
	virtual int LeaveNotifyEvent(GdkEventCrossing *);


	// non-BeOS only default method overriding glue
	void InitializeClass();

private:
	static void RealizeBinder(GtkWidget *);
	static void MapBinder(GtkWidget *);
	static void UnmapBinder(GtkWidget *);
	static void ShowAllBinder(GtkWidget *);
	static void HideAllBinder(GtkWidget *);
	static void SizeAllocateBinder(GtkWidget *, GtkAllocation *);
	static void SizeRequestBinder(GtkWidget *, GtkRequisition *);
	static void DrawBinder(GtkWidget *, GdkRectangle *);
	static int ExposeEventBinder(GtkWidget *, GdkEventExpose *);

protected:
	static int KeyPressEventBinder(GtkWidget *, GdkEventKey *);

private:
	static int KeyReleaseEventBinder(GtkWidget *, GdkEventKey *);
	static int FocusInEventBinder(GtkWidget *, GdkEventFocus *);
	static int FocusOutEventBinder(GtkWidget *, GdkEventFocus *);
	static int ButtonPressEventBinder(GtkWidget *, GdkEventButton *);
	static int ButtonReleaseEventBinder(GtkWidget *, GdkEventButton *);
	static int MotionNotifyEventBinder(GtkWidget *, GdkEventMotion *);
	static int EnterNotifyEventBinder(GtkWidget *, GdkEventCrossing *);
	static int LeaveNotifyEventBinder(GtkWidget *, GdkEventCrossing *);

	static void ContainerForAll(GtkContainer *, gboolean, GtkCallback, void *);
	static void ContainerAdd(GtkContainer *, GtkWidget *);
	static void ContainerRemove(GtkContainer *, GtkWidget *);

	void EachChildGtkWidget(void (*)(GtkWidget *));
	void AddedToWindow(FWindow *);
	void RemovedFromWindow(FWindow *);
	void AddViewsToWindow(FWindow *);
		// adds the entire view hierarchy to the window

	virtual void PrepareInitialSize(FRect);

	FRect fInitialRect;
	bool fInitialAllocation;
	uint32 fFlags;
	uint32 fFollowFlags;
	FPoint fPenOffset;
	FFont fFont;
	
	// current active gcs for foreground and background drawing
	GdkGC *fForegroundGC;
	GdkGC *fBackgroundGC;
	
	// gcs that support the different drawing modes
	GdkGC *fCopyGC;
	GdkGC *fInvertGC;
	GdkGC *fScratchGC;
		// used by calls such as AddLine to quickly swap color

	FRegion *fClipRegion;
		// current clip region
	
	rgb_color fViewColor;
	rgb_color fLowColor;
	rgb_color fHighColor;
	drawing_mode fDrawingMode;
	uint32 fIdleHandler;
	bool fGtkFocused;
	bool fEatNextMouseDown;
	bigtime_t fEatNextMouseDownTime;
	set<GdkWindow *> fInstalledFilters;

	ObjectList<FView> fChildren;

private:
	static uint32 lastKeyCode;
	static bool filterInstalled;
	static bool classInitialized;
};

#endif

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
