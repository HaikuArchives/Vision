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
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 */

#ifdef __HAIKU__
# include <ControlLook.h>
#endif
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "StatusView.h"
#include "URLCrunch.h"
#include "Vision.h"

StatusView::StatusView (BRect frame)
	: BView (
			BRect (frame.left,
						 frame.bottom - STATUS_HEIGHT,
						 frame.right,
						 frame.bottom),
			"status",
			B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
			B_WILL_DRAW)
{
	BFont font (be_plain_font);
	font_height fh;

	font.GetHeight (&fh);
	while (fh.ascent + fh.leading > STATUS_HEIGHT - 2)
	{
		font.SetSize (font.Size() - 2);
		font.GetHeight (&fh);
	}

	SetFont (&font);

	SetLowColor (ui_color (B_PANEL_BACKGROUND_COLOR));
#ifdef __HAIKU__
	SetViewColor (B_TRANSPARENT_COLOR);
#else
	SetViewColor (LowColor());
#endif
}

StatusView::~StatusView (void)
{
	while (!items.IsEmpty())
	{
		StatusItem *item ((StatusItem *)items.RemoveItem ((int32)0));

		delete item;
	}
}

void
StatusView::AddItem (StatusItem *item, bool erase)
{
	StatusItem *last ((StatusItem *)items.LastItem());
	float width (3.0);

	if (last)
		width = last->frame.right + 8.0;

	if (item->label.Length())
		width += StringWidth (item->label.String()) + 3;

	item->frame.top		= 2.0;
	item->frame.bottom = Bounds().Height();
	item->frame.left	 = width;
	item->frame.right	= width + StringWidth (item->value.String());

	if (erase)
		item->value = "";

	items.AddItem (item);
}

void
StatusView::MouseDown(BPoint point)
{
	StatusItem *item (NULL);
	for (int32 i = 0; i < items.CountItems(); i++)
	{
		item = (StatusItem *)items.ItemAt(i);
		if (item != NULL)
		{
			 if (point.x >= item->frame.left && point.x < item->frame.right)
			 {
				 item->GeneratePopUp(ConvertToScreen(point), ConvertToScreen(item->frame));
				 break;
			 }
		}
	}
	BView::MouseDown(point);
}

void
StatusView::Clear (void)
{
	int32 i,
				all (items.CountItems());

	for (i = 0; i <= all; i++)
	{
		StatusItem *item ((StatusItem *)items.RemoveItem ((int32)0));
		delete item;
	}
	
	Invalidate();
}

StatusItem *
StatusView::ItemAt (int32 which) const
{
	return (StatusItem *)items.ItemAt (which);
}

void
StatusView::SetItemValue (int32 which, const char *value, bool redraw)
{
	StatusItem *item (ItemAt (which));
	StatusItem *nextitem;
	bool completeInvalidate (false);
	
	if (item)
	{
		item->value = value;
		if (item->frame.left + StringWidth(value) != item->frame.right)
			completeInvalidate = true;
			
		item->frame.right = item->frame.left + StringWidth(value);
		for (int32 i = which+1; (nextitem = ItemAt(i)) != NULL ; i++)
		{
			nextitem->frame.left = item->frame.right + 8.0 + StringWidth(nextitem->label.String());
			nextitem->frame.right = nextitem->frame.left + StringWidth(nextitem->value.String());
			item = nextitem;
		}
		if (redraw)
		{
			Invalidate(BRect (ItemAt (which)->frame.left, item->frame.top,
				(completeInvalidate) ? Bounds().right : item->frame.right, item->frame.bottom));
		}
	}
}

void
StatusView::Draw (BRect update)
{
	SetDrawingMode (B_OP_COPY);
	SetHighColor (tint_color(LowColor(), B_DARKEN_2_TINT));
	StrokeLine (BPoint (update.left, Bounds().top),
							BPoint (update.right, Bounds().top));
#ifdef __HAIKU__
	BRect rect(Bounds());
	rect.top++;
	be_control_look->DrawMenuBarBackground(this, rect, update, LowColor());
#else
	SetHighColor (255, 255, 255, 255);
	StrokeLine (BPoint (update.left, Bounds().top + 1),
							BPoint (update.right, Bounds().top + 1));
#endif

	float width (5.0);
	font_height fh;

	GetFontHeight (&fh);

	SetDrawingMode (B_OP_OVER);
	SetHighColor (tint_color(LowColor(), 1.7));

	for (int32 i = 0; i < items.CountItems(); ++i)
	{
		if (i)
		{
			DrawSplit (width += 3);
			width += 5;
		}

		StatusItem *item (ItemAt (i));

		if (item->label.Length())
		{
			DrawString (item->label.String(),
			BPoint (width, fh.ascent + fh.leading + 2));
		}

		if (item->alignment == STATUS_ALIGN_RIGHT)
			width = item->frame.right - StringWidth (item->value.String());
		else
			width = item->frame.left;

		DrawString (item->value.String(),
		BPoint (width, fh.ascent + fh.leading + 2));
		width = item->frame.right;
	}
}


void
StatusView::DrawSplit (float x)
{
	BRect bounds (Bounds());

	PushState();

#ifdef __HAIKU__
	SetDrawingMode (B_OP_ALPHA);
	SetHighColor (0, 0, 0, 40);
	StrokeLine (BPoint (x, bounds.top + 3.0),
							BPoint (x, bounds.bottom - 2.0));

	SetHighColor (255, 255, 255, 80);
	StrokeLine (BPoint (x + 1, bounds.top + 3.0),
							BPoint (x + 1, bounds.bottom - 2.0));
#else
	SetDrawingMode (B_OP_COPY);
	SetHighColor (131, 131, 131, 255);
	StrokeLine (BPoint (x, bounds.top + 2.0),
							BPoint (x, bounds.bottom));

	SetHighColor (255, 255, 255, 255);
	StrokeLine (BPoint (x + 1, bounds.top + 2.0),
							BPoint (x + 1, bounds.bottom));
#endif

	PopState();
}

StatusItem::StatusItem (
	const char *label_,
	const char *value_,
	int32 alignment_)
{
	label = label_ ? label_ : "";
	value = value_ ? value_ : "";
	alignment = alignment_;
}

StatusItem::~StatusItem (void)
{

}

void
StatusItem::GeneratePopUp (BPoint point, BRect openrect)
{
	BString str (value);
	str.Append(" ");
	BString url;
	URLCrunch crunch (str.String(), str.Length());
	BPopUpMenu *menu = new BPopUpMenu("URLs");
	BMessage msg (M_LOAD_URL);
	BMessage *allocmsg (NULL);
	BMenuItem *item (NULL);

	while (crunch.Crunch(&url) != B_ERROR)
	{
		allocmsg = new BMessage (msg);
		allocmsg->AddString("url", url.String());
		item = new BMenuItem(url.String(), allocmsg);
		menu->AddItem(item);
		allocmsg = NULL;
	}
	
	if (menu->CountItems() > 0)
	{
		menu->SetTargetForItems(be_app);
		menu->SetAsyncAutoDestruct(true);
		menu->Go(point, true, true, openrect, true);
	}
	else
	{
		delete menu;
	}
}
