#ifndef STRING_VIEW_H__
#define STRING_VIEW_H__

#include <PlatformDefines.h>

#include <InterfaceDefs.h>
#include <View.h>


class FStringView : public FView {
public:
	FStringView(BRect bounds, const char *name, const char *text,
		uint32 resizeFlags = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		uint32 flags = B_WILL_DRAW);
	virtual ~FStringView();
	
	
	void SetText(const char *text);
	const char *Text() const;
	
	void SetAlignment(alignment flag);
	alignment Alignment() const;

	virtual void GetPreferredSize(float *width, float *height);
	virtual void SetFont(const FFont *, uint32 = 0);

protected:
	virtual	void AttachedToWindow();
	virtual	void Draw(BRect bounds);
	
private:	
	alignment fAlignment;
};

#endif

/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler
Copyright (c) 1999-2001, Gene Ragan

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
