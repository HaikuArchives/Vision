// rgb_color-like class and color manipulating utilities
//
// see License at the end of file

#ifndef FCOLOR__
#define FCOLOR__

#include "PlatformDefines.h"

#include <gdk/gdk.h>

struct rgb_color {


	bool operator==(const rgb_color &color) const
		{
			return color.red == red
				&& color.green == green
				&& color.blue == blue
				&& color.alpha == alpha;
		}

	bool operator!=(const rgb_color &color) const
		{ return !operator==(color); }

	guint32 AsGdkRgb() const
		{ return (alpha << 24) | (red << 16) | (green << 8) | blue; }

	void PrintToStream(const char *) const;
	
	rgb_color ShiftBy(float) const;
	
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
};

const rgb_color B_TRANSPARENT_32_BIT = {0, 0, 0, 0};

inline rgb_color
Color(uint8 r, uint8 g, uint8 b, uint8 alpha = 255)
{
	rgb_color result;
	result.red = r;
	result.green = g;
	result.blue = b;
	result.alpha = alpha;

	return result;
}

#endif
/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met: 

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE,
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE. 
*/
