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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 */

#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <View.h>

class ColorSwatch : public BView {
public:
	ColorSwatch(const char*, rgb_color, uint32 = B_WILL_DRAW | B_NAVIGABLE);

	virtual ~ColorSwatch();

	virtual void AttachedToWindow();
	virtual void Draw(BRect);

	rgb_color ValueAsColor() const;
	virtual void SetColor(rgb_color);

	BRect ColorRect() const;

private:
	rgb_color ShiftColor(rgb_color, float) const;
	rgb_color Inverted() const;

protected:
	rgb_color fColor;
	rgb_color fAlpha;
};

#endif
