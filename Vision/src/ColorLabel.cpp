

#define SHIFT_COMPONENT(x,y)		(y >= 1 ? \
											((uchar)(x * (2 - y))) : \
											((uchar)(255 - y * (255 - x))))

//#include <Application.h>
#include <ColorControl.h>

#include <stdio.h>

#include "ColorLabel.h"

const uint32 M_COLOR_CHANGE = 'mccc';


struct general_ui_info
{
	rgb_color			background_color;
	rgb_color			mark_color;
	rgb_color 			ighlight_color;
	bool					color_frame;
	rgb_color			window_frame_color;
};

extern _IMPORT general_ui_info general_info;

BaseColorControl::BaseColorControl (

	BRect frame,
	const char *name,
	const char *label,
	rgb_color color_,
	BMessage *msg,
	uint32 resize,
	uint32 flags)

	: BControl (frame, name, label, msg, resize, flags),
	  color (color_)
{
}

BaseColorControl::~BaseColorControl (void)
{
}

void
BaseColorControl::AttachedToWindow (void)
{
	BControl::AttachedToWindow();

	BView *view;

	if ((view = Parent()) != 0)
	{
		rgb_color c (view->ViewColor());

		SetViewColor (c);
		SetLowColor  (c);
	}
}

void
BaseColorControl::Draw (BRect)
{
	PushState();

	if (strcmp (Label(), ""))
	{	
	  font_height fh;
	
	  GetFontHeight (&fh);
	  SetHighColor (0, 0, 0, 255);
	  SetDrawingMode (B_OP_OVER);
	  DrawString (Label(), BPoint (0, fh.ascent));
    }
	SetDrawingMode (B_OP_COPY);
	
	rgb_color high (HighColor());
	BRect colorPad (ColorRect());
	SetHighColor (ValueAsColor());

	FillRect (colorPad);

	rgb_color light (ShiftColor (ValueAsColor(), 0.4));
	rgb_color dark  (ShiftColor (ValueAsColor(), 1.2));

	BeginLineArray (10);
	AddLine (
		colorPad.LeftTop(),
		colorPad.RightTop() - BPoint (1, 0),
		light);

	AddLine (
		colorPad.LeftTop(),
		colorPad.LeftBottom() - BPoint (0, 1),
		light);

	AddLine (
		colorPad.RightTop() + BPoint (0, 1),
		colorPad.RightBottom(),
		dark);
	
	AddLine (
		colorPad.RightBottom(),
		colorPad.LeftBottom() + BPoint (1, 0),
		dark);

	light = ShiftColor (ViewColor(), 0.1);
	dark  = ShiftColor (ViewColor(), 1.4);
	colorPad.InsetBy (-1, -1);

	BPoint hless (-1, 0);
	BPoint hmore (1, 0);
	BPoint vless (0, -1);
	BPoint vmore (0, 1);

	if (IsFocus() && Window()->IsActive())
	{
		light = general_info.mark_color;
		dark  = general_info.mark_color;
		hless = hmore = vless = vmore = BPoint (0, 0);
	}
	else
	{
		// A little blue residue clean up
		AddLine (
			colorPad.RightTop(),
			colorPad.RightTop(),
			ViewColor());
		AddLine (
			colorPad.LeftBottom(),
			colorPad.LeftBottom(),
			ViewColor());
	}

	AddLine (	
		colorPad.LeftTop(),
		colorPad.RightTop() + hless,
		dark);

	AddLine (
		colorPad.LeftTop(),
		colorPad.LeftBottom() + vless,
		dark);

	AddLine (
		colorPad.RightTop() + vmore,
		colorPad.RightBottom(),
		light);
	
	AddLine (
		colorPad.RightBottom(),
		colorPad.LeftBottom() + hmore,
		light);

	EndLineArray();
	PopState();
}

void
BaseColorControl::MessageReceived (BMessage *msg)
{
	if (msg->WasDropped())
	{
		const rgb_color *msgColor;
		ssize_t size;

		if (msg->FindData ("RGBColor", B_RGB_COLOR_TYPE,
			reinterpret_cast<const void **>(&msgColor), &size) == B_NO_ERROR)
		{
			SetColor (*msgColor);
			Invalidate();
			return;
		}
	}

	BControl::MessageReceived (msg);
}

void
BaseColorControl::GetPreferredSize (float *width, float *height)
{
	font_height fh;

	GetFontHeight (&fh);
	*height = fh.ascent + fh.leading + fh.descent;
	*width = StringWidth (Label())
		+ floor ((fh.ascent + fh.descent + fh.leading) * 1.5) + 5;
}

void
BaseColorControl::ResizeToPreferred (void)
{
	float width, height;

	GetPreferredSize (&width, &height);

	// Most controls don't update the width
	// We'll do the same
	ResizeTo (width, height);
}

rgb_color
BaseColorControl::ValueAsColor (void) const
{
	return color;
}

void
BaseColorControl::SetColor (rgb_color c)
{
	if (color.red   == c.red
	&&  color.green == c.green
	&&  color.blue  == c.blue
	&&  color.alpha == c.alpha)
		return;

	color = c;
	if (Parent()) Invalidate (ColorRect());
}


BRect
BaseColorControl::ColorRect (void) const
{
	BRect bounds (Bounds());
	if (strcmp(Label(), ""))
	{
	  font_height fh;

	  GetFontHeight (&fh);

	  bounds.left = bounds.right - floor ((fh.ascent + fh.descent + fh.leading) * 1.5);
	}
	return bounds.InsetByCopy (1, 1);
}

rgb_color
BaseColorControl::ShiftColor (rgb_color c, float percent) const
{
	rgb_color result =
	{
		SHIFT_COMPONENT(c.red,   percent),
		SHIFT_COMPONENT(c.green, percent),
		SHIFT_COMPONENT(c.blue,  percent),
		SHIFT_COMPONENT(c.alpha,  255)
	};

	return result;
}

rgb_color
BaseColorControl::Inverted (void) const
{
	rgb_color baseColor, value = ValueAsColor();

	baseColor.red   = 255 - value.red;
	baseColor.green = 255 - value.green;
	baseColor.blue  = 255 - value.blue;
	baseColor.alpha = value.alpha;

	return color;
}

ColorLabel::ColorLabel (

	BRect frame,
	const char *name,
	const char *label,
	rgb_color color_,
	BMessage *msg,
	uint32 resize,
	uint32 flags)

	: BaseColorControl (frame, name, label, color_, msg, resize, flags)
{
}

ColorLabel::~ColorLabel (void)
{
}

void
ColorLabel::MouseDown (BPoint)
{

	// Little song and dance for those who ... wait,
	// who doesn't like a little singing and dancing?
	saveColor = ValueAsColor();
	tracking  = true;
	
	//SetColor (Inverted());
	//Window()->UpdateIfNeeded();

	mousedown = system_time();
	SetMouseEventMask (B_POINTER_EVENTS);
}

void
ColorLabel::MouseUp (BPoint point)
{
	if (tracking)
	{
		if (Bounds().Contains (point))
		{
			bigtime_t now (system_time());

			if (now - mousedown < 100000)
				snooze (100000 - (now - mousedown));

			SetColor (saveColor);

			// We want to invoke here too!
			Invoke();
		}
		
		tracking = false;
	}
}

status_t
ColorLabel::Invoke (BMessage *msg)
{
	return BControl::Invoke (msg);
}

ColorPicker::ColorPicker (

	BPoint point,
	rgb_color color,
	BMessenger msgr,
	BMessage *msg)

	: BWindow (
		BRect (
			point, 
			point + BPoint (30, 30)),
		"Color",
		B_FLOATING_WINDOW_LOOK,
		B_NORMAL_WINDOW_FEEL, 
		B_NOT_RESIZABLE
			| B_NOT_ZOOMABLE
			| B_ASYNCHRONOUS_CONTROLS
			| B_WILL_ACCEPT_FIRST_CLICK),
		audience (msgr),
		invoked (*msg)
{
	picker = new BColorControl (
		BPoint (10, 10),
		B_CELLS_32x8,
		4.0,
		"picker",
		new BMessage (M_COLOR_CHANGE),
		true);

	float width, height;
	picker->GetPreferredSize (&width, &height);

	BRect frame (0, 0, width + 20, height + 20);
	BView *view (new BView (
		frame,
		"Background",
		B_FOLLOW_LEFT | B_FOLLOW_TOP,
		B_WILL_DRAW));

	view->SetViewColor (222, 222, 222, 255);
	AddChild (view);

	view->AddChild (picker);
	ResizeTo (view->Bounds().Width(), view->Bounds().Height());

	picker->SetValue (color);
	picker->MakeFocus (true);
	Activate(true);
}

ColorPicker::~ColorPicker (void)
{
}

void
ColorPicker::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_COLOR_CHANGE:
		{
			BMessage cMsg (invoked);
			rgb_color color;

			color = picker->ValueAsColor();
			cMsg.AddData (
				"color",
				B_RGB_COLOR_TYPE,
				&color,
				sizeof (rgb_color));

			audience.SendMessage (&cMsg);
			
			break;
		}

		case M_COLOR_SELECT:
		{
			const rgb_color *color;
			ssize_t size;

			if (msg->FindData (
					"color",
					B_RGB_COLOR_TYPE,
					reinterpret_cast<const void **>(&color),
					&size) == B_NO_ERROR)
			{
				picker->SetValue (*color);
				break;
			}

		}

		default:
			BWindow::MessageReceived (msg);
	}
}
