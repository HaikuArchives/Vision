
#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <View.h>

class ColorSwatch : public BView
{
	public:
							ColorSwatch (
								BRect,
								const char *,
								rgb_color,
								uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP,
								uint32 = B_WILL_DRAW | B_NAVIGABLE);

	virtual				~ColorSwatch (void);

	virtual void		AttachedToWindow (void);
	virtual void		Draw (BRect);

	rgb_color			ValueAsColor (void) const;
	virtual void		SetColor (rgb_color);

	BRect					ColorRect (void) const;

	private:
	rgb_color			ShiftColor (rgb_color, float) const;
	rgb_color			Inverted (void) const;

	protected:
	rgb_color			color;
	rgb_color			alpha;
};

#endif
