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
 * Contributor(s): Rene Gollent
 *
 */

#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "NotifyList.h"
#include "Theme.h"
#include "Vision.h"

NotifyList::NotifyList (BRect _frame)
  : BListView (_frame,
      "NotifyList",
      B_SINGLE_SELECTION_LIST,
      B_FOLLOW_ALL ),
      fActiveTheme (vision_app->ActiveTheme())
{
  fActiveTheme->ReadLock();
  SetViewColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));
  fActiveTheme->ReadUnlock();
}
 
NotifyList::~NotifyList (void)
{
  // empty for now
  MakeEmpty();
}

void
NotifyList::UpdateList(BList *newList)
{
  MakeEmpty();
  AddList(newList);
}

void
NotifyList::AttachedToWindow (void)
{
  
  BListView::AttachedToWindow ();
}

void
NotifyList::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_NOTIFYLIST_RESIZE:
      {
        ClientWindow *cWin (vision_app->pClientWin());
        cWin->DispatchMessage (msg, cWin->pCwDock());
        break;
      }
      
    default:
      BListView::MessageReceived (msg);
  }
}

NotifyListItem::NotifyListItem (const char *name, bool state)
  : BStringItem (name),
    fNotifyState (state)
{
  // empty c'tor
}

NotifyListItem::~NotifyListItem (void)
{
  // empty d'tor
}

void
NotifyListItem::SetState (bool newState)
{
  fNotifyState = newState;
}

bool
NotifyListItem::GetState(void)
{
  return fNotifyState;
}

void
NotifyListItem::DrawItem (BView *father, BRect frame, bool complete)
{
  Theme *fActiveTheme (vision_app->ActiveTheme());
  
  fActiveTheme->ReadLock();

  if (IsSelected())
  {
    father->SetHighColor (fActiveTheme->ForegroundAt (C_WINLIST_SELECTION));
    father->SetLowColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));    
    father->FillRect (frame);
  }
  else if (complete)
  {
    father->SetLowColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));
    father->FillRect (frame, B_SOLID_LOW);
  }

  rgb_color color = fActiveTheme->ForegroundAt ((fNotifyState) ? C_NOTIFY_ON : C_NOTIFY_OFF);

  font_height fh;
  father->GetFontHeight (&fh);

  father->MovePenTo (
    frame.left + 4,
    frame.bottom - fh.descent);

  BString drawString (Text());

  if (IsSelected())
    color = fActiveTheme->ForegroundAt (C_NOTIFY_OFF);
  
  fActiveTheme->ReadUnlock();
  
  father->SetHighColor (color);

  father->SetDrawingMode (B_OP_OVER);
  father->DrawString (drawString.String());
  father->SetDrawingMode (B_OP_COPY);
}
