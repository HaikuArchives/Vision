// BRect-like class
//
// see License at the end of file

#ifndef FRECT__
#define FRECT__

#include "PlatformDefines.h"

#include <gdk/gdk.h>
#include <StdDefs.h>
#include <Point.h>

class FRect {
public:
	FRect()
		:	left(0),
			top(0),
			right(-1),
			bottom(-1)
		{}

	FRect(const FRect &rect)
		:	left(rect.left),
			top(rect.top),
			right(rect.right),
			bottom(rect.bottom)
		{}

	FRect(float left, float top, float right, float bottom)
		:	left(left),
			top(top),
			right(right),
			bottom(bottom)
		{}

	FRect(FPoint leftTop, FPoint rightBottom)
		:	left(leftTop.x),
			top(leftTop.y),
			right(rightBottom.x),
			bottom(rightBottom.y)
		{}

	FRect(const GdkRectangle &rect)
		:	left(rect.x),
			top(rect.y),
			right(rect.x + rect.width),
			bottom(rect.y + rect.height)
		{}

	GdkRectangle GdkRect() const
		{
			GdkRectangle result;
			result.x = (gint16)left;
			result.y = (gint16)top;
			result.width =(gint16)(right - left);
			result.height = (gint16)(bottom - top);

			return result;
		}
		
	const FRect &Set(float newLeft, float newTop, float newRight, float newBottom)
		{
			left = newLeft;
			top = newTop;
			right = newRight;
			bottom = newBottom;

			return *this;
		}

	float Width() const
		{ return right - left; }

	float Height() const
		{ return bottom - top; }

	int32 IntegerWidth() const
		{ return (int32)Width(); }

	int32 IntegerHeight() const
		{ return (int32)Height(); }

	void SetWidth(float width)
		{ right = left + width; }
			
	void SetHeight(float height)
		{ bottom = top + height; }
			
	const FRect &InsetBy(float x, float y)
		{
			left += x;
			right -= x;
			top += y;
			bottom -= y;
	
			return *this; 
		}

	bool operator==(const FRect &rect) const
		{
			return rect.left == left
				&& rect.top == top
				&& rect.right == right
				&& rect.bottom == bottom;
		}

	const FRect &operator&(const FRect &rect)
		{
			if (left < rect.left)
				left = rect.left;
	
			if (right > rect.right)
				right = rect.right;
	
			if (top < rect.top)
				top = rect.top;
	
			if (bottom > rect.bottom)
				bottom = rect.bottom;
	
			return *this;
		}
	
	const FRect &operator|(const FRect &rect)
		{
			if (left > rect.left)
				left = rect.left;
	
			if (right < rect.right)
				right = rect.right;
	
			if (top > rect.top)
				top = rect.top;
	
			if (bottom < rect.bottom)
				bottom = rect.bottom;
	
			return *this;
		}
	
	bool Intersects(const FRect &rect) const
		{
			return rect.bottom > top
				&& rect.left < right
				&& rect.right > left
				&& rect.top < bottom;
		}

	bool Contains(FPoint point) const
		{
			return point.x >= left && point.x <= right
				&& point.y >= top && point.y <= bottom;
		}
	
	bool Contains(FRect rect) const
		{
			return rect.left >= left && rect.right <= right
				&& rect.top >= top && rect.bottom <= bottom;
		}
	
	void OffsetTo(float x, float y)
		{
			right = x + right - left;
			bottom = y + bottom - top;
			left = x;
			top = y;
		}

	void OffsetTo(FPoint point)
		{
			right = point.x - left;
			bottom = point.y - top;
			left = point.x;
			top = point.y;
		}

	void OffsetBy(float x, float y)
		{
			left += x;
			right += x;
			bottom += y;
			top += y;
		}

	void OffsetBy(FPoint point)
		{
			left += point.x;
			right += point.x;
			bottom += point.y;
			top += point.y;
		}

	FPoint LeftBottom() const
		{ return FPoint(left, bottom); }
	
	FPoint LeftTop() const
		{ return FPoint(left, top); }
	
	FPoint RightBottom() const
		{ return FPoint(right, bottom); }
	
	FPoint RightTop() const
		{ return FPoint(right, top); }

	void SetLeftTop(FPoint leftTop)
		{
			left = leftTop.x;
			top = leftTop.y;
		}
	
	void SetRightBottom(FPoint rightBottom)
		{
			right = rightBottom.x;
			bottom = rightBottom.y;
		}
	
	void PrintToStream(const char * = "") const;

	float left;
	float top;
	float right;
	float bottom;
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
