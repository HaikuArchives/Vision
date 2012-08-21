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
 * Contributor(s): Rene Gollent
 *								 Todd Lair
 *								 Alan Ellis <alan@cgsoftware.org>
 */

#define FORE_WHICH				0
#define BACK_WHICH				1
#define FONT_WHICH				2

#define MARGIN_WIDTH			10.0
#define MARGIN_INDENT			10.0

#define OFFVIEW_TIMER					(10000LL)
#define ABS(x)							(x * ((x<0) ? -1 : 1))
#define SOFTBREAK_STEP			5

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

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

#include "ObjectList.h"
#include "Theme.h"
#include "RunView.h"
#include "URLCrunch.h"
#include "Vision.h"
#include "Utilities.h"


// cursor data for hovering over URLs

static unsigned char URLCursorData[] = {16,1,2,2,
	0,0,0,0,56,0,36,0,36,0,19,224,18,92,9,42,
	8,1,60,33,76,49,66,121,48,125,12,253,2,0,1,0,
	0,0,0,0,56,0,60,0,60,0,31,224,31,252,15,254,
	15,255,63,255,127,255,127,255,63,255,15,255,3,254,1,248
};

struct SoftBreak
{
	int16					fOffset;
	float					fHeight;
	float					fAscent;
};

struct URL
{
	int32					fOffset;
	int32					fLength;
	BString					fUrl;

							URL (const char *address, int32 off, int32 len) :
								fOffset (off),
								fLength (len),
								fUrl (address)
								{ }
};

typedef BObjectList<URL> urllist;

struct SoftBreakEnd
{
	int16			 fOffset;

							SoftBreakEnd (int16 offset)
								:	fOffset (offset)
							{ }
};

struct FontColor
{
	int16					fOffset;
							// G++ is stupid.	We only need 2 bits
							// for fWhich, but the compiler has a bug
							// and warns us against fWhich == 2
	int16					fWhich			: 3;
	int16					fIndex			: 13;
};

struct Line
{
	char					 *fText;
	time_t				 fStamp;
	urllist				*fUrls;
	int16					*fSpaces;
	int16					*fEdges;
	FontColor			*fFcs;
	SoftBreak			*fSofties;
	float					fTop;
	float					fBottom;

	int16					fLength;
	int16					fSpace_count;
	int16					fEdge_count;
	int16					fFc_count;
	int16					fSoftie_size;
	int16					fSoftie_used;

							Line (
								const char *buffer,
								int16 fLength,
								float top,
								float width,
								Theme *fTheme,
								const char *fStamp_format,
								int16 fore,
								int16 back,
								int16 font);

							~Line (void);

	void					Append (
								const char *buffer,
								int16 len,
								float width,
								Theme *fTheme,
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
								Theme *fTheme,
								float width);

	void					SoftBreaks (
								Theme * fTheme,
								float width);

	void					AddSoftBreak (SoftBreakEnd , float &,
		uint16 &, int16 &, float &, float &, Theme *);

	int16				 CountChars (int16 pos, int16 len);
	size_t				SetStamp (const char *, bool);

	void					SelectWord (int16 *, int16 *);
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
		fScroller (NULL),
		fTheme (theme),
		fWorking (NULL),
		fLine_count (0),
		fStamp_format (NULL),
		fClipping_name (NULL),
		fSp_start (0, 0),
		fSp_end (0, 0),
		fTracking (0),
		fTrack_offset (0, 0),
		fOff_view_runner (NULL),
		fOff_view_time (0),
		fResizedirty (false),
		fFontsdirty (false),
		fMyPopUp (NULL),
		fLastClick (0,0),
		fLastClickTime (0)
{
	memset (fLines, 0, sizeof (fLines));
	fURLCursor = new BCursor (URLCursorData);
	fTheme->ReadLock();

	BView::SetViewColor (B_TRANSPARENT_COLOR);
	BView::SetLowColor (fTheme->BackgroundAt (Theme::NormalBack));
	BView::SetHighColor (fTheme->ForegroundAt (Theme::NormalFore));

	fTheme->ReadUnlock();
}

RunView::~RunView (void)
{
	for (int16 i = 0; i < fLine_count; ++i)
		delete fLines[i];

	delete fWorking;
	delete fURLCursor;
	delete [] fStamp_format;
	delete [] fClipping_name;
}

void
RunView::AttachedToWindow (void)
{
	BView::AttachedToWindow();
#if B_BEOS_VERSION_DANO
	SetDoubleBuffering (B_UPDATE_INVALIDATED | B_UPDATE_SCROLLED | B_UPDATE_EXPOSED | B_UPDATE_RESIZED);
#endif
	RecalcScrollBar (false);
	fTheme->WriteLock();
	fTheme->AddView (this);
	fTheme->WriteUnlock();
}

void
RunView::DetachedFromWindow (void)
{
	fTheme->WriteLock();
	fTheme->RemoveView (this);
	fTheme->WriteUnlock();
}

void
RunView::FrameResized (float start_width, float height)
{
	BView::FrameResized (start_width, height);

	if (IsHidden())
	{
		fResizedirty = true;
		return;
	}
	ResizeRecalc();
}

void
RunView::TargetedByScrollView (BScrollView *s)
{
	fScroller = s;
	BView::TargetedByScrollView (fScroller);
}

void
RunView::Show (void)
{
	if (fFontsdirty)
	{
		FontChangeRecalc();
		// this effectively does the same thing as resize so if both have changed, only
		// do the fonts recalculation
		fFontsdirty = false;
		fResizedirty = false;
	}
	else if (fResizedirty)
	{
		ResizeRecalc();
		fResizedirty = false;
	}
	BView::Show();
}

void
RunView::Draw (BRect frame)
{
	Window()->DisableUpdates();
	Window()->BeginViewTransaction();

	rgb_color low_color, hi_color, view_color, sel_color, sel_fText;
	float height (frame.bottom);
	BRect bounds (Bounds());
	BRegion clipper;
	bool drawSelection (false);
	bool checkSelection (fSp_start != fSp_end);

	clipper.Set (frame);
	ConstrainClippingRegion (&clipper);

	fTheme->ReadLock();
	view_color = fTheme->BackgroundAt (Theme::NormalBack);

	sel_color = fTheme->BackgroundAt (Theme::SelectionBack);
	if (((sel_color.red + sel_color.blue + sel_color.green) / 3) >= 127)
	{
		sel_fText.red = sel_fText.green = sel_fText.blue = 0;
		sel_fText.alpha = 255;
	}
	else
	{
		sel_fText.red = sel_fText.green = sel_fText.blue = sel_fText.alpha = 255;
	}
	BRect remains;
	if (fLine_count == 0)
		remains = frame;
	else if (frame.bottom >= fLines[fLine_count - 1]->fBottom + 1.0)
		remains.Set (
			frame.left,
			fLines[fLine_count - 1]->fBottom + 1.0,
			frame.right,
			frame.bottom);

	if (remains.IsValid())
	{
		SetLowColor (view_color);
		FillRect (remains, B_SOLID_LOW);
	}

	for (int16 i = fLine_count - 1; i >= 0; --i)
	{
		Line *line (fLines[i]);
		if (line->fBottom < frame.top)
			break;

		BRect r (bounds.left, line->fTop, bounds.right, line->fBottom);

		if (!frame.Intersects (r))
			continue;

		float indent (ceil (MARGIN_WIDTH / 2.0));
		int16 place (0);

		int16 fore (0);
		int16 back (0);
		int16 font (0);

		height = line->fTop;

		for (int16 sit = 0; sit < line->fSoftie_used; ++sit)
		{
			int16 last_len (UTF8_CHAR_LEN (line->fText[line->fSofties[sit].fOffset]));
			float left (indent);
			float start (0.0);

			// Fill indentation
			SetLowColor (view_color);

			SetDrawingMode (B_OP_COPY);
			r.Set (0.0, height, indent - 1.0, height + line->fSofties[sit].fHeight - 1.0);
			FillRect (r, B_SOLID_LOW);

			if (sit)
			{
				int16 j (place);

				while (--j >= 0)
					if ((start = line->fEdges[j]) != 0)
						break;
			}

			while (place < line->fSofties[sit].fOffset + last_len)
			{
				// Get current foreground color and set
				while (fore < line->fFc_count)
				{
					if (line->fFcs[fore].fWhich == FORE_WHICH)
					{
						if (line->fFcs[fore].fOffset > place)
							break;

						hi_color = fTheme->ForegroundAt (line->fFcs[fore].fIndex);
					}

					++fore;
				}

				// Get current background color and set
				while (back < line->fFc_count)
				{
					if (line->fFcs[back].fWhich == BACK_WHICH)
					{
						if (line->fFcs[back].fOffset > place)
							break;


						low_color = fTheme->BackgroundAt (line->fFcs[back].fIndex);
					}

					++back;
				}

				// Get current font and set
				while (font < line->fFc_count)
				{
					if (line->fFcs[font].fWhich == FONT_WHICH)
					{
						if (line->fFcs[font].fOffset > place)
							break;

						const BFont &f (fTheme->FontAt (line->fFcs[font].fIndex));

						SetFont (&f);
					}

					++font;
				}

				int16 fLength (line->fSofties[sit].fOffset - place + last_len);

				if (fore < line->fFc_count
				&&	line->fFcs[fore].fOffset - place < fLength)
					fLength = line->fFcs[fore].fOffset - place;

				if (back < line->fFc_count
				&&	line->fFcs[back].fOffset - place < fLength)
					fLength = line->fFcs[back].fOffset - place;

				if (font < line->fFc_count
				&&	line->fFcs[font].fOffset - place < fLength)
					fLength = line->fFcs[font].fOffset - place;

				if (checkSelection)
				{
					// case 1: current line marks beginning of selection
					if (i == fSp_start.fLine)
					{
						// if we're just prior to the selection, clip fLength to only
						// draw up to the selection start
						if (place + fLength >= fSp_start.fOffset && place < fSp_start.fOffset)
						{
							fLength = fSp_start.fOffset - place;
							drawSelection = false;
						}
						// we're at the selection, switch drawing color mode
						else if (place >= fSp_start.fOffset)
						{
							if (fSp_end.fLine == fSp_start.fLine)
							{
								if (place < fSp_end.fOffset)
								{
									drawSelection = true;
									if ((fSp_end.fOffset - place) < fLength)
										fLength = fSp_end.fOffset - place;
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
					else if (i > fSp_start.fLine && i < fSp_end.fLine)
						drawSelection = true;
					// case 3: last line of selection, with multiple fLines in between
					else if (i == fSp_end.fLine && i != fSp_start.fLine)
					{
						if (place < (fSp_end.fOffset))
						{
							if (fSp_end.fOffset - place < fLength)
								fLength = (fSp_end.fOffset - place);
							drawSelection = true;
						}
						else
							drawSelection = false;
					}
					else
						drawSelection = false;
				}

				if (place + fLength == line->fLength)
					--fLength;

				int16 k (place + fLength - 1);
				while (line->fEdges[k] == 0)
					--k;

				r.Set (
					left,
					height,
					line->fEdges[k] + indent - start,
					height + line->fSofties[sit].fHeight - 1.0);

				SetDrawingMode (B_OP_COPY);
				if (drawSelection)
					SetLowColor (sel_color);
				else
					SetLowColor (low_color);
				SetHighColor (hi_color);
				FillRect (r, B_SOLID_LOW);

				if (drawSelection)
					SetHighColor (sel_fText);

				SetDrawingMode (B_OP_OVER);

				DrawString (
					line->fText + place,
					min_c (fLength, line->fLength - place - 1),
					BPoint (left, height + line->fSofties[sit].fAscent));

				left = line->fEdges[k] + indent - start;

				if ((place += fLength) + 1 >= line->fLength)
					++place;
			}


			// Margin after fText
			SetDrawingMode (B_OP_COPY);
			SetLowColor (view_color);
			FillRect (
				BRect (
					left + 1.0,
					height,
					bounds.right,
					height + line->fSofties[sit].fHeight - 1.0),
				B_SOLID_LOW);

			height += line->fSofties[sit].fHeight;

			if (sit == 0)
				indent += (MARGIN_INDENT / 2.0);
		}
	}

	fTheme->ReadUnlock();
	Window()->EndViewTransaction();
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
	// This function checks certain criteria (fText is selected,
	// TextView is editable, etc) to determine fWhich MenuItems
	// to enable and disable

	bool enablecopy (true),
			 enableselectall (true),
			 enablelookup (false);
	BString querystring ("");

	if (fSp_start == fSp_end)
		enablecopy = false; // no selection

	if (!fLine_count)
		enableselectall = false;

	if (enablecopy)
	{
		enablelookup = true; // has a selection less than 32 chars long
		GetSelectionText(querystring);
	}

	fMyPopUp = new BPopUpMenu ("IRCView Context Menu", false, false);

	BMenuItem *item;

	BMessage *lookup;
	lookup = new BMessage (M_LOOKUP_BROWSER);
	lookup->AddString ("string", querystring);
	item = new BMenuItem("Open in browser", lookup);
	item->SetEnabled (enablelookup);
	item->SetTarget (Parent());
	fMyPopUp->AddItem (item);

	lookup = new BMessage (M_LOOKUP_WEBSTER);
	lookup->AddString ("string", querystring);
	item = new BMenuItem("Lookup (Dictionary)", lookup);
	item->SetEnabled (enablelookup);
	item->SetTarget (Parent());
	fMyPopUp->AddItem (item);

	lookup = new BMessage (M_LOOKUP_GOOGLE);
	lookup->AddString ("string", querystring);
	item = new BMenuItem("Lookup (Google)", lookup);
	item->SetEnabled (enablelookup);
	item->SetTarget (Parent());
	fMyPopUp->AddItem (item);

	lookup = new BMessage (M_LOOKUP_ACRONYM);
	lookup->AddString ("string", querystring);
	item = new BMenuItem("Lookup (Acronym Finder)", lookup);
	item->SetEnabled (enablelookup);
	item->SetTarget (Parent());
	fMyPopUp->AddItem (item);

	fMyPopUp->AddSeparatorItem();

	item = new BMenuItem("Copy", new BMessage (B_COPY), 'C');
	item->SetEnabled (enablecopy);
	item->SetTarget (this);
	fMyPopUp->AddItem (item);

	item = new BMenuItem("Select All", new BMessage (B_SELECT_ALL), 'A');
	item->SetEnabled (enableselectall);
	item->SetTarget (this);
	fMyPopUp->AddItem (item);

	fMyPopUp->SetFont (be_plain_font);
}

bool
RunView::CheckClickBounds (const SelectPos &s, const BPoint &point) const
{
	return ((point.x <= fLines[s.fLine]->fEdges[fLines[s.fLine]->fLength - 1])
			&& (point.y <= fLines[s.fLine]->fBottom));
}

void
RunView::MouseDown (BPoint point)
{
	if (!fLine_count)
		return;

	BMessage *msg (Window()->CurrentMessage());
	uint32 buttons;
	uint32 mouseModifiers;
	bigtime_t sysTime;
	msg->FindInt64 ("when", &sysTime);
	uint16 clicks = CheckClickCount (point, fLastClick, sysTime, fLastClickTime, fClickCount) % 3;
	msg->FindInt32 ("buttons", reinterpret_cast<int32 *>(&buttons));
	msg->FindInt32 ("modifiers", reinterpret_cast<int32 *>(&mouseModifiers));

	SelectPos s (PositionAt (point));
	bool inBounds (CheckClickBounds (s, point));

	if (buttons == B_SECONDARY_MOUSE_BUTTON
	&&		(mouseModifiers & B_SHIFT_KEY) == 0
	&&		(mouseModifiers & B_COMMAND_KEY) == 0
	&&		(mouseModifiers & B_CONTROL_KEY) == 0
	&&		(mouseModifiers & B_OPTION_KEY) == 0
	&&			(mouseModifiers & B_MENU_KEY) == 0)
	{
		SelectPos start (s),
					end (s);

		// select word
		if (inBounds && !IntersectSelection (s,s))
		{
			fLines[s.fLine]->SelectWord (&start.fOffset, &end.fOffset);

			Select (start, end);
		}

			BuildPopUp();
			fMyPopUp->Go (
				ConvertToScreen (point),
				true,
				false);

			delete fMyPopUp;
			fMyPopUp = 0;
			return;
	}

	if (buttons == B_PRIMARY_MOUSE_BUTTON
	&&			(mouseModifiers & B_SHIFT_KEY)	 == 0
	&&			(mouseModifiers & B_COMMAND_KEY) == 0
	&&			(mouseModifiers & B_CONTROL_KEY) == 0
	&&			(mouseModifiers & B_OPTION_KEY)	== 0
	&&			(mouseModifiers & B_MENU_KEY)		== 0)
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
					fLines[s.fLine]->SelectWord (&start.fOffset, &end.fOffset);

					Select (start, end);
					return;
				}
			}
			break;

			case 0:
			{
				if (inBounds)
				{
					start.fOffset = 0;
					end.fOffset = fLines[s.fLine]->fLength - 1;
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
				fTracking = 1;
				fTrack_offset = s;
				return;
			}
		}
	}
	else if (buttons					== B_PRIMARY_MOUSE_BUTTON
	&&			(mouseModifiers & B_SHIFT_KEY)	 != 0
	&&			(mouseModifiers & B_COMMAND_KEY) == 0
	&&			(mouseModifiers & B_CONTROL_KEY) == 0
	&&			(mouseModifiers & B_OPTION_KEY)	== 0
	&&			(mouseModifiers & B_MENU_KEY)		== 0)
	{
		if (s.fLine < fSp_start.fLine || s.fOffset < fSp_start.fOffset)
		{
			Select (s, fSp_end);
			fTrack_offset = SelectPos (fSp_end.fLine, (fSp_end.fOffset > 0) ? fSp_end.fOffset - 1 : fSp_end.fOffset);
		}
		else
		{
			Select (fSp_start, s);
			fTrack_offset = fSp_start;
		}

		SetMouseEventMask (B_POINTER_EVENTS);
		fTracking = 2;
	}
}

void
RunView::CheckURLCursor (BPoint point)
{
	if (!fLine_count)
		return;

	SelectPos s = PositionAt (point);

	if (!fLines[s.fLine]->fUrls)
	{
		// if there aren't any URLs in the current line, go back to default
		SetViewCursor (B_CURSOR_SYSTEM_DEFAULT);
		return;
	}
	Line *curline (fLines[s.fLine]);

	for (int32 i = 0; i < curline->fUrls->CountItems(); i++)
	{
		URL *current = curline->fUrls->ItemAt(i);
		if ((s.fOffset >= current->fOffset)
		 && (s.fOffset <= current->fOffset + current->fLength))
		 {
			 SetViewCursor (fURLCursor);
			 return;
		 }
	}

	// no URLs found, set back to default
	SetViewCursor (B_CURSOR_SYSTEM_DEFAULT);
}

void
RunView::MouseMoved (BPoint point, uint32 transit, const BMessage *msg)
{
	if (fTracking == 0
	&&	fLine_count
	&& (transit == B_ENTERED_VIEW
	||	transit == B_INSIDE_VIEW))
		CheckURLCursor (point);


	if (!fLine_count || fTracking == 0)
	{
		BView::MouseMoved (point, transit, msg);
		return;
	}

	switch (transit)
	{
		case B_ENTERED_VIEW:
			if (fOff_view_runner)
			{
				delete fOff_view_runner;
				fOff_view_runner = 0;
			}

			if (fTracking == 1 || fTracking == 2)
				ExtendTrackingSelect (point);

			break;

		case B_EXITED_VIEW:
			if (fTracking == 1 || fTracking == 2)
				ShiftTrackingSelect (point, true, OFFVIEW_TIMER);
			break;

		case B_OUTSIDE_VIEW:

			if (fTracking == 1 || fTracking == 2)
			{
				bigtime_t now (system_time());

				ShiftTrackingSelect (
					point,
					false,
					max_c ((int32)0L, min_c (OFFVIEW_TIMER, OFFVIEW_TIMER - (now - fOff_view_time))));
			}
			break;

		case B_INSIDE_VIEW:

			if ((fTracking == 1) && (fSp_start != fSp_end))
			{
				BMessage message (B_MIME_DATA);
				BString text;

				GetSelectionText (text);
				message.AddData (
					"text/plain",
					B_MIME_TYPE,
					text.String(),
					text.Length());

				BString clip_name (" Clipping");

				if (fClipping_name)
					clip_name.Prepend (fClipping_name);
				else
					clip_name.Prepend ("RunView");

				message.AddString ("be:clip_name", clip_name.String());
				message.AddInt32 ("be:actions", B_COPY_TARGET);

				BRect frame (
					fLines[fSp_start.fLine]->fEdges[fSp_start.fOffset],
					fLines[fSp_start.fLine]->fTop,
					fLines[fSp_end.fLine]->fEdges[fSp_end.fOffset],
					fLines[fSp_end.fLine]->fBottom);

				if (fSp_start.fLine != fSp_end.fLine)
				{
					frame.left = 0.0;
					frame.right = Bounds().right;
				}
				// selection lies within the bounds of a line, check
				// if it fLines on one of the wrapped subfLines and calculate rectangle
				// appropriately
				else
				{
					Line *line (fLines[fSp_start.fLine]);
					float left (line->fEdges[fSp_start.fOffset]),
						top (line->fTop),
						right (line->fEdges[fSp_end.fOffset]),
						bottom (line->fBottom);
					int top_softie (0), bottom_softie (0);
					bool start_found (false);
					bool end_found (false);

					if (line->fSoftie_used)
					{
						if (fSp_start.fOffset < line->fSofties[0].fOffset)
							start_found = true;

						if (fSp_end.fOffset < line->fSofties[0].fOffset)
							end_found = true;
					}

					if (!end_found)
					for (int16 sit = 1; sit < line->fSoftie_used; ++sit)
					{
						if (!start_found && fSp_start.fOffset < line->fSofties[sit].fOffset)
						{
							left = line->fEdges[fSp_start.fOffset] -
								line->fEdges[line->fSofties[sit-1].fOffset];

							top += (sit) * line->fSofties[sit].fHeight;
							top_softie = sit;
							start_found = true;
						}

						if (fSp_end.fOffset < line->fSofties[sit].fOffset)
						{
							right = line->fEdges[fSp_end.fOffset] -
								line->fEdges[line->fSofties[sit-1].fOffset];

							bottom = top + (sit - top_softie + 1) * line->fSofties[sit].fHeight;
							bottom_softie = sit;
							end_found = true;
							break;
						}
					}
					if (!end_found)
					{
						int32 soft_count = (line->fSoftie_used >= 2) ?
							line->fSoftie_used - 2 : 0;
						right = line->fEdges[line->fLength - 1] -
							line->fEdges[line->fSofties[soft_count].fOffset];
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

				DragMessage (&message, frame);

				fTracking = 3;
			}
			else if (fTracking == 1 || fTracking == 2)
				ExtendTrackingSelect (point);
			break;
	}
}

void
RunView::MouseUp (BPoint point)
{
	SelectPos s (PositionAt (point));
	bool url_handle (false);

	if (!fLine_count)
	{
		fTracking = 0;
		return;
	}

	if (fTracking == 1)
	{
		Line *curline (fLines[s.fLine]);
		if (curline->fUrls)
		{
			for (int32 i = 0; i < curline->fUrls->CountItems(); i++)
			{
				URL *current = curline->fUrls->ItemAt(i);
				if ((s.fOffset >= current->fOffset)
				 && (s.fOffset <= current->fOffset + current->fLength))
				 {
					 vision_app->LoadURL (current->fUrl.String());
					 url_handle = true;
					 break;
				 }
			}
		}
		if (!url_handle && s == fTrack_offset)
			Select (s, s);
	}

	if (fOff_view_runner)
	{
		delete fOff_view_runner;
		fOff_view_runner = 0;
	}

	fTracking = 0;

}

void
RunView::ExtendTrackingSelect (BPoint point)
{
	SelectPos s (PositionAt (point));

	if (s.fLine < fTrack_offset.fLine || (s.fLine == fTrack_offset.fLine && s.fOffset < fTrack_offset.fOffset))
	{
		Select (s, fTrack_offset);
		fTracking = 2;
	}
	else if (s.fLine > fTrack_offset.fLine || (s.fLine == fTrack_offset.fLine && s.fOffset > fTrack_offset.fOffset))
	{
		Select (fTrack_offset, s);
		fTracking = 2;
	}
}

void
RunView::ShiftTrackingSelect (BPoint point, bool move, bigtime_t timer)
{
	BRect bounds (Bounds());

	if (fOff_view_runner)
	{
		delete fOff_view_runner;
		fOff_view_runner = 0;
	}

	if (point.y < bounds.top)
	{
		if (bounds.top > 0.0)
		{
			float delta (bounds.top - point.y);

			if (fOff_view_runner == 0)
			{
				BMessage *msg (new BMessage (M_OFFVIEW_SELECTION));

				msg->AddFloat ("delta", delta);
				msg->AddBool	("bottom", false);
				msg->AddPoint ("point", point);

				fOff_view_runner = new BMessageRunner (
					BMessenger (this),
					msg,
					timer == (int32)0L ? OFFVIEW_TIMER : timer);
			}

			if (move || timer == 0)
			{
				delta = max_c (ABS (ceil (delta / 2.0)), 10.0);
				delta = min_c (delta, Bounds().Height());

				if (bounds.top - delta < 0.0)
					delta = bounds.top;

				ScrollBy (0.0, -delta);
				fOff_view_time = system_time();
			}
		}

		point.x = 0.0;
		point.y = Bounds().top;
		ExtendTrackingSelect (point);
	}

	if (point.y > bounds.bottom)
	{
		Line *line (fLines[fLine_count-1]);
		if (line
		&&	line->fBottom > bounds.bottom)
		{
			float delta (point.y - bounds.bottom);

			if (fOff_view_runner == 0)
			{
				BMessage *msg (new BMessage (M_OFFVIEW_SELECTION));

				msg->AddFloat ("delta", delta);
				msg->AddBool	("bottom", true);
				msg->AddPoint ("point", point);

				fOff_view_runner = new BMessageRunner (
					BMessenger (this),
					msg,
					timer == (int32)0L ? OFFVIEW_TIMER : timer);
			}

			if (move || timer == 0)
			{
				delta = max_c (ABS (ceil (delta / 2.0)), 10.0);
				delta = min_c (delta, Bounds().Height());

				if (bounds.bottom + delta > line->fBottom)
					delta = line->fBottom - bounds.bottom;

				ScrollBy (0.0, delta);
				fOff_view_time = system_time();
			}
		}

		point.x = Bounds().right;
		point.y = Bounds().bottom;
		ExtendTrackingSelect (point);
	}
	else
		ExtendTrackingSelect (point);
}

void
RunView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_THEME_FOREGROUND_CHANGE:
		case M_THEME_BACKGROUND_CHANGE:
				if (!IsHidden())
					Invalidate (Bounds());
			break;

		case M_THEME_FONT_CHANGE:
		{
			Theme *save (fTheme);

			fTheme = NULL;
			SetTheme (save);
			break;
		}

		case B_SELECT_ALL:
			SelectAll();
			break;

		case B_COPY:
			if (fSp_start != fSp_end
			&&	be_clipboard->Lock())
			{
				BString fText;
				GetSelectionText (fText);

				be_clipboard->Clear();

				BMessage *message (be_clipboard->Data());
				message->AddData ("text/plain", B_MIME_TYPE, fText.String(), fText.Length());

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
	int16 fSoftie_size (0), fSoftie_used (0);
	SoftBreak *fSofties (NULL);
	BRect bounds (Bounds());
	BRegion region;
	float top (0.0);

	fTheme->ReadLock();
	for (int16 i = 0; i < fLine_count; ++i)
	{
		float old_top (fLines[i]->fTop), old_bottom (fLines[i]->fBottom);
		if (fSoftie_size < fLines[i]->fSoftie_used)
		{
			delete [] fSofties;
			fSofties = new SoftBreak [fSoftie_size = fLines[i]->fSoftie_size];
		}

		fSoftie_used = fLines[i]->fSoftie_used;
		memcpy (fSofties, fLines[i]->fSofties, (fSoftie_used * sizeof (SoftBreak)));

		fLines[i]->fTop = top;
		fLines[i]->SoftBreaks (fTheme, width);
		top = fLines[i]->fBottom + 1.0;

		BRect r (0.0, fLines[i]->fTop, bounds.right, fLines[i]->fBottom);

		if (bounds.Intersects (r)
		&& (old_top != fLines[i]->fTop
		||	old_bottom != fLines[i]->fBottom
		||	fSoftie_used != fLines[i]->fSoftie_used
		||	memcmp (fSofties, fLines[i]->fSofties, fSoftie_used * sizeof (SoftBreak))))
			region.Include (r);
	}

	fTheme->ReadUnlock();

	if (Window())
	{
		if (RecalcScrollBar (true))
			Invalidate (Bounds());
		else
		{
			int32 count (region.CountRects()), j;

			for (j = 0; j < count; ++j)
				Invalidate (region.RectAt (j));

			if (top <= bounds.bottom)
			{
				BRect r (bounds);

				r.top = top;
				Invalidate (r);
			}
		}

		Window()->Sync();
	}

	if (fWorking) fWorking->fTop = top;
	delete [] fSofties;
}

void
RunView::FontChangeRecalc (void)
{
	float width (Bounds().Width() - MARGIN_WIDTH);
	float top (0.0);

	for (int16 i = 0; i < fLine_count; ++i)
	{
		fLines[i]->fTop = top;

		fLines[i]->FigureSpaces();
		fLines[i]->FigureEdges (fTheme, width);

		top = fLines[i]->fBottom + 1.0;
	}

	if (fWorking)
		fWorking->fTop = top;

	RecalcScrollBar (false);
	if (!IsHidden())
		Invalidate (Bounds());
	if (Window()) Window()->UpdateIfNeeded();
}

bool
RunView::RecalcScrollBar (bool constrain)
{
	BScrollBar *bar;

	if (fScroller == NULL
	|| (bar = fScroller->ScrollBar (B_VERTICAL)) == NULL)
		return false;

	float value (bar->Value());
	BRect bounds (Bounds());
	bool changed (false);
	float bottom (0.0);
	float scrollMin, scrollMax;

	bar->GetRange (&scrollMin, &scrollMax);

	if (fLine_count
	&& (bounds.Contains (BPoint (0.0, 0.0)) == false
	||	bounds.Contains (BPoint (0.0, fLines[fLine_count - 1]->fBottom)) == false))
	{
		bottom = fLines[fLine_count - 1]->fBottom + 5.0;
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

	if (scrollMax != bottom)
	{
		bar->SetRange (0.0, bottom);

		if (value == scrollMax || scrollMin == scrollMax)
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
		if (buffer == NULL)
			return;
	float width (Bounds().Width() - 10);
	int32 place (0);

	assert (fore != Theme::TimestampFore);
	assert (back != Theme::TimestampBack);
	assert (font != Theme::TimestampFont);
	assert (fore != Theme::TimespaceFore);
	assert (back != Theme::TimespaceBack);
	assert (font != Theme::TimespaceFont);
	assert (back != Theme::SelectionBack);

	fTheme->ReadLock();


	while (place < len)
	{
		int32 end (place);

		while (end < len && buffer[end] != '\n')
			++end;

		if (end < len) ++end;

		if (fWorking)
		{
			URLCrunch crunch (buffer + place, end - place);
			BString temp;
			int32 url_offset (0),
				last_offset (0);


			while ((url_offset = crunch.Crunch (&temp)) != B_ERROR)
			{
				fWorking->Append	(buffer + place,
									(url_offset - last_offset),
									width,
									fTheme,
									fore,
									back,
									font);

				fWorking->Append	(temp.String(),
									temp.Length(),
									width,
									fTheme,
									C_URL,
									back,
									F_URL);

				place += (url_offset - last_offset) + temp.Length();
				last_offset = url_offset + temp.Length();
			}

			if (place < end)
				fWorking->Append (
				buffer + place,
				end - place,
				width,
				fTheme,
				fore,
				back,
				font);
		}
		else
		{
			float top (0.0);

			if (fLine_count > 0)
				top = fLines[fLine_count - 1]->fBottom + 1.0;


			fWorking = new Line (
				buffer + place,
				0,
				top,
				width,
				fTheme,
				fStamp_format,
				fore,
				back,
				font);

			URLCrunch crunch (buffer + place, end - place);
			BString temp;
			int32 url_offset (0),
				last_offset (0);

			while ((url_offset = crunch.Crunch (&temp)) != B_ERROR)
			{
				fWorking->Append	(buffer + place,
									(url_offset - last_offset),
									width,
									fTheme,
									fore,
									back,
									font);

				fWorking->Append	(temp.String(),
									temp.Length(),
									width,
									fTheme,
									C_URL,
									back,
									F_URL);

				place += (url_offset - last_offset) + temp.Length();
				last_offset = url_offset + temp.Length();
			}

			if (place < end)
				fWorking->Append (buffer + place,
									end - place,
									width,
									fTheme,
									fore,
									back,
									font);
		}

		if (fWorking->fLength
		&&	fWorking->fText[fWorking->fLength - 1] == '\n')
		{
			bool chopped;

			if (Window()) Window()->DisableUpdates();

			if ((chopped = (fLine_count == LINE_COUNT)))
			{
				Line *first (fLines[0]);
				float shift (first->fBottom + 1);

				for (int16 i = 1; i < LINE_COUNT; ++i)
				{
					fLines[i]->fTop		-= shift;
					fLines[i]->fBottom -= shift;

					fLines[i - 1] = fLines[i];
				}

				fWorking->fTop		-= shift;
				fWorking->fBottom -= shift;

				delete first;

				if (fSp_start.fLine > 0)
					fSp_start.fLine--;
				else
					fSp_start.fOffset = 0;

				if (fSp_end.fLine > 0)
					fSp_end.fLine--;
				else
					fSp_end.fOffset = 0;

				// Recalc the scrollbar so that we have clean drawing
				// after the line has been removed
				--fLine_count;
				RecalcScrollBar (true);
			}

			fLines[fLine_count++] = fWorking;
			RecalcScrollBar (true);

			Invalidate (Bounds());

			if (Window())
			{
				Window()->EnableUpdates();
				Window()->UpdateIfNeeded();
			}

			fWorking = NULL;
		}

		place = end;
	}

	fTheme->ReadUnlock();
}

void
RunView::Clear (void)
{
	for (int16 i = 0; i < fLine_count; ++i)
		delete fLines[i];

	fLine_count = 0;
	RecalcScrollBar (true);
	Invalidate();

	fSp_start.fLine = 0;
	fSp_start.fOffset = 0;
	fSp_end = fSp_start;

	if (fWorking)
		fWorking->fTop = 0.0;
}

int16
RunView::LineCount (void) const
{
	return fLine_count;
}

const char *
RunView::LineAt (int16 which) const
{
	if (which < 0 || which >= fLine_count)
		return NULL;

	return fLines[which]->fText;
}

void
RunView::SetTimeStampFormat (const char *format)
{
	if ((format == NULL
	&&	 fStamp_format == NULL)
	||	(format != NULL
	&&	 fStamp_format != NULL
	&&	 strcmp (format, fStamp_format) == 0))
		return;

	bool was_on (false);

	if (fStamp_format)
	{
		delete [] fStamp_format;
		fStamp_format = NULL;
		was_on = true;
	}

	if (format)
		fStamp_format = strcpy (new char [strlen (format) + 1], format);

	float width (Bounds().Width() - MARGIN_WIDTH);
	float top (0.0);

	fTheme->ReadLock();
	for (int16 i = 0; i < fLine_count; ++i)
	{
		fLines[i]->fTop = top;

		fLines[i]->SetStamp (fStamp_format, was_on);
		fLines[i]->FigureSpaces();
		fLines[i]->FigureEdges(fTheme, width);

		top = fLines[i]->fBottom + 1.0;
	}
	fTheme->ReadUnlock();

	if (fWorking)
	{
		fWorking->fTop = top;
		fWorking->SetStamp (fStamp_format, was_on);
	}

	RecalcScrollBar (false);
	Invalidate (Bounds());
	if (Window()) Window()->UpdateIfNeeded();
}

void
RunView::SetTheme (Theme *t)
{
	if (t == NULL || fTheme == t)
		return;

	fTheme = t;

	if (IsHidden())
	{
		fFontsdirty = true;
		return;
	}
	FontChangeRecalc();
}

SelectPos
RunView::PositionAt (BPoint point) const
{
	int16 i, lfIndex (0);
	SelectPos pos (-1, 0);

	if (fLine_count == 0)
		return pos;

	// find the line
	for (i = 0; i < fLine_count; ++i)
	{
		if (fLines[i]->fTop > point.y)
			break;
		lfIndex = i;
	}

	// check to make sure we actually did find a line and not just run into fLine_count
	if (fLines[lfIndex]->fBottom < point.y)
	{
		pos.fLine = fLine_count - 1;
		pos.fOffset = fLines[fLine_count - 1]->fLength;
		return pos;
	}

	float height (fLines[lfIndex]->fTop);
	int16 sfIndex (0);

	for (i = 0; i < fLines[lfIndex]->fSoftie_used; ++i)
	{
		if (height > point.y)
			break;

		sfIndex = i;
		height += fLines[lfIndex]->fSofties[i].fHeight;
	}

	float margin (MARGIN_WIDTH / 2.0);
	float width (0);
	int16 start (0);

	if (sfIndex)
	{
		int16 offset (fLines[lfIndex]->fSofties[sfIndex - 1].fOffset);

		width = fLines[lfIndex]->fEdges[offset];
		start = offset + UTF8_CHAR_LEN (fLines[lfIndex]->fText[offset]);
	}

	for (i = start; i <= fLines[lfIndex]->fSofties[sfIndex].fOffset; ++i)
		if (fLines[lfIndex]->fEdges[i] + margin - width >= point.x)
			break;

	pos.fLine = lfIndex;
	pos.fOffset = min_c (i, fLines[lfIndex]->fSofties[sfIndex].fOffset);
	if (pos.fOffset > 0) pos.fOffset += UTF8_CHAR_LEN (fLines[pos.fLine]->fText[pos.fOffset]);

	return pos;
}

BPoint
RunView::PointAt (SelectPos s) const
{
	return BPoint(fLines[s.fLine]->fTop + fLines[s.fLine]->fBottom / 2.0, fLines[s.fLine]->fEdges[s.fOffset]);
}

void
RunView::GetSelectionText (BString &string) const
{
	if (fSp_start == fSp_end)
		return;

	if (fSp_start.fLine == fSp_end.fLine)
	{
		const char *line (LineAt (fSp_start.fLine));
		string.Append (line + fSp_start.fOffset, fSp_end.fOffset - fSp_start.fOffset);
		return;
	}

	for (int32 i = fSp_start.fLine; i <= fSp_end.fLine; i++)
	{
		const char *line (LineAt (i));
		if (i == fSp_start.fLine)
		{
			line += fSp_start.fOffset;
			string.Append (line);
		}
		else if (i == fSp_end.fLine)
		{
			string.Append (line, fSp_end.fOffset);
			break;
		}
		else
			string.Append (line);
	}
}

bool
RunView::IntersectSelection (const SelectPos &start, const SelectPos &end) const
{
	if (fSp_start.fLine == fSp_end.fLine)
	{
		if (start.fLine == fSp_start.fLine && start.fOffset >= fSp_start.fOffset && start.fOffset < fSp_end.fOffset)
			return true;
		if (end.fLine == fSp_start.fLine && end.fOffset >= fSp_start.fOffset && end.fOffset < fSp_end.fOffset)
			return true;
	}
	else
	{
		if (start.fLine > fSp_start.fLine && start.fLine < fSp_end.fLine)
			return true;
		if (end.fLine > fSp_start.fLine && end.fLine < fSp_end.fLine)
			return true;
		if (start.fLine == fSp_start.fLine && start.fOffset >= fSp_start.fOffset)
			return true;
		if (end.fLine == fSp_start.fLine && end.fOffset >= fSp_start.fOffset)
			return true;
		if (start.fLine == fSp_end.fLine && start.fOffset < fSp_end.fOffset)
			return true;
		if (end.fLine == fSp_end.fLine && end.fOffset < fSp_end.fOffset)
			return true;
	}

	return false;
}

BRect
RunView::GetTextFrame(const SelectPos &start, const SelectPos &end) const
{
	return BRect (0.0, fLines[(start.fLine > 0) ? (start.fLine - 1) : 0]->fTop,
		Bounds().Width(), fLines[end.fLine]->fBottom);
}

void
RunView::Select (const SelectPos &start, const SelectPos &end)
{
	if (fSp_start != fSp_end)
	{
		if (start == end || !IntersectSelection (start, end))
		{
			BRect frame (GetTextFrame (fSp_start, fSp_end));

			fSp_start = start;
			fSp_end	 = start;
			Invalidate (frame);
		}
		else
		{
			if (fSp_start.fLine < start.fLine || (fSp_start.fLine == start.fLine && fSp_start.fOffset < start.fOffset))
			{
				BRect frame (GetTextFrame (fSp_start, start));

				fSp_start = start;
				Invalidate (frame);
			}

			if (end.fLine < fSp_end.fLine || (fSp_end.fLine == end.fLine && end.fOffset < fSp_end.fOffset))
			{
				BRect frame (GetTextFrame (end, fSp_end));

				fSp_end = end;
				Invalidate (frame);
			}
		}
	}

	if (fSp_start == fSp_end)
	{
		fSp_start = start;
		fSp_end	 = end;

		if (fSp_start != fSp_end)
		{
			BRect frame (GetTextFrame (start, end));

			Invalidate (frame);
		}
	}
	else // extension
	{
		if (start.fLine < fSp_start.fLine || (start.fLine == fSp_start.fLine && start.fOffset < fSp_start.fOffset))
		{
			BRect frame (GetTextFrame (start, fSp_start));

			fSp_start = start;
			Invalidate (frame);
		}

		if (end.fLine > fSp_end.fLine || (end.fLine == fSp_end.fLine && end.fOffset > fSp_end.fOffset))
		{
			BRect frame (GetTextFrame (fSp_end, end));

			fSp_end = end;
			Invalidate (frame);
		}
	}
}

void
RunView::SelectAll (void)
{
	if (fLine_count)
	{
		fSp_start = SelectPos (0, 0);
		fSp_end = SelectPos (fLine_count-1, fLines[fLine_count-1]->fLength);
		Invalidate(Bounds());
	}
}

void
RunView::SetClippingName (const char *name)
{
	delete [] fClipping_name;
	fClipping_name = new char[strlen(name) + 1];
	memcpy (fClipping_name, name, strlen(name));
	fClipping_name[strlen(name)] = '\0';
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
	:	fText (NULL),
		fStamp (time(NULL)),
		fUrls (NULL),
		fSpaces (NULL),
		fEdges (NULL),
		fFcs (NULL),
		fSofties (NULL),
		fTop (top),
		fBottom (0.0),
		fLength (len),
		fSpace_count (0),
		fEdge_count (0),
		fFc_count (0),
		fSoftie_size (0),
		fSoftie_used (0)
{
	// Very important to call SetStamp before Append, It would look real funny otherwise!
	SetStamp( stamp_format, false );

	Append( buffer, len, width, theme, fore, back, font );
}

Line::~Line (void)
{
	delete [] fSpaces;
	delete [] fEdges;
	delete [] fFcs;
	delete [] fText;
	delete [] fSofties;

	if (fUrls)
	{
		while (fUrls->CountItems() > 0)
			delete fUrls->RemoveItemAt((int32)0);
		delete fUrls;
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
	int16 save (fLength);
	char *new_fText;

	new_fText = new char [fLength + len + 1];

	if (fText != NULL)
	{
		memcpy (new_fText, fText, fLength);
		delete [] fText;
	}

	memcpy (new_fText + fLength, buffer, len);
	fLength += len;
	new_fText[fLength] = '\0';

	// replace Tab chars with spaces.
	// todo: This should be temp until RunView can properly
	//				display tabs.
	for( char* pos = new_fText + save; *pos; ++pos )
	{
		if( '\t' == *pos )
		{
			*pos = ' ';
		}
	}

	fText = new_fText;

	FigureFontColors (save, fore, back, font);

	if (fore == C_URL)
	{
		if (!fUrls)
			fUrls = new urllist;
		fUrls->AddItem (new URL (buffer, save, len));
	}

	if (fText[fLength - 1] == '\n')
	{
		FigureSpaces();
		FigureEdges (theme, width);
	}
}

void
Line::FigureSpaces (void)
{
	const char spacers[] = " \t\n-\\/";
	const char *buffer (fText);
	size_t offset (0), n;
	int16 count (0);

	delete [] fSpaces;
	fSpace_count = 0;
	while ((n = strcspn (buffer + offset, spacers)) < fLength - offset)
	{
		++count;
		offset += n + 1;
	}

	fSpaces = new int16 [count];

	offset = 0;
	while ((n = strcspn (buffer + offset, spacers)) < fLength - offset)
	{
		fSpaces[fSpace_count++] = n + offset;
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
	if (fFc_count)
	{
		int16 last_fore = -1;
		int16 last_back = -1;
		int16 last_font = -1;
		int16 i;

		// we have fFcs, so we backtrack for last of each fWhich
		for (i = fFc_count - 1; i >= 0; --i)
		{
			if (last_fore < 0
			&&	fFcs[i].fWhich == FORE_WHICH)
				last_fore = i;
			else if (last_back < 0
			&&			 fFcs[i].fWhich == BACK_WHICH)
				last_back = i;
			else if (last_font < 0
			&&			 fFcs[i].fWhich == FONT_WHICH)
				last_font = i;

			if (last_fore >= 0
			&&	last_back >= 0
			&&	last_font >= 0)
				break;
		}

		// now figure out how many more we need
		int16 count = 0;
		if (fFcs[last_fore].fIndex != fore)
			++count;
		if (fFcs[last_back].fIndex != back)
			++count;
		if (fFcs[last_font].fIndex != font)
			++count;

		if (count)
		{
			FontColor *new_fFcs;
			new_fFcs = new FontColor [fFc_count + count];
			memcpy (new_fFcs, fFcs, fFc_count * sizeof (FontColor));
			delete [] fFcs;
			fFcs = new_fFcs;

			if (fFcs[last_fore].fIndex != fore)
			{
				fFcs[fFc_count].fWhich = FORE_WHICH;
				fFcs[fFc_count].fOffset = pos;
				fFcs[fFc_count].fIndex = fore;
				++fFc_count;
			}

			if (fFcs[last_back].fIndex != back)
			{
				fFcs[fFc_count].fWhich = BACK_WHICH;
				fFcs[fFc_count].fOffset = pos;
				fFcs[fFc_count].fIndex = back;
				++fFc_count;
			}

			if (fFcs[last_font].fIndex != font)
			{
				fFcs[fFc_count].fWhich = FONT_WHICH;
				fFcs[fFc_count].fOffset = pos;
				fFcs[fFc_count].fIndex = font;
				++fFc_count;
			}
		}
	}
	else
	{
		fFcs = new FontColor [fFc_count = 3];
		fFcs[0].fWhich = FORE_WHICH;
		fFcs[0].fOffset = 0;
		fFcs[0].fIndex = fore;
		fFcs[1].fWhich = BACK_WHICH;
		fFcs[1].fOffset = 0;
		fFcs[1].fIndex = back;
		fFcs[2].fWhich = FONT_WHICH;
		fFcs[2].fOffset = 0;
		fFcs[2].fIndex = font;
	}
}

void
Line::FigureEdges (
	Theme *theme,
	float width)
{
	delete [] fEdges;
	fEdges = new int16 [fLength];

	int16 cur_fFcs (0), next_fFcs (0), cur_font (0);

	fEdge_count = 0;
	while (cur_fFcs < fFc_count)
	{
		if (fFcs[cur_fFcs].fWhich == FONT_WHICH)
		{
			cur_font = cur_fFcs;
			break;
		}

		++cur_fFcs;
	}

	while (cur_fFcs < fFc_count)
	{
		int16 last_offset (fFcs[cur_fFcs].fOffset);
		next_fFcs = cur_fFcs + 1;

		while (next_fFcs < fFc_count)
		{
			// We want to break at every difference
			// but, we want to break on a font if available
			if (fFcs[next_fFcs].fOffset > last_offset)
			{
				while (next_fFcs < fFc_count
				&&		 fFcs[next_fFcs].fWhich != FONT_WHICH
				&&		 next_fFcs + 1 < fFc_count
				&&		 fFcs[next_fFcs + 1].fOffset == fFcs[next_fFcs].fOffset)
					++next_fFcs;

				break;
			}
			++next_fFcs;
		}

		if (fFcs[cur_fFcs].fWhich == FONT_WHICH)
			cur_font = cur_fFcs;

		int16 ccount;
		int16 seglen;

		if (next_fFcs == fFc_count)
		{
			ccount = CountChars (fFcs[cur_fFcs].fOffset, fLength - fFcs[cur_fFcs].fOffset);
			seglen = fLength - fFcs[cur_fFcs].fOffset;
		}
		else
		{
			ccount = CountChars (
				fFcs[cur_fFcs].fOffset,
				fFcs[next_fFcs].fOffset - fFcs[cur_fFcs].fOffset);
			seglen = fFcs[next_fFcs].fOffset - fFcs[cur_fFcs].fOffset;
		}

		const BFont &f (theme->FontAt (fFcs[cur_font].fIndex));

		float eshift[ccount];
		f.GetEscapements (
			fText + fFcs[cur_fFcs].fOffset,
			ccount,
			eshift);

		// This is not perfect, because we are including the left edge,
		// but BFont::GetEdges doesn't seem to work as we'd like

		int16 i;

		float incrementor = (fEdge_count > 0) ? fEdges[fEdge_count - 1] : 0;

		for (i = 0; i < ccount; ++i)
		{
			incrementor += eshift[i] * f.Size();

			fEdges[fEdge_count+i] = (int16) incrementor;

			// this little backfTracking routine is necessary in the case where an fFcs change
			// comes immediately after a UTF8-char, since all but the first edge will be 0
			// and thus the new edge's starting position will be thrown off if we don't
			// backtrack to the beginning of the char
			if ((fEdge_count + i > 0) && fEdges[fEdge_count + i - 1] == 0)
			{
				 int32 temp = fEdge_count + i - 1;
				 while (fEdges[--temp] == 0) ;
				 fEdges[fEdge_count + i] += fEdges[temp];
			}
		}

		for (i = fFcs[cur_fFcs].fOffset; i < fFcs[cur_fFcs].fOffset + seglen;)
		{
			int32 len (UTF8_CHAR_LEN (fText[i]) - 1);

			if (len)
			{
				int16 k;
				for (k = fEdge_count + ccount - 1; k > i; --k)
					fEdges[k + len] = fEdges[k];

				for (k = 1; k <= len; ++k)
					fEdges[i + k] = 0;

				ccount += len;
			}

			i += len + 1;
		}

		cur_fFcs = next_fFcs;
		fEdge_count += ccount;
	}

	SoftBreaks (theme, width);
}


void
Line::AddSoftBreak (SoftBreakEnd sbe, float &start, uint16 &fText_place,
	int16 &font, float &width, float &start_width, Theme *theme)
{
		fText_place = sbe.fOffset;

		if (fSoftie_size < fSoftie_used + 1)
		{
			SoftBreak *new_softies;

			new_softies = new SoftBreak [fSoftie_size += SOFTBREAK_STEP];

			if (fSofties)
			{
				memcpy (new_softies, fSofties, sizeof (SoftBreak) * fSoftie_used);
				delete [] fSofties;
			}

			fSofties = new_softies;
		}

		// consume whitespace
		while (fText_place + 1 < fLength
		&&		 isspace (fText[fText_place + 1]))
			++fText_place;

		fSofties[fSoftie_used].fOffset = fText_place;
		fSofties[fSoftie_used].fHeight = 0.0;
		fSofties[fSoftie_used].fAscent = 0.0;

		int16 last (font);
		while (font < fFc_count)
		{
			const BFont &f (theme->FontAt (fFcs[font].fIndex));
			font_height fh;
			float height;

			f.GetHeight (&fh);

			height = ceil (fh.ascent + fh.descent + fh.leading);
			if (fSofties[fSoftie_used].fHeight < height)
				fSofties[fSoftie_used].fHeight = height;
			if (fSofties[fSoftie_used].fAscent < fh.ascent)
				fSofties[fSoftie_used].fAscent = fh.ascent;

			// now try and find next
			while (++font < fFc_count)
				if (fFcs[font].fWhich == FONT_WHICH)
					break;

			if (font == fFc_count
			||	fFcs[font].fOffset > fText_place)
			{
				font = last;
				break;
			}

			last = font;
		}

		if (fText_place < fLength)
			start = fEdges[fText_place];

		fBottom += fSofties[fSoftie_used++].fHeight;
		fText_place += UTF8_CHAR_LEN (fText[fText_place]);
		width = start_width - MARGIN_INDENT;
}

void
Line::SoftBreaks (Theme *theme, float start_width)
{
	float margin (ceil (MARGIN_WIDTH / 2.0));
	float width (start_width);
	float start (0.0);
	uint16 fText_place (0);
	int16 space_place (0);
	int16 font (0);

	fSoftie_used = 0;
	fBottom = fTop;

	// find first font
	while (font < fFc_count && fFcs[font].fWhich != FONT_WHICH)
		++font;

	while (fText_place < fLength)
	{
			while (space_place < fSpace_count)
			{
				if (fEdges[fSpaces[space_place]] - start > width)
					break;

				++space_place;
			}

			// we've reached the end of the line (but it might not all fit)
			// or we only have one space, so we check if we need to split the word
			if (space_place == fSpace_count
			||	space_place == 0
			||	fSpaces[space_place - 1] < fText_place)
			{
				// everything fits.. how wonderful (but we want at least one softbreak)
				if (fEdge_count == 0)
				{
					AddSoftBreak (SoftBreakEnd(fLength - 1), start, fText_place, font, width, start_width, theme);
					break;
				}

				int16 i (fEdge_count - 1);

				while (fEdges[i] == 0)
					--i;

				if (fEdges[i] - start <= width)
				{
					AddSoftBreak (SoftBreakEnd(fLength - 1), start, fText_place, font, width, start_width, theme);
					continue;
				}

				// we force at least one character
				// your font may be a little too large for your window!
				fText_place += UTF8_CHAR_LEN (fText[fText_place]);
				while (fText_place < fLength)
				{
					if (fEdges[fText_place] - start > width - margin)
						break;

					fText_place += UTF8_CHAR_LEN (fText[fText_place]);
				}
				AddSoftBreak (SoftBreakEnd(fText_place), start, fText_place, font, width, start_width, theme);
				continue;
			}

			// we encountered more than one space, so we rule out having to
			// split the word, if the current word will fit within the bounds
			int16 ccount1, ccount2;
			--space_place;

			ccount1 = fSpaces[space_place];
			ccount2 = fSpaces[space_place+1] - ccount1;

			int16 i (ccount1 - 1);
			while (fEdges[i] == 0)
				--i;

			if (fEdges[ccount1 + ccount2] - fEdges[i] < width - margin)
				{
					AddSoftBreak (SoftBreakEnd(fSpaces[space_place]), start, fText_place, font, width, start_width, theme);
					continue;
				}

			// We need to break up the really long word
			fText_place = fSpaces[space_place];
			while (fText_place < fEdge_count)
			{
				if ((fEdges[fText_place] - start) > width)
					break;

				fText_place += UTF8_CHAR_LEN (fText[fText_place]);
			}
	}

	fBottom -= 1.0;
}

int16
Line::CountChars (int16 pos, int16 len)
{
	int16 ccount (0);

	if (pos >= fLength)
		return ccount;

	if (pos + len > fLength)
		len = fLength - pos;

	register int16 i = pos;
	while (i < pos + len)
	{
		i += UTF8_CHAR_LEN(fText[i]);
		++ccount;
	}

	return ccount;
}

size_t
Line::SetStamp (const char *format, bool was_on)
{
	size_t size (0);
	int32 i (0);

	if (was_on)
	{
		int16 offset (fFcs[4].fOffset + 1);

		if (fUrls)
		{
			for (i = 0; i < fUrls->CountItems(); i++)
				fUrls->ItemAt(i)->fOffset -= offset;
		}
		memmove (fText, fText + offset, fLength - offset);
		fText[fLength -= offset] = '\0';

		for (i = 6; i < fFc_count; ++i)
		{
			fFcs[i].fOffset -= offset;
			fFcs[i - 6] = fFcs[i];
		}

		fFc_count -= 6;
	}

	if (format)
	{
		char buffer[1024];
		struct tm curTime;

		localtime_r (&fStamp, &curTime);
		size = strftime (buffer, 1023, format, &curTime);
		if (fUrls)
		{
			for (i = 0; i < fUrls->CountItems(); i++)
				fUrls->ItemAt(i)->fOffset += size;
		}

		char *new_fText;

		new_fText = new char [fLength + size + 2];
		memcpy (new_fText, buffer, size);
		new_fText[size++] = ' ';
		new_fText[size] = '\0';

		if (fText)
		{
			memcpy (new_fText + size, fText, fLength);
			delete [] fText;
		}

		fText = new_fText;
		fText[fLength += size] = '\0';

		FontColor *new_fFcs;
		new_fFcs = new FontColor [fFc_count + 6];

		if (fFcs)
		{
			memcpy (
				new_fFcs + 6,
				fFcs,
				fFc_count * sizeof (FontColor));
			delete [] fFcs;
		}
		fFcs = new_fFcs;
		fFc_count += 6;

		fFcs[0].fWhich	= FORE_WHICH;
		fFcs[0].fIndex	= Theme::TimestampFore;
		fFcs[0].fOffset	= 0;
		fFcs[1].fWhich	= BACK_WHICH;
		fFcs[1].fIndex	= Theme::TimestampBack;
		fFcs[1].fOffset	= 0;
		fFcs[2].fWhich	= FONT_WHICH;
		fFcs[2].fIndex	= Theme::TimestampFont;
		fFcs[2].fOffset	= 0;

		fFcs[3].fWhich	= FORE_WHICH;
		fFcs[3].fIndex	= Theme::TimespaceFore;
		fFcs[3].fOffset	= size - 1;
		fFcs[4].fWhich	= BACK_WHICH;
		fFcs[4].fIndex	= Theme::TimespaceBack;
		fFcs[4].fOffset	= size - 1;
		fFcs[5].fWhich	= FONT_WHICH;
		fFcs[5].fIndex	= Theme::TimespaceFont;
		fFcs[5].fOffset	= size - 1;

		for (i = 6; i < fFc_count; ++i)
			fFcs[i].fOffset += size;
	}

	return size;
}

void
Line::SelectWord (int16 *start, int16 *end)
{
	int16 start_tmp (*start), end_tmp (*end);

	while(start_tmp > 0 && fText[start_tmp-1] != ' ')
			start_tmp--;

	while ((end_tmp - 1) < fLength && fText[end_tmp] != ' ')
			end_tmp++;

	while (end_tmp >= fLength)
		--end_tmp;

	*start = start_tmp;
	*end = end_tmp;
}

bool
RunView::FindText(const char *text)
{
	bool result (false);
	if (text != NULL)
	{
		for (int32 i = 0; i < fLine_count; i++)
		{
			char *offset (NULL);
			if ((offset = strstr(fLines[i]->fText, text)) != NULL)
			{
				SelectPos start (i, offset - text),
					end (i, (offset - text) + strlen(text));
				Select(start, end);
				ScrollTo(0.0, fLines[i]->fTop);
				result = true;
				break;
			}
		}
	}
	return result;
}

void
RunView::ScrollToSelection(void)
{
	if (fLine_count > 0)
	{
		if (fSp_start != fSp_end)
		{
			ScrollTo(0.0, fLines[fSp_start.fLine]->fTop);
		}
	}
}

void
RunView::ScrollToBottom(void)
{
	if (fLine_count > 0)
	{
		ScrollTo(0.0, fLines[fLine_count - 1]->fTop);
	}
}
