/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Vision.
 *
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 *
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 */

#define SHIFT_COMPONENT(x,y)		(y >= 1 ? \
											((uchar)(x * (2 - y))) : \
											((uchar)(255 - y * (255 - x))))

#include <Window.h>
#include "ColorSwatch.h"

struct general_ui_info
{
	rgb_color			background_color;
	rgb_color			mark_color;
	rgb_color 			ighlight_color;
	bool					color_frame;
	rgb_color			window_frame_color;
};

extern general_ui_info general_info;

ColorSwatch::ColorSwatch (

	BRect frame,
	const char *name,
	rgb_color color_,
	uint32 resize,
	uint32 flags)

	: BView (frame, name, resize, flags),
		fColor (color_)
{
}

ColorSwatch::~ColorSwatch (void)
{
}

void
ColorSwatch::AttachedToWindow (void)
{
	BView::AttachedToWindow();

	BView *view;

	if ((view = Parent()) != 0)
	{
		rgb_color c (view->ViewColor());

		SetViewColor (c);
		SetLowColor	(c);
	}
}

void
ColorSwatch::Draw (BRect)
{
	PushState();

	SetDrawingMode (B_OP_COPY);

	rgb_color high (HighColor());
	BRect colorPad (Bounds());
	SetHighColor (ValueAsColor());

	FillRect (colorPad);

	rgb_color light (ShiftColor (ValueAsColor(), 0.4));
	rgb_color dark	(ShiftColor (ValueAsColor(), 1.2));

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
	dark	= ShiftColor (ViewColor(), 1.4);
	colorPad.InsetBy (-1, -1);

	BPoint hless (-1, 0);
	BPoint hmore (1, 0);
	BPoint vless (0, -1);
	BPoint vmore (0, 1);

	if (IsFocus() && Window()->IsActive())
	{
		light = general_info.mark_color;
		dark	= general_info.mark_color;
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

rgb_color
ColorSwatch::ValueAsColor (void) const
{
	return fColor;
}

void
ColorSwatch::SetColor (rgb_color c)
{
	if (fColor.red	 == c.red
	&&	fColor.green == c.green
	&&	fColor.blue	== c.blue
	&&	fColor.alpha == c.alpha)
		return;

	fColor = c;
	if (Parent()) Invalidate (Bounds());
}

rgb_color
ColorSwatch::ShiftColor (rgb_color c, float percent) const
{
	rgb_color result =
	{
		SHIFT_COMPONENT(c.red,	 percent),
		SHIFT_COMPONENT(c.green, percent),
		SHIFT_COMPONENT(c.blue,	percent),
		SHIFT_COMPONENT(c.alpha,	255)
	};

	return result;
}

rgb_color
ColorSwatch::Inverted (void) const
{
	rgb_color baseColor, value = ValueAsColor();

	baseColor.red	 = 255 - value.red;
	baseColor.green = 255 - value.green;
	baseColor.blue	= 255 - value.blue;
	baseColor.alpha = value.alpha;

	return baseColor;
}
