// BBitmap-like class
//
// see License at the end of file

#ifndef F_BITMAP__
#define F_BITMAP__


#include "PlatformDefines.h"
#include "GraphicDefs.h"
#include "View.h"

typedef struct _GdkPixbuf GdkPixbuf;

class FBitmap {
public:
	FBitmap(FRect, int32);
	FBitmap (FRect bounds, color_space space, bool acceptsViews = false,
		 bool needContiguousMemory = false);
	virtual ~FBitmap();

	FRect Bounds() const;
	void SetBits(const unsigned char *bits, int32 size, int32, color_space color_depth);
	
	void *Bits() const;
	int32 BitsLength() const;
	
	int32 BytesPerRow() const;
	
	color_space ColorSpace() const;
	
	virtual void AddChild(FView *child);
	virtual bool RemoveChild(FView *child);

	bool Lock() const;
	void Unlock() const;

	// used by FView only.
	void Draw(FView *, const FRegion *, FRect, FRect) const;
	
private:
	GdkPixbuf *PixBuf() const
		{ return fPixbuf; }
	
	FRect fBounds;
	GdkPixbuf *fPixbuf;
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
