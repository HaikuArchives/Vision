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

#define SOFTBREAK_STEP			5

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <slist>

#include <Message.h>
#include <Messenger.h>
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
	slist<URL *>			urls;
	int16					*spaces;
	int16					*edges;
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
								BRect **boxbuf,
								int16 *boxbuf_size,
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
								BRect **boxbuf,
								int16 *boxbuf_size,
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
								BRect **boxbuf,
								int16 *boxbuf_size,
								Theme *theme,
								float width);

	void					SoftBreaks (
								Theme * theme,
								float width);

	int16					CountChars (int16 pos, int16 len);
	size_t				SetStamp (const char *, bool);
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
		boxbuf (NULL),
		boxbuf_size (0),
		working (NULL),
		line_count (0),
		stamp_format (NULL),
		sp_start (0, 0),
		sp_end (0, 0),
		resizedirty (false),
		fontsdirty (false)
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

	delete [] boxbuf;
	delete working;
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
	rgb_color low_color, hi_color, view_color;
	float height (frame.bottom);
	BRect bounds (Bounds());
	BRegion clipper;

	clipper.Set (frame);
	ConstrainClippingRegion (&clipper);

	theme->ReadLock();
	view_color = theme->BackgroundAt (Theme::NormalBack);

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
				SetLowColor (low_color);
				SetHighColor (hi_color);
				FillRect (r, B_SOLID_LOW);

				SetDrawingMode (B_OP_OVER);

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
RunView::MouseDown (BPoint point)
{
	sp_start = PositionAt (point);
	slist<URL *>::const_iterator it;
	for (it = lines[sp_start.line]->urls.begin(); it != lines[sp_start.line]->urls.end(); ++it)
		if ((sp_start.offset >= (*it)->offset)
		 && (sp_start.offset < (*it)->offset + (*it)->length))
		 {
		 	vision_app->LoadURL ((*it)->url.String());
		 	break;
		 }
}

void
RunView::MouseMoved (BPoint point, uint32 transit, const BMessage *msg)
{
	if (line_count == 0)
		return;
		
	SelectPos s = PositionAt (point);
		
	slist<URL *>::const_iterator it;
	for (it = lines[s.line]->urls.begin(); it != lines[s.line]->urls.end(); ++it)
		if ((s.offset >= (*it)->offset)
		 && (s.offset < (*it)->offset + (*it)->length))
		 {
		 	SetViewCursor (URLCursor);
		 	return;
		 }
	SetViewCursor (B_CURSOR_SYSTEM_DEFAULT);
}

void
RunView::MouseUp (BPoint point)
{
	sp_end = PositionAt (point);
	if ((sp_end.line < sp_start.line) ||
	  (sp_end.line == sp_start.line) && (sp_end.offset < sp_start.offset))
	{
		SelectPos temp = sp_end;
		sp_end = sp_start;
		sp_start = temp;
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
		lines[i]->FigureEdges (&boxbuf, &boxbuf_size, theme, width);

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
		bottom = lines[line_count - 1]->bottom;
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
	unsigned long lcount (0);
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
									&boxbuf,
									&boxbuf_size,
									width,
									theme,
									fore,
									back,
									font);

				working->Append  (temp.String(),
									temp.Length(),
									&boxbuf,
									&boxbuf_size,
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
				&boxbuf,
				&boxbuf_size,
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
				&boxbuf,
				&boxbuf_size,
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
									&boxbuf,
									&boxbuf_size,
									width,
									theme,
									fore,
									back,
									font);

				working->Append  (temp.String(),
									temp.Length(),
									&boxbuf,
									&boxbuf_size,
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
									&boxbuf,
									&boxbuf_size,
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
		lines[i]->FigureEdges(&boxbuf, &boxbuf_size, theme, width);

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
	SelectPos pos;

	if (line_count == 0)
		return pos;

	// find the line
	for (i = 0; i < line_count; ++i)
	{
		if (lines[i]->top > point.y)
			break;

		lindex = i;
	}

//	printf ("Line: %hd\n", lindex);

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
	int16 width (0);
	int16 start (0);

//	printf ("Softie: %hd\n", sindex);

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

//	printf ("Char: %c\n", lines[lindex]->text[pos.offset]);

	return pos;
}

BPoint
RunView::PointAt (SelectPos) const
{
	return BPoint();
}

void
RunView::GetSelection (SelectPos *, SelectPos *) const
{
}

void
RunView::Select (SelectPos, SelectPos)
{
}

void
RunView::SelectAll (void)
{
}

Line::Line (
	const char *buffer,
	int16 len,
	float top,
	BRect **boxbuf,
	int16 *boxbuf_size,
	float width,
	Theme *theme,
	const char *stamp_format,
	int16 fore,
	int16 back,
	int16 font)
	:	text (NULL),
		stamp (time(NULL)),
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
		FigureEdges (boxbuf, boxbuf_size, theme, width);
	}
}

Line::~Line (void)
{
	delete [] spaces;
	delete [] edges;
	delete [] fcs;
	delete [] text;

	slist<URL *>::const_iterator it = urls.begin();
	for (; it != urls.end(); ++it)
		delete (*it);
}

void
Line::Append (
	const char *buffer,
	int16 len,
	BRect **boxbuf,
	int16 *boxbuf_size,
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

	if (text[length - 1] == '\n')
	{
		URLCrunch crunch (text, length);
		int32 offset (0);
		BString temp_url;
		
		while ((offset = crunch.Crunch (&temp_url)) != B_ERROR)
			urls.push_front (new URL (temp_url.String(), offset, temp_url.Length()));
		
		urls.sort();
		FigureSpaces();
		FigureEdges (boxbuf, boxbuf_size, theme, width);
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
	BRect **boxbuf,
	int16 *boxbuf_size,
	Theme *theme,
	float width)
{
	delete [] edges;
	edges = new int16 [length];

	int16 cur_fcs (0), next_fcs (0), cur_font (0);
	escapement_delta delta = {0.0, 0.0};

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

		if (ccount >= *boxbuf_size)
		{
			delete [] *boxbuf;
			*boxbuf = new BRect [*boxbuf_size = ccount];
		}

		const BFont &f (theme->FontAt (fcs[cur_font].index));

		int16 offset (fcs[cur_fcs].offset);

		f.GetBoundingBoxesAsString (
			text + fcs[cur_fcs].offset,
			ccount,
			B_SCREEN_METRIC,
			&delta,
			*boxbuf);

		float eshift;
		f.GetEscapements (
			text + fcs[cur_fcs].offset + (seglen - 1),
			1,
			&eshift);

		// This is not perfect, because we are including the left edge,
		// but BFont::GetEdges doesn't seem to work as we'd like
		BRect &lastbox (*((*boxbuf) + (ccount - 1)));
		lastbox.right += eshift * f.Size() - lastbox.Width();

		int16 shift (0);

		if (fcs[cur_fcs].offset)
			shift = edges[edge_count - 1];

		int16 i;

		for (i = 0; i < ccount; ++i)
			edges[edge_count + i] = (int16)(((*boxbuf) + i)->right) + shift;
		//edges[edge_count + ccount - 1] += (int16) einfo.right;

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

		slist<URL *>::const_iterator it = urls.begin();
		for (; it != urls.end(); ++it)
			(*it)->offset -= offset;

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

		slist<URL *>::const_iterator it = urls.begin();
		for (; it != urls.end(); ++it)
			(*it)->offset += size;

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

