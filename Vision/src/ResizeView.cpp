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
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Ted Stodgell <kart@hal-pc.org>
 */

#include <Window.h>
#include <Message.h>
#include <assert.h>
 
#include "ResizeView.h"
#include "VisionBase.h"

  
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
  SetMouseEventMask (B_POINTER_EVENTS, B_NO_POINTER_HISTORY);
  mousePressed = true;
}
 
void
ResizeView::MouseUp (BPoint)
{
  mousePressed = false;
}
 
void
ResizeView::MouseMoved (BPoint current, uint32 transit, const BMessage *)
{
  if (transit == B_EXITED_VIEW && !mousePressed)
  {
    SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
    return;
  }
  else
    SetViewCursor(&cursor);

  if (mousePressed)
  {
    BWindow *window (Window ());
    BMessage msg (M_RESIZE_VIEW);
    msg.AddPointer ("view", attachedView);
    msg.AddFloat ("delta", (current.x - Bounds().right));
    window->PostMessage (&msg);
  }
}
