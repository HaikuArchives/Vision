// rgb_color-like class and color manipulating utilities
//
// see License at the end of file

#include "PlatformDefines.h"

#include "Color.h"

#include <stdio.h>

void
rgb_color::PrintToStream(const char *message) const
{
	printf("%s: r%d g%d b%d\n", message, red, green, blue);
}

static uchar 
ShiftComponent(uchar color, float percent)
{
	if (percent >= 1.0)
		return (uchar)(color * (2 - percent));

	return (uchar)(255 - percent * (255 - color));
}

rgb_color 
rgb_color::ShiftBy(float by) const
{
	rgb_color result;
	result.red = ShiftComponent(red, by);
	result.green = ShiftComponent(green, by);
	result.blue = ShiftComponent(blue, by);
	return result;
}

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
