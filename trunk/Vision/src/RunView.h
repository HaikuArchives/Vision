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
 
#ifndef RUNVIEW_H_
#define RUNVIEW_H_

#define LINE_COUNT		1000

#include <View.h>

struct Line;
class Theme;
class RunView;
class BScrollView;
class BCursor;
class BMessageRunner;
class BPopUpMenu;

class SelectPos
{
	public:

	int16					line;
	int16					offset;

							SelectPos (
								int16 selLine = 0,
								int16 selOffset = 0)
								:	line (selLine),
									offset (selOffset)
							{ }

							SelectPos (const SelectPos &pos)
								:	line (pos.line),
									offset (pos.offset)
							{ }

							~SelectPos (void)
							{ }

	SelectPos			&operator = (const SelectPos &pos)
							{
								line = pos.line;
								offset = pos.offset;

								return *this;
							}
							
	inline int			operator == (const SelectPos &rhs) const
							{
								return ((line == rhs.line) && (offset == rhs.offset));
							}
	

};

class RunView : public BView
{
	BScrollView			*scroller;
	BCursor				*URLCursor;
	Theme					*theme;

	Line					*working;
	Line					*lines[LINE_COUNT];
	int16					line_count;

	char					*stamp_format;
	char					*clipping_name;
	
	SelectPos			sp_start, sp_end;
	
	int32							tracking;
	SelectPos							track_offset;
	BMessageRunner				*off_view_runner;
	bigtime_t					off_view_time;

	bool 				resizedirty;
	bool				fontsdirty;
	BPopUpMenu			*myPopUp;
	BPoint				lastClick;
	bigtime_t			lastClickTime;

	bool					RecalcScrollBar (bool constrain);
	void					ResizeRecalc (void);
	void					FontChangeRecalc (void);
	void					ExtendTrackingSelect (BPoint);
	void					ShiftTrackingSelect (BPoint, bool, bigtime_t);
	void					CheckURLCursor (BPoint);
	void					BuildPopUp (void);
	uint16				CheckClickCount (BPoint);
	bool				CheckClickBounds (const SelectPos &, const BPoint &) const;
	
	public:

							RunView (
								BRect,
								const char *,
								Theme *,
								uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP,
								uint32 = 0UL);
	virtual				~RunView (void);

	virtual void		AttachedToWindow (void);
	virtual void		DetachedFromWindow (void);
	virtual void		FrameResized (float, float);
	virtual void		TargetedByScrollView (BScrollView *);
	virtual void		Show ();
	virtual void		Draw (BRect);
	virtual void		MessageReceived (BMessage *);

	virtual void		SetViewColor (rgb_color);
	void					SetViewColor (uchar red, uchar green, uchar blue, uchar alpha = 255)
							{ rgb_color color = {red, green, blue, alpha}; SetViewColor (color); }

	
	virtual void		MouseDown (BPoint);
	virtual void		MouseMoved (BPoint, uint32, const BMessage *);
	virtual void		MouseUp (BPoint);
	

	void					Append (const char *, int32, int16, int16, int16);
	void					Append (const char *, int16, int16, int16);
	void					Clear (void);

	int16					LineCount (void) const;
	const char			*LineAt (int16) const;

	void					SetTimeStampFormat (const char *);
	void					SetTheme (Theme *);

	SelectPos			PositionAt (BPoint) const;
	BPoint				PointAt (SelectPos) const;
	
	BRect					GetTextFrame (const SelectPos &, const SelectPos &) const;
	bool					IntersectSelection (const SelectPos &, const SelectPos &) const;
	void					GetSelectionText (BString &) const;
	void					Select (const SelectPos &, const SelectPos &);
	void					SelectAll (void);
	void					SetClippingName (const char *);
};

#endif
