// BPoint-like class
//
// see License at the end of file

#ifndef FPOINT__
#define FPOINT__

#include "PlatformDefines.h"

class FPoint {
public:
	FPoint()
		:	x(0),
			y(0)
		{}

	FPoint(float x, float y)
		:	x(x),
			y(y)
		{}

	FPoint(const FPoint &point)
		:	x(point.x),
			y(point.y)
		{}


	FPoint &operator=(const FPoint &point)
		{
			x = point.x;
			y = point.y;
			return *this;
		}

	FPoint operator+(const FPoint &point) const
		{
			FPoint result;
			result.x = x + point.x;
			result.y = y + point.y;
			return result;
		}

	FPoint operator-(const FPoint &point) const
		{
			FPoint result;
			result.x = x - point.x;
			result.y = y - point.y;
			return result;
		}

	FPoint &operator+=(const FPoint &point)
		{
			x += point.x;
			y += point.y;
			return *this;
		}

	FPoint &operator-=(const FPoint &point)
		{
			x -= point.x;
			y -= point.y;
			return *this;
		}
	
	void Set (float newX, float newY) { x = newX; y = newY; }
	
	bool operator==(const FPoint &point) const
		{ return point.x == x && point.y == y; }

	bool operator!=(const FPoint &point) const
		{ return point.x != x || point.y != y; }

	float x;
	float y;
};


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
