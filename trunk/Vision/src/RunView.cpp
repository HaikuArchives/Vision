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
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 *
 * Contributor(s): Rene Gollent
 *                 Todd Lair
 */

#define FORE_WHICH				0
#define BACK_WHICH				1
#define FONT_WHICH				2

#define MARGIN_WIDTH			10.0
#define MARGIN_INDENT			10.0

#define M_OFFVIEW_SELECTION		'mros'
#define OFFVIEW_TIMER					(10000LL)
#define ABS(x)							(x * ((x<0) ? -1 : 1))
#define SOFTBREAK_STEP			5

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <slist>

#include <Message.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Clipboard.h>
#include <ScrollView.h>
#include <ScrollBar.h>
#include <Region.h>
#include <Window.h>
#include <Bitmap.h>
#include <Cursor.h>

#include <Debug.h>
#include <StopWatch.h>

#include "Theme.h"
#include "RunView.h"
#include "URLCrunch.h"
#include "Vision.h"


// cursor data for hovering over URLs

static unsigned char URLCursorData[] = {16,1,2,2,
0,0,0,0,56,0,36,0,36,0,19,224,18,92,9,42,
8,1,60,33,76,49,66,121,48,125,12,253,2,0,1,0,
0,0,0,0,56,0,60,0,60,0,31,224,31,252,15,254,
15,255,63,255,127,255,127,255,63,255,15,255,3,254,1,248
};

const float doubleClickThresh = 6;

struct SoftBreak
{
	int16					offset;
	float					height;
	float					ascent;
};

struct URL
{
	int32					offset;
	int32					length;
	BString					url;

							URL (const char *address, int32 off, int32 len) :
								offset (off),
								length (len),
								url (address)
								{ }
};

typedef slist<URL *> urllist;

struct SoftBreakEnd
{
	int16					offset;

							SoftBreakEnd (int16 offset)
								:	offset (offset)
							{ }
};

struct FontColor
{
	int16					offset;
							// G++ is stupid.  We only need 2 bits
							// for which, but the compiler has a bug
							// and warns us against which == 2
	int16					which			: 3;
	int16					index			: 13;
};

struct Line
{
	char					*text;
	time_t					stamp;
	urllist					*urls;
	int16					*spaces;
	float					*edges;
	FontColor				*fcs;
	SoftBreak				*softies;
	float					top;
	float					bottom;

	int16					length;
	int16					space_count;
	int16					edge_count;
	int16					fc_count;
	int16					softie_size;
	int16					softie_used;

							Line (
								const char *buffer,
								int16 length,
								float top,
								float width,
								Theme *theme,
								const char *stamp_format,
								int16 fore,
								int16 back,
								int16 font);

							~Line (void);

	void					Append (
								const char *buffer,
								int16 len,
								float width,
								Theme *theme,
								int16 fore,
								int16 back,
								int16 font);

	void					FigureSpaces (void);

	void					FigureFontColors (
								int16 pos,
								int16 fore,
								int16 back,
								int16 font);

	void					FigureEdges (
								Theme *theme,
								float width);

	void					SoftBreaks (
								Theme * theme,
								float width);

	int16					CountChars (int16 pos, int16 len);
	size_t				SetStamp (const char *, bool);

	void				SelectWord (int16 *, int16 *);
};

inline int32
UTF8_CHAR_LEN (uchar c)
{
 return (((0xE5000000 >> (((c) >> 3) & 0x1E)) & 3) + 1);
}

RunView::RunView (
	BRect frame,
	const char *name,
	Theme *theme,
	uint32 resizingMode,
	uint32 flags)
	:	BView (
			frame,
			name,
			resizingMode,
			flags | B_WILL_DRAW | B_FRAME_EVENTS),
		scroller (NULL),
		theme (theme),
		working (NULL),
		line_count (0),
		stamp_format (NULL),
		clipping_name (NULL),
		sp_start (0, 0),
		sp_end (0, 0),
		tracking (0),
		track_offset (0, 0),
		off_view_runner (NULL),
		off_view_time (0),
		resizedirty (false),
		fontsdirty (false),
		myPopUp (NULL),
		lastClick (0,0),
		lastClickTime (0)
{
	URLCursor = new BCursor (URLCursorData);
	theme->ReadLock();

	BView::SetViewColor (B_TRANSPARENT_COLOR);
	BView::SetLowColor (theme->BackgroundAt (Theme::NormalBack));
	BView::SetHighColor (theme->ForegroundAt (Theme::NormalFore));

	theme->ReadUnlock();
}

RunView::~RunView (void)
{
	for (int16 i = 0; i < line_count; ++i)
		delete lines[i];

	delete working;
	delete URLCursor;
}

void
RunView::AttachedToWindow (void)
{
	BView::AttachedToWindow();

	RecalcScrollBar (false);
	theme->WriteLock();
	theme->AddView (this);
	theme->WriteUnlock();
}

void
RunView::DetachedFromWindow (void)
{
	theme->WriteLock();
	theme->RemoveView (this);
	theme->WriteUnlock();
}

void
RunView::FrameResized (float start_width, float height)
{
	BView::FrameResized (start_width, height);

	if (IsHidden())
	{
		resizedirty = true;
		return;
	}
	ResizeRecalc();
}

void
RunView::TargetedByScrollView (BScrollView *s)
{
	scroller = s;
	BView::TargetedByScrollView (scroller);
}

void
RunView::Show (void)
{
	if (fontsdirty)
	{
		FontChangeRecalc();
		// this effectively does the same thing as resize so if both have changed, only
		// do the fonts recalculation
		fontsdirty = false;
		resizedirty = false;
	}
	else if (resizedirty)
	{
		ResizeRecalc();
		resizedirty = false;
	}
	BView::Show();
}

void
RunView::Draw (BRect frame)
{
	Window()->DisableUpdates();
	//BStopWatch watch ("draw");
	rgb_color low_color, hi_color, view_color, sel_color, sel_text;
	float height (frame.bottom);
	BRect bounds (Bounds());
	BRegion clipper;
	bool drawSelection (false);
	bool checkSelection (sp_start != sp_end);

	clipper.Set (frame);
	ConstrainClippingRegion (&clipper);

	theme->ReadLock();
	view_color = theme->BackgroundAt (Theme::NormalBack);

	sel_color = theme->BackgroundAt (Theme::SelectionBack);
	if (((sel_color.red + sel_color.blue + sel_color.green) / 3) >= 127)
	{
		sel_text.red = sel_text.green = sel_text.blue = 0;
		sel_text.alpha = 255;
	}
	else
	{
		sel_text.red = sel_text.green = sel_text.blue = sel_text.alpha = 255;
	}
	BRect remains;
	if (line_count == 0)
		remains = frame;
	else if (frame.bottom >= lines[line_count - 1]->bottom + 1.0)
		remains.Set (
			frame.left,
			lines[line_count - 1]->bottom + 1.0,
			frame.right,
			frame.bottom);

	if (remains.IsValid())
	{
		SetLowColor (view_color);
		FillRect (remains, B_SOLID_LOW);
	}

	for (int16 i = line_count - 1; i >= 0; --i)
	{
		Line *line (lines[i]);
		if (line->bottom < frame.top)
			break;

		BRect r (bounds.left, line->top, bounds.right, line->bottom);

		if (!frame.Intersects (r))
			continue;

		float indent (ceil (MARGIN_WIDTH / 2.0));
		int16 place (0);

		int16 fore (0);
		int16 back (0);
		int16 font (0);

		height = line->top;

		for (int16 sit = 0; sit < line->softie_used; ++sit)
		{
			int16 last_len (UTF8_CHAR_LEN (line->text[line->softies[sit].offset]));
			float left (indent);
			float start (0.0);

			// Fill indentation
			SetLowColor (view_color);

			SetDrawingMode (B_OP_COPY);
			r.Set (0.0, height, indent - 1.0, height + line->softies[sit].height - 1.0);
			FillRect (r, B_SOLID_LOW);

			if (sit)
			{
				int16 i (place);

				while (--i >= 0)
					if ((start = line->edges[i]) != 0)
						break;
			}

			while (place < line->softies[sit].offset + last_len)
			{
				// Get current foreground color and set
				while (fore < line->fc_count)
				{
					if (line->fcs[fore].which == FORE_WHICH)
					{
						if (line->fcs[fore].offset > place)
							break;

						hi_color = theme->ForegroundAt (line->fcs[fore].index);
					}

					++fore;
				}

				// Get current background color and set
				while (back < line->fc_count)
				{
					if (line->fcs[back].which == BACK_WHICH)
					{
						if (line->fcs[back].offset > place)
							break;


						low_color = theme->BackgroundAt (line->fcs[back].index);
					}

					++back;
				}

				// Get current font and set
				while (font < line->fc_count)
				{
					if (line->fcs[font].which == FONT_WHICH)
					{
						if (line->fcs[font].offset > place)
							break;

						const BFont &f (theme->FontAt (line->fcs[font].index));

						SetFont (&f);
					}

					++font;
				}

				int16 length (line->softies[sit].offset - place + last_len);

				if (fore < line->fc_count
				&&  line->fcs[fore].offset - place < length)
					length = line->fcs[fore].offset - place;

				if (back < line->fc_count
				&&  line->fcs[back].offset - place < length)
					length = line->fcs[back].offset - place;

				if (font < line->fc_count
				&&  line->fcs[font].offset - place < length)
					length = line->fcs[font].offset - place;

				if (checkSelection)
				{
					// case 1: current line marks beginning of selection
					if (i == sp_start.line)
					{
						// if we're just prior to the selection, clip length to only
						// draw up to the selection start
						if (place + length >= sp_start.offset && place < sp_start.offset)
						{
							length = sp_start.offset - place;
							drawSelection = false;
						}
						// we're at the selection, switch drawing color mode
						else if (place >= sp_start.offset)
						{
							if (sp_end.line == sp_start.line)
							{
								if (place < sp_end.offset)
								{
									drawSelection = true;
									if ((sp_end.offset - place) < length)
										length = sp_end.offset - place;
								}
								else
									drawSelection = false;
							}
							else
								drawSelection = true;
						}
						else
							drawSelection = false;

					}
					// case 2: line in between beginning and end of selection,
					// highlight entire line
					else if (i > sp_start.line && i < sp_end.line)
						drawSelection = true;
					// case 3: last line of selection, with multiple lines in between
					else if (i == sp_end.line && i != sp_start.line)
					{
						if (place < (sp_end.offset))
						{
							if (sp_end.offset - place < length)
								length = (sp_end.offset - place);
							drawSelection = true;
						}
						else
							drawSelection = false;
					}
					else
						drawSelection = false;
				}

				if (place + length == line->length)
					--length;

				int16 i (place + length - 1);
				while (line->edges[i] == 0)
					--i;

				r.Set (
					left,
					height,
					line->edges[i] + indent - start,
					height + line->softies[sit].height - 1.0);

				SetDrawingMode (B_OP_COPY);
				if (drawSelection)
					SetLowColor (sel_color);
				else
					SetLowColor (low_color);
				SetHighColor (hi_color);
				FillRect (r, B_SOLID_LOW);

				if (drawSelection)
					SetHighColor (sel_text);

				SetDrawingMode (B_OP_COPY);

				DrawString (
					line->text + place,
					min_c (length, line->length - place - 1),
					BPoint (left, height + line->softies[sit].ascent));

				left = line->edges[i] + indent - start;

				if ((place += length) + 1 >= line->length)
					++place;
			}


			// Margin after text
			SetDrawingMode (B_OP_COPY);
			SetLowColor (view_color);
			FillRect (
				BRect (
					left + 1.0,
					height,
					bounds.right,
					height + line->softies[sit].height - 1.0),
				B_SOLID_LOW);

			height += line->softies[sit].height;

			if (sit == 0)
				indent += (MARGIN_INDENT / 2.0);
		}
	}

	theme->ReadUnlock();
	Window()->EnableUpdates();
	ConstrainClippingRegion (NULL);
}

void
RunView::SetViewColor (rgb_color color)
{
	assert (memcmp (&color, &B_TRANSPARENT_COLOR, sizeof (rgb_color)) != 0);
	BView::SetViewColor (color);
}

void
RunView::BuildPopUp (void)
{
  // This function checks certain criteria (text is selected,
  // TextView is editable, etc) to determine which MenuItems
  // to enable and disable

  bool enablecopy (true),
       enableselectall (true),
       enablelookup (false);
  BString querystring ("");

  if (sp_start == sp_end)
    enablecopy = false; // no selection

  if (!line_count)
    enableselectall = false;

  if (enablecopy)
  {
    enablelookup = true; // has a selection less than 32 chars long
    GetSelectionText(querystring);
  }

  myPopUp = new BPopUpMenu ("IRCView Context Menu", false, false);

  BMenuItem *item;

  BMessage *lookup;
  lookup = new BMessage (M_LOOKUP_WEBSTER);
  lookup->AddString ("string", querystring);
  item = new BMenuItem("Lookup (Dictionary)", lookup);
  item->SetEnabled (enablelookup);
  item->SetTarget (Parent());
  myPopUp->AddItem (item);

  lookup = new BMessage (M_LOOKUP_GOOGLE);
  lookup->AddString ("string", querystring);
  item = new BMenuItem("Lookup (Google)", lookup);
  item->SetEnabled (enablelookup);
  item->SetTarget (Parent());
  myPopUp->AddItem (item);

  myPopUp->AddSeparatorItem();

  item = new BMenuItem("Copy", new BMessage (B_COPY), 'C');
  item->SetEnabled (enablecopy);
  item->SetTarget (this);
  myPopUp->AddItem (item);

  item = new BMenuItem("Select All", new BMessage (B_SELECT_ALL), 'A');
  item->SetEnabled (enableselectall);
  item->SetTarget (this);
  myPopUp->AddItem (item);

  myPopUp->SetFont (be_plain_font);
}

uint16
RunView::CheckClickCount(BPoint point)
{
	static int clickCount (1);
	// check time and proximity
	BPoint delta = point - lastClick;

	bigtime_t sysTime;
	Window()->CurrentMessage()->FindInt64("when", &sysTime);

	bigtime_t timeDelta = sysTime - lastClickTime;

	bigtime_t doubleClickSpeed;
	get_click_speed(&doubleClickSpeed);

	lastClickTime = sysTime;

	if (timeDelta < doubleClickSpeed
		&& fabs(delta.x) < doubleClickThresh
		&& fabs(delta.y) < doubleClickThresh)
		  return (++clickCount);

	lastClick = point;
	clickCount = 1;
	return clickCount;
}

bool
RunView::CheckClickBounds (const SelectPos &s, const BPoint &point) const
{
	return ((point.x <= lines[s.line]->edges[lines[s.line]->length - 1])
			&& (point.y <= lines[s.line]->bottom));
}

void
RunView::MouseDown (BPoint point)
{
	if (!line_count)
		return;

	BMessage *msg (Window()->CurrentMessage());
	uint32 buttons;
	uint32 modifiers;
	uint16 clicks = CheckClickCount (point) % 3;
	msg->FindInt32 ("buttons", reinterpret_cast<int32 *>(&buttons));
	msg->FindInt32 ("modifiers", reinterpret_cast<int32 *>(&modifiers));

	SelectPos s (PositionAt (point));
	bool inBounds (CheckClickBounds (s, point));

	if (buttons == B_SECONDARY_MOUSE_BUTTON
	&&		(modifiers & B_SHIFT_KEY) == 0
	&&		(modifiers & B_COMMAND_KEY) == 0
	&&		(modifiers & B_CONTROL_KEY) == 0
	&&		(modifiers & B_OPTION_KEY) == 0
	&&      (modifiers & B_MENU_KEY) == 0)
	{
		SelectPos start (s),
					end (s);

		// select word
		if (inBounds && !IntersectSelection (s,s))
		{
			lines[s.line]->SelectWord (&start.offset, &end.offset);

			Select (start, end);
		}

    	BuildPopUp();
    	myPopUp->Go (
    	  ConvertToScreen (point),
    	  true,
    	  false);

    	delete myPopUp;
    	myPopUp = 0;
    	return;
	}

	if (buttons == B_PRIMARY_MOUSE_BUTTON
	&&      (modifiers & B_SHIFT_KEY)   == 0
	&&      (modifiers & B_COMMAND_KEY) == 0
	&&      (modifiers & B_CONTROL_KEY) == 0
	&&      (modifiers & B_OPTION_KEY)  == 0
	&&      (modifiers & B_MENU_KEY)    == 0)
	{
		SelectPos start (s),
					end (s);

		switch (clicks)
		{
			case 2:
			{
				if (inBounds)
				{
					// select word
					lines[s.line]->SelectWord (&start.offset, &end.offset);

					Select (start, end);
					return;
				}
			}
			break;

			case 0:
			{
				if (inBounds)
				{
					start.offset = 0;
					end.offset = lines[s.line]->length - 1;
					Select (start, end);
					return;
				}
			}
			break;

			default:
			{
				if (!inBounds || !IntersectSelection (s, s))
					Select (s,s);
				SetMouseEventMask (B_POINTER_EVENTS);
				tracking = 1;
				track_offset = s;
				return;
			}
		}
	}
	else if (buttons					== B_PRIMARY_MOUSE_BUTTON
	&&      (modifiers & B_SHIFT_KEY)   != 0
	&&      (modifiers & B_COMMAND_KEY) == 0
	&&      (modifiers & B_CONTROL_KEY) == 0
	&&      (modifiers & B_OPTION_KEY)  == 0
	&&      (modifiers & B_MENU_KEY)    == 0)
	{
		if (s.line < sp_start.line || s.offset < sp_start.offset)
		{
			Select (s, sp_end);
			track_offset = SelectPos (sp_end.line, (sp_end.offset > 0) ? sp_end.offset - 1 : sp_end.offset);
		}
		else
		{
			Select (sp_start, s);
			track_offset = sp_start;
		}

		SetMouseEventMask (B_POINTER_EVENTS);
		tracking = 2;
	}
}

void
RunView::CheckURLCursor (BPoint point)
{
	if (!line_count)
		return;

	SelectPos s = PositionAt (point);

	if (!lines[s.line]->urls)
	{
		// if there aren't any URLs in the current line, go back to default
		SetViewCursor (B_CURSOR_SYSTEM_DEFAULT);
		return;
	}

	urllist::const_iterator it;
	for (it = lines[s.line]->urls->begin(); it != lines[s.line]->urls->end(); ++it)
		if ((s.offset >= (*it)->offset)
		 && (s.offset <= (*it)->offset + (*it)->length))
		 {
		 	SetViewCursor (URLCursor);
		 	return;
		 }

	// no URLs found, set back to default
	SetViewCursor (B_CURSOR_SYSTEM_DEFAULT);
}

void
RunView::MouseMoved (BPoint point, uint32 transit, const BMessage *msg)
{
	if (tracking == 0
	&&  line_count
	&& (transit == B_ENTERED_VIEW
	||  transit == B_INSIDE_VIEW))
		CheckURLCursor (point);


	if (!line_count || tracking == 0)
	{
		BView::MouseMoved (point, transit, msg);
		return;
	}

	switch (transit)
	{
		case B_ENTERED_VIEW:
			if (off_view_runner)
			{
				delete off_view_runner;
				off_view_runner = 0;
			}

			if (tracking == 1 || tracking == 2)
				ExtendTrackingSelect (point);

			break;

		case B_EXITED_VIEW:
			if (tracking == 1 || tracking == 2)
				ShiftTrackingSelect (point, true, OFFVIEW_TIMER);
			break;

		case B_OUTSIDE_VIEW:

			if (tracking == 1 || tracking == 2)
			{
				bigtime_t now (system_time());

				ShiftTrackingSelect (
					point,
					false,
					max_c (0LL, min_c (OFFVIEW_TIMER, OFFVIEW_TIMER - (now - off_view_time))));
			}
			break;

		case B_INSIDE_VIEW:

			if ((tracking == 1) && (sp_start != sp_end))
			{
				BMessage msg (B_MIME_DATA);
				BString text;

				GetSelectionText (text);
				msg.AddData (
					"text/plain",
					B_MIME_TYPE,
					text.String(),
					text.Length() + 1);

				BString clip_name (" Clipping");

				if (clipping_name)
					clip_name.Prepend (clipping_name);
				else
					clip_name.Prepend ("RunView");

				msg.AddString ("be:clip_name", clip_name.String());
				msg.AddInt32 ("be:actions", B_COPY_TARGET);

				BRect frame (
					lines[sp_start.line]->edges[sp_start.offset],
					lines[sp_start.line]->top,
					lines[sp_end.line]->edges[sp_end.offset],
					lines[sp_end.line]->bottom);

				if (sp_start.line != sp_end.line)
				{
					frame.left = 0.0;
					frame.right = Bounds().right;
				}
				// selection lies within the bounds of a line, check
				// if it lines on one of the wrapped sublines and calculate rectangle
				// appropriately
				else
				{
					Line *line (lines[sp_start.line]);
					float left (line->edges[sp_start.offset]),
						top (line->top),
						right (line->edges[sp_end.offset]),
						bottom (line->bottom);
					int top_softie (0), bottom_softie (0);
					bool start_found (false);
					bool end_found (false);

					if (line->softie_used)
					{
						if (sp_start.offset < line->softies[0].offset)
							start_found = true;

						if (sp_end.offset < line->softies[0].offset)
							end_found = true;
					}

					if (!end_found)
					for (int16 sit = 1; sit < line->softie_used; ++sit)
					{
						if (!start_found && sp_start.offset < line->softies[sit].offset)
						{
							left = line->edges[sp_start.offset] -
								line->edges[line->softies[sit-1].offset];

							top += (sit) * line->softies[sit].height;
							top_softie = sit;
							start_found = true;
						}

						if (sp_end.offset < line->softies[sit].offset)
						{
							right = line->edges[sp_end.offset] -
								line->edges[line->softies[sit-1].offset];

							bottom = top + (sit - top_softie + 1) * line->softies[sit].height;
							bottom_softie = sit;
							end_found = true;
							break;
						}
					}
					if (!end_found)
					{
						int32 soft_count = (line->softie_used >= 2) ?
							line->softie_used - 2 : 0;
						right = line->edges[line->length - 1] -
							line->edges[line->softies[soft_count].offset];
						bottom_softie = soft_count - 2;

					}
					if (right < left || (bottom_softie - top_softie) > 0)
					{
						left = 0.0;
						right = Bounds().right;
					}

					frame.Set (left, top, right, bottom);
					frame.OffsetBy (MARGIN_WIDTH / 2.0, 0.0);
				}

				if (frame.Height() > Bounds().Height())
					frame = Bounds();

				DragMessage (&msg, frame);

				tracking = 3;
			}
			else if (tracking == 1 || tracking == 2)
				ExtendTrackingSelect (point);
			break;
	}
}

void
RunView::MouseUp (BPoint point)
{
	SelectPos s (PositionAt (point));
	bool url_handle (false);

	if (!line_count)
	{
		tracking = 0;
		return;
	}

	if (tracking == 1)
	{
		if (lines[s.line]->urls)
		{
			urllist::const_iterator it;
			for (it = lines[s.line]->urls->begin(); it != lines[s.line]->urls->end(); ++it)
				if ((s.offset >= (*it)->offset)
				 && (s.offset <= (*it)->offset + (*it)->length))
				 {
				 	vision_app->LoadURL ((*it)->url.String());
				 	url_handle = true;
				 	break;
				 }
		}
		if (!url_handle && s == track_offset)
			Select (s, s);
	}

	if (off_view_runner)
	{
		delete off_view_runner;
		off_view_runner = 0;
	}

	tracking = 0;

}

void
RunView::ExtendTrackingSelect (BPoint point)
{
	SelectPos s (PositionAt (point));

	if (s.line < track_offset.line || (s.line == track_offset.line && s.offset < track_offset.offset))
	{
		Select (s, track_offset);
		tracking = 2;
	}
	else if (s.line > track_offset.line || (s.line == track_offset.line && s.offset > track_offset.offset))
	{
		Select (track_offset, s);
		tracking = 2;
	}
}

void
RunView::ShiftTrackingSelect (BPoint point, bool move, bigtime_t timer)
{
	BRect bounds (Bounds());

	if (off_view_runner)
	{
		delete off_view_runner;
		off_view_runner = 0;
	}

	if (point.y < bounds.top)
	{
		if (bounds.top > 0.0)
		{
			float delta (bounds.top - point.y);

			if (off_view_runner == 0)
			{
				BMessage *msg (new BMessage (M_OFFVIEW_SELECTION));

				msg->AddFloat ("delta", delta);
				msg->AddBool  ("bottom", false);
				msg->AddPoint ("point", point);

				off_view_runner = new BMessageRunner (
					BMessenger (this),
					msg,
					timer == 0LL ? OFFVIEW_TIMER : timer);
			}

			if (move || timer == 0)
			{
				delta = max_c (ABS (ceil (delta / 2.0)), 10.0);
				delta = min_c (delta, Bounds().Height());

				if (bounds.top - delta < 0.0)
					delta = bounds.top;

				ScrollBy (0.0, -delta);
				off_view_time = system_time();
			}
		}

		point.x = 0.0;
		point.y = Bounds().top;
		ExtendTrackingSelect (point);
	}

	if (point.y > bounds.bottom)
	{
		Line *line (lines[line_count-1]);
		if (line
		&&  line->bottom > bounds.bottom)
		{
			float delta (point.y - bounds.bottom);

			if (off_view_runner == 0)
			{
				BMessage *msg (new BMessage (M_OFFVIEW_SELECTION));

				msg->AddFloat ("delta", delta);
				msg->AddBool  ("bottom", true);
				msg->AddPoint ("point", point);

				off_view_runner = new BMessageRunner (
					BMessenger (this),
					msg,
					timer == 0LL ? OFFVIEW_TIMER : timer);
			}

			if (move || timer == 0)
			{
				delta = max_c (ABS (ceil (delta / 2.0)), 10.0);
				delta = min_c (delta, Bounds().Height());

				if (bounds.bottom + delta > line->bottom)
					delta = line->bottom - bounds.bottom;

				ScrollBy (0.0, delta);
				off_view_time = system_time();
			}
		}

		point.x = Bounds().right;
		point.y = Bounds().bottom;
		ExtendTrackingSelect (point);
	}
}

void
RunView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_FOREGROUND_CHANGE:
		case M_BACKGROUND_CHANGE:
		    if (!IsHidden())
				Invalidate (Bounds());
			break;

		case M_FONT_CHANGE:
		{
			Theme *save (theme);

			theme = NULL;
			SetTheme (save);
			break;
		}

		case B_SELECT_ALL:
			SelectAll();
			break;

		case B_COPY:
			if (sp_start != sp_end
			&&  be_clipboard->Lock())
			{
				BString text;
				GetSelectionText (text);

				be_clipboard->Clear();

				BMessage *msg (be_clipboard->Data());
				msg->AddData ("text/plain", B_MIME_TYPE, text.String(), text.Length());

				be_clipboard->Commit();
				be_clipboard->Unlock();
			}
			break;

		case M_OFFVIEW_SELECTION:
		{
			BPoint point;
			float delta;
			bool bottom;

			msg->FindPoint ("point", &point);
			msg->FindBool ("bottom", &bottom);
			msg->FindFloat ("delta", &delta);

			if (bottom)
				point.y = Bounds().bottom + delta;
			else
				point.y = Bounds().top - delta;

			ShiftTrackingSelect (point, true, OFFVIEW_TIMER);
			break;
		}

		default:
			BView::MessageReceived (msg);
	}
}

void
RunView::ResizeRecalc (void)
{
	float width (Bounds().Width() - MARGIN_WIDTH);
	int16 softie_size (0), softie_used (0);
	SoftBreak *softies (NULL);
	BRect bounds (Bounds());
	BRegion region;
	float top (0.0);

	theme->ReadLock();
	for (int16 i = 0; i < line_count; ++i)
	{
		float old_top (lines[i]->top), old_bottom (lines[i]->bottom);
		if (softie_size < lines[i]->softie_used)
		{
			delete [] softies;
			softies = new SoftBreak [softie_size = lines[i]->softie_size];
		}

		softie_used = lines[i]->softie_used;
		memcpy (softies, lines[i]->softies, (softie_used * sizeof (SoftBreak)));

		lines[i]->top = top;
		lines[i]->SoftBreaks (theme, width);
		top = lines[i]->bottom + 1.0;

		BRect r (0.0, lines[i]->top, bounds.right, lines[i]->bottom);

		if (bounds.Intersects (r)
		&& (old_top != lines[i]->top
		||  old_bottom != lines[i]->bottom
		||  softie_used != lines[i]->softie_used
		||  memcmp (softies, lines[i]->softies, softie_used * sizeof (SoftBreak))))
			region.Include (r);
	}

	theme->ReadUnlock();

	if (Window())
	{
		if (RecalcScrollBar (true))
			Invalidate (Bounds());
		else
		{
			int32 count (region.CountRects()), i;

			for (i = 0; i < count; ++i)
				Invalidate (region.RectAt (i));

			if (top <= bounds.bottom)
			{
				BRect r (bounds);

				r.top = top;
				Invalidate (r);
			}
		}

		Window()->Sync();
	}

	if (working) working->top = top;
	delete [] softies;
}

void
RunView::FontChangeRecalc (void)
{
	float width (Bounds().Width() - MARGIN_WIDTH);
	float top (0.0);

	for (int16 i = 0; i < line_count; ++i)
	{
		lines[i]->top = top;

		lines[i]->FigureSpaces();
		lines[i]->FigureEdges (theme, width);

		top = lines[i]->bottom + 1.0;
	}

	if (working)
		working->top = top;

	RecalcScrollBar (false);
	if (!IsHidden())
		Invalidate (Bounds());
	if (Window()) Window()->UpdateIfNeeded();
}

bool
RunView::RecalcScrollBar (bool constrain)
{
	BScrollBar *bar;

	if (scroller == NULL
	|| (bar = scroller->ScrollBar (B_VERTICAL)) == NULL)
		return false;

	float value (bar->Value());
	BRect bounds (Bounds());
	bool changed (false);
	float bottom (0.0);
	float min, max;

	bar->GetRange (&min, &max);

	if (line_count
	&& (bounds.Contains (BPoint (0.0, 0.0)) == false
	||  bounds.Contains (BPoint (0.0, lines[line_count - 1]->bottom)) == false))
	{
		bottom = lines[line_count - 1]->bottom + 5.0;
		bar->SetProportion (bounds.Height() / bottom);
		bar->SetSteps (10.0, bounds.Height());

		bottom -= bounds.Height();
	}

	// We don't want the bar to cause a draw/copybits, so we restrict the
	// clipping region to nothing

	if (constrain)
	{
		BRegion region;
		ConstrainClippingRegion (&region);
	}

	if (max != bottom)
	{
		bar->SetRange (0.0, bottom);

		if (value == max || min == max)
		{
			bar->SetValue (bottom);
			changed = true;
		}
	}

	if (constrain)
		ConstrainClippingRegion (NULL);

	return changed;
}

void
RunView::Append (
	const char *buffer,
	int16 fore,
	int16 back,
	int16 font)
{
	Append (buffer, strlen (buffer), fore, back, font);
}

void
RunView::Append (
	const char *buffer,
	int32 len,
	int16 fore,
	int16 back,
	int16 font)
{
	float width (Bounds().Width() - 10);
	int32 place (0);

	assert (fore != Theme::TimestampFore);
	assert (back != Theme::TimestampBack);
	assert (font != Theme::TimestampFont);
	assert (fore != Theme::TimespaceFore);
	assert (back != Theme::TimespaceBack);
	assert (font != Theme::TimespaceFont);
	assert (back != Theme::SelectionBack);

	theme->ReadLock();


	while (place < len)
	{
		int32 end (place);

		while (end < len && buffer[end] != '\n')
			++end;

		if (end < len) ++end;

		if (working)
		{
			URLCrunch crunch (buffer + place, end - place);
			BString temp;
			int32 url_offset (0),
				last_offset (0);


			while ((url_offset = crunch.Crunch (&temp)) != B_ERROR)
			{
				working->Append  (buffer + place,
									(url_offset - last_offset),
									width,
									theme,
									fore,
									back,
									font);

				working->Append  (temp.String(),
									temp.Length(),
									width,
									theme,
									C_URL,
									back,
									F_URL);

				place += (url_offset - last_offset) + temp.Length();
				last_offset = url_offset + temp.Length();
			}

			if (place < end)
				working->Append (
				buffer + place,
				end - place,
				width,
				theme,
				fore,
				back,
				font);
		}
		else
		{
			float top (0.0);

			if (line_count > 0)
				top = lines[line_count - 1]->bottom + 1.0;


			working = new Line (
				buffer + place,
				0,
				top,
				width,
				theme,
				stamp_format,
				fore,
				back,
				font);

			URLCrunch crunch (buffer + place, end - place);
			BString temp;
			int32 url_offset (0),
				last_offset (0);

			while ((url_offset = crunch.Crunch (&temp)) != B_ERROR)
			{
				working->Append  (buffer + place,
									(url_offset - last_offset),
									width,
									theme,
									fore,
									back,
									font);

				working->Append  (temp.String(),
									temp.Length(),
									width,
									theme,
									C_URL,
									back,
									F_URL);

				place += (url_offset - last_offset) + temp.Length();
				last_offset = url_offset + temp.Length();
			}

			if (place < end)
				working->Append (buffer + place,
									end - place,
									width,
									theme,
									fore,
									back,
									font);
		}

		if (working->length
		&&  working->text[working->length - 1] == '\n')
		{
			bool chopped;

			if (Window()) Window()->DisableUpdates();

			if ((chopped = line_count == LINE_COUNT))
			{
				Line *first (lines[0]);
				float shift (first->bottom + 1);

				for (int16 i = 1; i < LINE_COUNT; ++i)
				{
					lines[i]->top    -= shift;
					lines[i]->bottom -= shift;

					lines[i - 1] = lines[i];
				}

				working->top    -= shift;
				working->bottom -= shift;

				delete first;
				
				if (sp_start.line > 0)
					sp_start.line--;
				else
					sp_start.offset = 0;
				
				if (sp_end.line > 0)
					sp_end.line--;
				else
					sp_end.offset = 0;

				// Recalc the scrollbar so that we have clean drawing
				// after the line has been removed
				--line_count;
				RecalcScrollBar (true);
			}

			lines[line_count++] = working;
			RecalcScrollBar (true);

			Invalidate (Bounds());

			if (Window())
			{
				Window()->EnableUpdates();
				Window()->UpdateIfNeeded();
			}

			working = NULL;
		}

		place = end;
	}

	theme->ReadUnlock();
}

void
RunView::Clear (void)
{
	for (int16 i = 0; i < line_count; ++i)
		delete lines[i];

	line_count = 0;
	RecalcScrollBar (true);
	Invalidate();

	sp_start.line = 0;
	sp_start.offset = 0;
	sp_end = sp_start;

	if (working)
		working->top = 0.0;
}

int16
RunView::LineCount (void) const
{
	return line_count;
}

const char *
RunView::LineAt (int16 which) const
{
	if (which < 0 || which >= line_count)
		return NULL;

	return lines[which]->text;
}

void
RunView::SetTimeStampFormat (const char *format)
{
	if ((format == NULL
	&&   stamp_format == NULL)
	||  (format != NULL
	&&   stamp_format != NULL
	&&   strcmp (format, stamp_format) == 0))
		return;

	bool was_on (false);

	if (stamp_format)
	{
		delete [] stamp_format;
		stamp_format = NULL;
		was_on = true;
	}

	if (format)
		stamp_format = strcpy (new char [strlen (format) + 1], format);

	float width (Bounds().Width() - MARGIN_WIDTH);
	float top (0.0);

	theme->ReadLock();
	for (int16 i = 0; i < line_count; ++i)
	{
		lines[i]->top = top;

		lines[i]->SetStamp (stamp_format, was_on);
		lines[i]->FigureSpaces();
		lines[i]->FigureEdges(theme, width);

		top = lines[i]->bottom + 1.0;
	}
	theme->ReadUnlock();

	if (working)
	{
		working->top = top;
		working->SetStamp (stamp_format, was_on);
	}

	RecalcScrollBar (false);
	Invalidate (Bounds());
	if (Window()) Window()->UpdateIfNeeded();
}

void
RunView::SetTheme (Theme *t)
{
	if (t == NULL || theme == t)
		return;

	theme = t;

	if (IsHidden())
	{
		fontsdirty = true;
		return;
	}
	FontChangeRecalc();
}

SelectPos
RunView::PositionAt (BPoint point) const
{
	int16 i, lindex (0);
	SelectPos pos (-1, 0);

	if (line_count == 0)
		return pos;

	// find the line
	for (i = 0; i < line_count; ++i)
	{
		if (lines[i]->top > point.y)
			break;
		lindex = i;
	}

	// check to make sure we actually did find a line and not just run into line_count
	if (lines[lindex]->bottom < point.y)
	{
		pos.line = line_count - 1;
		pos.offset = lines[line_count - 1]->length;
		return pos;
	}

	float height (lines[lindex]->top);
	int16 sindex (0);

	for (i = 0; i < lines[lindex]->softie_used; ++i)
	{
		if (height > point.y)
			break;

		sindex = i;
		height += lines[lindex]->softies[i].height;
	}

	float margin (MARGIN_WIDTH / 2.0);
	float width (0);
	int16 start (0);

	if (sindex)
	{
		int16 offset (lines[lindex]->softies[sindex - 1].offset);

		width = lines[lindex]->edges[offset];
		start = offset + UTF8_CHAR_LEN (lines[lindex]->text[offset]);
	}

	for (i = start; i <= lines[lindex]->softies[sindex].offset; ++i)
		if (lines[lindex]->edges[i] + margin - width >= point.x)
			break;

	pos.line = lindex;
	pos.offset = min_c (i, lines[lindex]->softies[sindex].offset);
	if (pos.offset > 0) pos.offset += UTF8_CHAR_LEN (lines[pos.line]->text[pos.offset]);

	return pos;
}

BPoint
RunView::PointAt (SelectPos s) const
{
	return BPoint(lines[s.line]->top + lines[s.line]->bottom / 2.0, lines[s.line]->edges[s.offset]);
}

void
RunView::GetSelectionText (BString &string) const
{
	if (sp_start == sp_end)
		return;

	if (sp_start.line == sp_end.line)
	{
		const char *line (LineAt (sp_start.line));
		string.Append (line + sp_start.offset, sp_end.offset - sp_start.offset);
		return;
	}

	for (int32 i = sp_start.line; i <= sp_end.line; i++)
	{
		const char *line (LineAt (i));
		if (i == sp_start.line)
		{
			line += sp_start.offset;
			string.Append (line);
		}
		else if (i == sp_end.line)
		{
			string.Append (line, sp_end.offset);
			break;
		}
		else
			string.Append (line);
	}
}

bool
RunView::IntersectSelection (const SelectPos &start, const SelectPos &end) const
{
	if (sp_start.line == sp_end.line)
	{
		if (start.line == sp_start.line && start.offset >= sp_start.offset && start.offset < sp_end.offset)
			return true;
		if (end.line == sp_start.line && end.offset >= sp_start.offset && end.offset < sp_end.offset)
			return true;
	}
	else
	{
		if (start.line > sp_start.line && start.line < sp_end.line)
			return true;
		if (end.line > sp_start.line && end.line < sp_end.line)
			return true;
		if (start.line == sp_start.line && start.offset >= sp_start.offset)
			return true;
		if (end.line == sp_start.line && end.offset >= sp_start.offset)
			return true;
		if (start.line == sp_end.line && start.offset < sp_end.offset)
			return true;
		if (end.line == sp_end.line && end.offset < sp_end.offset)
			return true; 
	}

	return false;
}

BRect
RunView::GetTextFrame(const SelectPos &start, const SelectPos &end) const
{
	return BRect (0.0, lines[(start.line > 0) ? (start.line - 1) : 0]->top,
		Bounds().Width(), lines[end.line]->bottom);
}

void
RunView::Select (const SelectPos &start, const SelectPos &end)
{
	if (sp_start != sp_end)
	{
		if (start == end || !IntersectSelection (start, end))
		{
			BRect frame (GetTextFrame (sp_start, sp_end));

			sp_start = start;
			sp_end   = start;
			Invalidate (frame);
		}
		else
		{
			if (sp_start.line < start.line || (sp_start.line == start.line && sp_start.offset < start.offset))
			{
				BRect frame (GetTextFrame (sp_start, start));

				sp_start = start;
				Invalidate (frame);
			}

			if (end.line < sp_end.line || (sp_end.line == end.line && end.offset < sp_end.offset))
			{
				BRect frame (GetTextFrame (end, sp_end));

				sp_end = end;
				Invalidate (frame);
			}
		}
	}

	if (sp_start == sp_end)
	{
		sp_start = start;
		sp_end   = end;

		if (sp_start != sp_end)
		{
			BRect frame (GetTextFrame (start, end));

			Invalidate (frame);
		}
	}
	else // extension
	{
		if (start.line < sp_start.line || (start.line == sp_start.line && start.offset < sp_start.offset))
		{
			BRect frame (GetTextFrame (start, sp_start));

			sp_start = start;
			Invalidate (frame);
		}

		if (end.line > sp_end.line || (end.line == sp_end.line && end.offset > sp_end.offset))
		{
			BRect frame (GetTextFrame (sp_end, end));

			sp_end = end;
			Invalidate (frame);
		}
	}
}

void
RunView::SelectAll (void)
{
	if (line_count)
	{
		sp_start = SelectPos (0, 0);
		sp_end = SelectPos (line_count-1, lines[line_count-1]->length);
		Invalidate(Bounds());
	}
}

void
RunView::SetClippingName (const char *name)
{
	delete [] clipping_name;
	clipping_name = new char[strlen(name) + 1];
	memcpy (clipping_name, name, strlen(name));
	clipping_name[strlen(name)] = '\0';
}

Line::Line (
	const char *buffer,
	int16 len,
	float top,
	float width,
	Theme *theme,
	const char *stamp_format,
	int16 fore,
	int16 back,
	int16 font)
	:	text (NULL),
		stamp (time(NULL)),
		urls (NULL),
		spaces (NULL),
		edges (NULL),
		fcs (NULL),
		softies (NULL),
		top (top),
		bottom (0.0),
		length (len),
		space_count (0),
		edge_count (0),
		fc_count (0),
		softie_size (0),
		softie_used (0)
{
	// Very important to call SetStamp before text is allocated (avoids reallocation)
	size_t stampsize (SetStamp (stamp_format, false));

	if (text == NULL)
		text = new char [length + 1];

	memcpy (text + stampsize, buffer, length - stampsize);
	text[length] = '\0';

	FigureFontColors (stampsize, fore, back, font);

	if (text[length - 1] == '\n')
	{
		FigureSpaces();
		FigureEdges (theme, width);
	}
}

Line::~Line (void)
{
	delete [] spaces;
	delete [] edges;
	delete [] fcs;
	delete [] text;

	if (urls)
	{
		urllist::const_iterator it = urls->begin();
		for (; it != urls->end(); ++it)
			delete (*it);
		delete urls;
	}
}

void
Line::Append (
	const char *buffer,
	int16 len,
	float width,
	Theme *theme,
	int16 fore,
	int16 back,
	int16 font)
{
	int16 save (length);
	char *new_text;

	new_text = new char [length + len + 1];

	memcpy (new_text, text, length);
	memcpy (new_text + length, buffer, len);
	new_text[length += len] = '\0';

	delete [] text;
	text = new_text;

	FigureFontColors (save, fore, back, font);

	if (fore == C_URL)
	{
		if (!urls)
			urls = new urllist;
		urls->push_front (new URL (buffer, save, len));
	}

	if (text[length - 1] == '\n')
	{
		if (urls)
			urls->sort();

		FigureSpaces();
		FigureEdges (theme, width);
	}
	
    
}

void
Line::FigureSpaces (void)
{
	const char spacers[] = " \t\n-\\/";
	const char *buffer (text);
	size_t offset (0), n;
	int16 count (0);

	delete [] spaces;
	space_count = 0;
	while ((n = strcspn (buffer + offset, spacers)) < length - offset)
	{
		++count;
		offset += n + 1;
	}

	spaces = new int16 [count];

	offset = 0;
	while ((n = strcspn (buffer + offset, spacers)) < length - offset)
	{
		spaces[space_count++] = n + offset;
		offset += n + 1;
	}
}

void
Line::FigureFontColors (
	int16 pos,
	int16 fore,
	int16 back,
	int16 font)
{
	if (fc_count)
	{
		int16 last_fore = -1;
		int16 last_back = -1;
		int16 last_font = -1;
		int16 i;

		// we have fcs, so we backtrack for last of each which
		for (i = fc_count - 1; i >= 0; --i)
		{
			if (last_fore < 0
			&&  fcs[i].which == FORE_WHICH)
				last_fore = i;
			else if (last_back < 0
			&&       fcs[i].which == BACK_WHICH)
				last_back = i;
			else if (last_font < 0
			&&       fcs[i].which == FONT_WHICH)
				last_font = i;

			if (last_fore >= 0
			&&  last_back >= 0
			&&  last_font >= 0)
				break;
		}

		// now figure out how many more we need
		int16 count = 0;
		if (fcs[last_fore].index != fore)
			++count;
		if (fcs[last_back].index != back)
			++count;
		if (fcs[last_font].index != font)
			++count;

		if (count)
		{
			FontColor *new_fcs;
			new_fcs = new FontColor [fc_count + count];
			memcpy (new_fcs, fcs, fc_count * sizeof (FontColor));
			delete [] fcs;
			fcs = new_fcs;

			if (fcs[last_fore].index != fore)
			{
				fcs[fc_count].which = FORE_WHICH;
				fcs[fc_count].offset = pos;
				fcs[fc_count].index = fore;
				++fc_count;
			}

			if (fcs[last_back].index != back)
			{
				fcs[fc_count].which = BACK_WHICH;
				fcs[fc_count].offset = pos;
				fcs[fc_count].index = back;
				++fc_count;
			}

			if (fcs[last_font].index != font)
			{
				fcs[fc_count].which = FONT_WHICH;
				fcs[fc_count].offset = pos;
				fcs[fc_count].index = font;
				++fc_count;
			}
		}
	}
	else
	{
		fcs = new FontColor [fc_count = 3];
		fcs[0].which = FORE_WHICH;
		fcs[0].offset = 0;
		fcs[0].index = fore;
		fcs[1].which = BACK_WHICH;
		fcs[1].offset = 0;
		fcs[1].index = back;
		fcs[2].which = FONT_WHICH;
		fcs[2].offset = 0;
		fcs[2].index = font;
	}
}

void
Line::FigureEdges (
	Theme *theme,
	float width)
{
	delete [] edges;
	edges = new float [length];

	int16 cur_fcs (0), next_fcs (0), cur_font (0);

	edge_count = 0;
	while (cur_fcs < fc_count)
	{
		if (fcs[cur_fcs].which == FONT_WHICH)
		{
			cur_font = cur_fcs;
			break;
		}

		++cur_fcs;
	}

	while (cur_fcs < fc_count)
	{
		int16 last_offset (fcs[cur_fcs].offset);
		next_fcs = cur_fcs + 1;

		while (next_fcs < fc_count)
		{
			// We want to break at every difference
			// but, we want to break on a font if available
			if (fcs[next_fcs].offset > last_offset)
			{
				while (next_fcs < fc_count
				&&     fcs[next_fcs].which != FONT_WHICH
				&&     next_fcs + 1 < fc_count
				&&     fcs[next_fcs + 1].offset == fcs[next_fcs].offset)
					++next_fcs;

				break;
			}
			++next_fcs;
		}

		if (fcs[cur_fcs].which == FONT_WHICH)
			cur_font = cur_fcs;

		int16 ccount;
		int16 seglen;

		if (next_fcs == fc_count)
		{
			ccount = CountChars (fcs[cur_fcs].offset, length - fcs[cur_fcs].offset);
			seglen = length - fcs[cur_fcs].offset;
		}
		else
		{
			ccount = CountChars (
				fcs[cur_fcs].offset,
				fcs[next_fcs].offset - fcs[cur_fcs].offset);
			seglen = fcs[next_fcs].offset - fcs[cur_fcs].offset;
		}

		const BFont &f (theme->FontAt (fcs[cur_font].index));

		float eshift[ccount];
		f.GetEscapements (
			text + fcs[cur_fcs].offset,
			ccount,
			eshift);

		// This is not perfect, because we are including the left edge,
		// but BFont::GetEdges doesn't seem to work as we'd like

		int16 i;

		for (i = 0; i < ccount; ++i)
		{
			edges[edge_count + i] = ((edge_count + i > 0) ? edges[edge_count + i - 1] : 0) +
				(eshift[i] * f.Size());
			
			// this little backtracking routine is necessary in the case where an fcs change
			// comes immediately after a UTF8-char, since all but the first edge will be 0
			// and thus the new edge's starting position will be thrown off if we don't
			// backtrack to the beginning of the char
			if ((edge_count + i > 0) && edges[edge_count + i - 1] == 0)
			{
			   int32 temp = edge_count + i - 1;
			   while (edges[--temp] == 0);
			   edges[edge_count + i] += edges[temp];
			}
		}

		for (i = fcs[cur_fcs].offset; i < fcs[cur_fcs].offset + seglen;)
		{
			int32 len (UTF8_CHAR_LEN (text[i]) - 1);

			if (len)
			{
				int16 k;
				for (k = edge_count + ccount - 1; k > i; --k)
					edges[k + len] = edges[k];

				for (k = 1; k <= len; ++k)
					edges[i + k] = 0;

				ccount += len;
			}

			i += len + 1;
		}

		cur_fcs = next_fcs;
		edge_count += ccount;
	}

	SoftBreaks (theme, width);
}

void
Line::SoftBreaks (Theme *theme, float start_width)
{
	float margin (ceil (MARGIN_WIDTH / 2.0));
	float width (start_width);
	float start (0.0);
	int16 text_place (0);
	int16 space_place (0);
	int16 font (0);

	softie_used = 0;
	bottom = top;

	// find first font
	while (font < fc_count && fcs[font].which != FONT_WHICH)
		++font;

	while (text_place < length)
	{
		try
		{
			while (space_place < space_count)
			{
				if ((float)edges[spaces[space_place]] - start > width)
					break;

				++space_place;
			}

			// we've reached the end of the line (but it might not all fit)
			// or we only have one space, so we check if we need to split the word
			if (space_place == space_count
			||  space_place == 0
			||  spaces[space_place - 1] < text_place)
			{
				// everything fits.. how wonderful (but we want at least one softbreak)
				if (edge_count == 0)
					throw SoftBreakEnd (length - 1);

				int16 i (edge_count - 1);

				while (edges[i] == 0)
					--i;

				if (edges[i] - start <= width)
					throw SoftBreakEnd (length - 1);

				// we force at least one character
				// your font may be a little too large for your window!
				text_place += UTF8_CHAR_LEN (text[text_place]);

				while (text_place < length)
				{
					if (edges[text_place] - start > width - margin)
						break;

					text_place += UTF8_CHAR_LEN (text[text_place]);
				}

				throw SoftBreakEnd (text_place);
			}

			// we encountered more than one space, so we rule out having to
			// split the word, if the current word will fit within the bounds
			int16 ccount1, ccount2;

			ccount1 = spaces[space_place - 1];
			ccount2 = spaces[space_place] - spaces[space_place - 1];

			int16 i (ccount1 - 1);
			while (edges[i] == 0)
				--i;

			--space_place; // TODO we probably have to move this up a bit
			if (edges[ccount1 + ccount2] - edges[i] < width - margin)
				throw SoftBreakEnd (spaces[space_place]);

			// We need to break up the really long word
			text_place = spaces[space_place];
			while (text_place < edge_count)
			{
				if ((edges[text_place] - start) > width)
					break;

				text_place += UTF8_CHAR_LEN (text[text_place]);
			}
			--text_place;
		}
		catch (SoftBreakEnd &sbe)
		{
			text_place = sbe.offset;
		}

		if (softie_size < softie_used + 1)
		{
			SoftBreak *new_softies;

			new_softies = new SoftBreak [softie_size += SOFTBREAK_STEP];

			if (softies)
			{
				memcpy (new_softies, softies, sizeof (SoftBreak) * softie_used);
				delete [] softies;
			}

			softies = new_softies;
		}

		// consume whitespace
		while (text_place + 1 < length
		&&     isspace (text[text_place + 1]))
			++text_place;

		softies[softie_used].offset = text_place;
		softies[softie_used].height = 0.0;
		softies[softie_used].ascent = 0.0;

		int16 last (font);
		while (font < fc_count)
		{
			const BFont &f (theme->FontAt (fcs[font].index));
			font_height fh;
			float height;

			f.GetHeight (&fh);

			height = ceil (fh.ascent + fh.descent + fh.leading);
			if (softies[softie_used].height < height)
				softies[softie_used].height = height;
			if (softies[softie_used].ascent < fh.ascent)
				softies[softie_used].ascent = fh.ascent;

			// now try and find next
			while (++font < fc_count)
				if (fcs[font].which == FONT_WHICH)
					break;

			if (font == fc_count
			||  fcs[font].offset > text_place)
			{
				font = last;
				break;
			}

			last = font;
		}

		if (text_place < length)
			start = (float)edges[text_place];

		bottom += softies[softie_used++].height;
		text_place += UTF8_CHAR_LEN (text[text_place]);
		width = start_width - MARGIN_INDENT;
	}

	bottom -= 1.0;
}

int16
Line::CountChars (int16 pos, int16 len)
{
	int16 ccount (0);

	if (pos >= length)
		return ccount;

	if (pos + len > length)
		len = length - pos;

	register int16 i = pos;
	while (i < pos + len)
	{
		i += UTF8_CHAR_LEN(text[i]);
		++ccount;
	}

	return ccount;
}

size_t
Line::SetStamp (const char *format, bool was_on)
{
	size_t size (0);

	if (was_on)
	{
		int16 offset (fcs[4].offset + 1);

		if (urls)
		{
			urllist::const_iterator it = urls->begin();
			for (; it != urls->end(); ++it)
				(*it)->offset -= offset;
		}
		memmove (text, text + offset, length - offset);
		text[length -= offset] = '\0';

		for (int16 i = 6; i < fc_count; ++i)
		{
			fcs[i].offset -= offset;
			fcs[i - 6] = fcs[i];
		}

		fc_count -= 6;
	}

	if (format)
	{
		char buffer[1024];
		struct tm tm;

		localtime_r (&stamp, &tm);
		size = strftime (buffer, 1023, format, &tm);
		if (urls)
		{
			urllist::const_iterator it = urls->begin();
			for (; it != urls->end(); ++it)
				(*it)->offset += size;
		}

		char *new_text;

		new_text = new char [length + size + 2];
		memcpy (new_text, buffer, size);
		new_text[size++] = ' ';
		new_text[size] = '\0';

		if (text)
		{
			memcpy (new_text + size, text, length);
			delete [] text;
		}

		text = new_text;
		text[length += size] = '\0';

		FontColor *new_fcs;
		new_fcs = new FontColor [fc_count + 6];

		if (fcs)
		{
			memcpy (
				new_fcs + 6,
				fcs,
				fc_count * sizeof (FontColor));
			delete [] fcs;
		}
		fcs = new_fcs;
		fc_count += 6;

		fcs[0].which	= FORE_WHICH;
		fcs[0].index	= Theme::TimestampFore;
		fcs[0].offset	= 0;
		fcs[1].which	= BACK_WHICH;
		fcs[1].index	= Theme::TimestampBack;
		fcs[1].offset	= 0;
		fcs[2].which	= FONT_WHICH;
		fcs[2].index	= Theme::TimestampFont;
		fcs[2].offset	= 0;

		fcs[3].which	= FORE_WHICH;
		fcs[3].index	= Theme::TimespaceFore;
		fcs[3].offset	= size - 1;
		fcs[4].which	= BACK_WHICH;
		fcs[4].index	= Theme::TimespaceBack;
		fcs[4].offset	= size - 1;
		fcs[5].which	= FONT_WHICH;
		fcs[5].index	= Theme::TimespaceFont;
		fcs[5].offset	= size - 1;

		for (int16 i = 6; i < fc_count; ++i)
			fcs[i].offset += size;
	}

	return size;
}

void
Line::SelectWord (int16 *start, int16 *end)
{
	int16 start_tmp (*start), end_tmp (*end);

	while(start_tmp > 0 && text[start_tmp-1] != ' ')
			start_tmp--;

	while ((end_tmp - 1) < length && text[end_tmp] != ' ')
			end_tmp++;

	while (end_tmp >= length)
		--end_tmp;

	*start = start_tmp;
	*end = end_tmp;
}

