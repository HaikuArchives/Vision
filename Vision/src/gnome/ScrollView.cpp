// BScrollView-like class
//
// see License at the end of file

#include "ScrollView.h"
#include "Window.h"
#include <gdk/gdkx.h>
#include <gdk/gdk.h>

static FRect
AdaptFrame(BView *target, border_style, bool horizontal, bool vertical)
{
	FRect result = target->Frame();
	if (horizontal)
		result.bottom += FScrollBar::HScrollBarHeight();
	if (vertical)
		result.right += FScrollBar::VScrollBarWidth();
	// add more here for fancy border
	return result;
}

FScrollView::FScrollView(const char *name, BView *target, uint32 resizeMode,
	uint32 flags, bool horizontal, bool vertical, border_style border)
	:	FView(AdaptFrame(target, border, horizontal, vertical), 
			name, resizeMode, flags),
		fHorizontal(NULL),
		fVertical(NULL),
		fTracking(false)
{
	target->MoveTo(FPoint(0, 0));
	AddChild(target);
	if (horizontal) {
		FRect frame(Bounds());
		frame.top = frame.bottom - FScrollBar::HScrollBarHeight();
		if (vertical)
			frame.right -= FScrollBar::VScrollBarWidth();
		fHorizontal =  new BScrollBar(frame, "_HSB_", target, 0, 100, B_HORIZONTAL);
		AddChild(fHorizontal);
	}
	if (vertical) {
		FRect frame(Bounds());
		frame.left = frame.right - FScrollBar::VScrollBarWidth();
		if (horizontal)
			frame.bottom -= FScrollBar::HScrollBarHeight();
		fVertical =  new BScrollBar(frame, "_VSB_", target, 0, 100, B_VERTICAL);
		AddChild(fVertical);
	}
}

FScrollBar *
FScrollView::ScrollBar(orientation which) const
{
	return (which == B_VERTICAL) ? fVertical : fHorizontal;
}


void 
FScrollView::Draw(FRect)
{
	// ToDo:
	// make the dot pattern diagonal to match the corner resizing

	fResizeCornerRect = Bounds();

	fResizeCornerRect.left = fResizeCornerRect.right - FScrollBar::VScrollBarWidth();
	fResizeCornerRect.top = fResizeCornerRect.bottom - FScrollBar::HScrollBarHeight();
	
	SetHighColor(152, 152, 152);
	StrokeRect(fResizeCornerRect);
	fResizeCornerRect.InsetBy(1, 1);

	rgb_color color = Color(225, 225, 225);
	SetHighColor(color);
	FillRect(fResizeCornerRect);
	rgb_color borderHigh = color.ShiftBy(0.4);
	rgb_color borderLow = color.ShiftBy(1.1);
	BeginLineArray(4);
	AddLine(fResizeCornerRect.RightTop(), fResizeCornerRect.RightBottom(), borderLow);
	AddLine(fResizeCornerRect.RightBottom(), fResizeCornerRect.LeftBottom(), borderLow);
	AddLine(fResizeCornerRect.LeftBottom(), fResizeCornerRect.LeftTop(), borderHigh);
	AddLine(fResizeCornerRect.LeftTop(), fResizeCornerRect.RightTop(), borderHigh);
	EndLineArray();
	
	borderHigh = Color(255, 255, 255);
	borderLow = color.ShiftBy(1.6);

	fResizeCornerRect.InsetBy(3, 3);
	for (float y = fResizeCornerRect.top; y < fResizeCornerRect.bottom; y += 3)
		for (float x = fResizeCornerRect.left; x < fResizeCornerRect.right; x += 3) {
			SetHighColor(borderLow);
			StrokeLine(BPoint(x, y), BPoint(x, y));
			SetHighColor(borderHigh);
			StrokeLine(BPoint(x + 1, y + 1), BPoint(x + 1, y + 1));
		}
}


void 
FScrollView::MouseDown(BPoint point)
{
	if (fResizeCornerRect.Contains(point)) {
#if 0
		fTracking = true;
		fTrackInitialPoint = point;
		fTrackLastPoint = point;
		fTrackInitialWindowSize.x = Window()->Frame().Width();
		fTrackInitialWindowSize.y = Window()->Frame().Height();
#else

		FPoint rootWindowRelativePoint;
		rootWindowRelativePoint = ConvertToScreen(point);
		rootWindowRelativePoint = Window()->ConvertFromScreen(rootWindowRelativePoint);
		
		GdkEventClient event;
		event.message_type = gdk_atom_intern("_NET_WM_MOVERESIZE", false);
		event.data_format = 32;
		event.data.l[0] = (int)rootWindowRelativePoint.x;
		event.data.l[1] = (int)rootWindowRelativePoint.y;
		event.data.l[2] = 4;

		g_print("sending client message type %lx, x %ld, y %ld, window xid %lx\n",
			event.message_type, event.data.l[0],
			event.data.l[1], GDK_WINDOW_XWINDOW(Window()->AsGtkWidget()->window));

		gdk_event_send_client_message((GdkEvent *)&event,
			GDK_WINDOW_XWINDOW(Window()->AsGtkWidget()->window));
#endif
	}
}

void 
FScrollView::MouseUp(BPoint)
{
	fTracking = false;
}

void 
FScrollView::MouseMoved(BPoint, uint32, const BMessage *)
{
	uint32 buttons;
	FPoint point;
	GetMouse(&point, &buttons);
	
//	if (!buttons) {
//		fTracking = false;
//		return;
//	}
	
	if (fTracking && point != fTrackLastPoint) {
		FPoint delta = point - fTrackInitialPoint;
		
		float width = fTrackInitialWindowSize.x + delta.x;
		float height = fTrackInitialWindowSize.y + delta.y;
											
		Window()->ResizeTo(width, height);
		fTrackLastPoint = point;
	}	
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
