#ifndef RUNVIEW_H_
#define RUNVIEW_H_

#define LINE_COUNT		1000

#include <View.h>

struct Line;
class Theme;
class RunView;
class BScrollView;

class SelectPos
{
	public:

	int16					line;
	int16					offset;

							SelectPos (
								int16 line = 0,
								int16 offset = 0)
								:	line (line),
									offset (offset)
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

};

class RunView : public BView
{
	BScrollView			*scroller;
	Theme					*theme;
	BRect					*boxbuf;
	int16					boxbuf_size;

	Line					*working;
	Line					*lines[LINE_COUNT];
	int16					line_count;

	char					*stamp_format;

	SelectPos			sp_start, sp_end;
	
	bool 				resizedirty;
	bool				fontsdirty;


	bool					RecalcScrollBar (bool constrain);
	void					ResizeRecalc (void);
	void					FontChangeRecalc (void);
	
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
	void					GetSelection (SelectPos *, SelectPos *) const;
	void					Select (SelectPos, SelectPos);
	void					SelectAll (void);
};

#endif
