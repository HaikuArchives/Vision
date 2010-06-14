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

#include <Window.h>
#include <Message.h>
#include <assert.h>
 
#include "ResizeView.h"
#include "VisionBase.h"
#include "Vision.h"
	
ResizeView::ResizeView (BView *child, BRect frame, const char *title,
	uint32 resizeMode, uint32 flags) :
		BView (frame, title, resizeMode, flags),
		mousePressed (false),
		attachedView (child),
		cursor (kHorizontalResizeCursor)
{
	assert (attachedView != NULL);

	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
}
 
ResizeView::~ResizeView()
{
}
 
void
ResizeView::MouseDown (BPoint)
{
	SetMouseEventMask (B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS|B_SUSPEND_VIEW_FOCUS|B_NO_POINTER_HISTORY);
	mousePressed = true;
	vision_app->SetCursor (&cursor);
}
 
void
ResizeView::MouseUp (BPoint)
{
	mousePressed = false;
	vision_app->SetCursor (B_HAND_CURSOR);
}
 
void
ResizeView::MouseMoved (BPoint, uint32, const BMessage *)
{
	SetViewCursor (&cursor);
	if (mousePressed)
	{
		BWindow *window (Window ());
		BMessage *windowmsg (window->CurrentMessage());
		BPoint windowCoord;
		windowmsg->FindPoint ("where", &windowCoord);
		BMessage msg (M_RESIZE_VIEW);
		if (windowCoord.x <= 0.0 || windowCoord.x >= window->Bounds().right)
			return;
		msg.AddPoint ("loc", windowCoord);
		msg.AddPointer ("view", attachedView);
		window->PostMessage (&msg);
	}
}
