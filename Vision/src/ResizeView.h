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
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Ted Stodgell <kart@hal-pc.org>
 */
 
#ifndef _RESIZEVIEW_H
#define _RESIZEVIEW_H

#include <View.h>
#include <Cursor.h>

// horizontal resize cursor taken from OpenTracker, see www.opentracker.org for license

const unsigned char kHorizontalResizeCursor[] = {
	16, 1, 7, 7,
	0, 0, 1, 0, 1, 0, 1, 0, 9, 32, 25, 48, 57, 56, 121, 60,
	57, 56, 25, 48, 9, 32, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0,
	3, 128, 3, 128, 3, 128, 15, 224, 31, 240, 63, 248, 127, 252, 255, 254,
	127, 252, 63, 248, 31, 240, 15, 224, 3, 128, 3, 128, 3, 128, 0, 0
};

class ResizeView : public BView
{
	public:
		ResizeView (BView *, BRect, const char * = "resizeView", uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, uint32 = 0);
		virtual ~ResizeView (void);
		virtual void MouseDown(BPoint);
		virtual void MouseMoved (BPoint, uint32, const BMessage *);
		virtual void MouseUp (BPoint);
	
	private:
		bool mousePressed;
		BView *attachedView;
		BCursor cursor;
};

#endif // _RESIZEVIEW_H

