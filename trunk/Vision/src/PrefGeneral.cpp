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
 */

#include "PrefGeneral.h"
#include "PrefApp.h"
#include "PrefColor.h"
#include "PrefCommand.h"
#include "PrefEvent.h"

#include <stdio.h>

#include <ListView.h>
#include <Box.h>
#include <ScrollView.h>

GeneralPrefsView::GeneralPrefsView (BRect frame, const char *title, uint32 redraw, uint32 flags)
  : BView (frame, title, redraw, flags),
              lastindex (0)
{
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  prefsItems[0] = new AppWindowPrefsView (BRect(0,0,0,0));
  prefsItems[1] = new ColorPrefsView (BRect (0,0,0,0));
  prefsBox = new BBox (BRect (0.0, 0.0, prefsItems[1]->Bounds().right + 10 + be_plain_font->StringWidth("W"), prefsItems[0]->Bounds().bottom + be_plain_font->Size() / 2), "Bleat", B_FOLLOW_ALL_SIDES);
  prefsBox->AddChild(prefsItems[0]);
  prefsBox->AddChild(prefsItems[1]);
  prefsList = new BListView (BRect (0.0, 0.0, prefsItems[0]->Bounds().right / 2, prefsItems[0]->Bounds().bottom), "PrefsList", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
  prefsList->MoveTo(5, 5);
  prefsList->AddItem (new BStringItem ("Application"));
  prefsList->AddItem (new BStringItem ("Colors"));
  prefsList->AddItem (new BStringItem ("Commands"));
  prefsList->AddItem (new BStringItem ("Events"));
  prefsList->SetSelectionMessage (new BMessage (M_GENERALPREFS_SELECTION_CHANGED));
  BScrollView *scroller (new BScrollView("list scroller", prefsList, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, 0, false, true));
  AddChild(scroller);
  ResizeTo(scroller->Frame().right + prefsBox->Bounds().Width() + 10, prefsBox->Bounds().Height() + 5);
  prefsBox->MoveTo(scroller->Frame().right + 5, 5);
  AddChild (prefsBox);
  BRect bounds (prefsBox->Bounds());
  bounds.left += 3;
  bounds.right -= 3;
  bounds.top += 12;
  bounds.bottom -= 5;

  prefsItems[0]->MoveTo(be_plain_font->StringWidth("i"), be_plain_font->Size() * 1.5);
  prefsItems[0]->ResizeBy(be_plain_font->StringWidth("i") * 3, -1.2 * (be_plain_font->Size()));
  prefsItems[1]->MoveTo(be_plain_font->StringWidth("i"), be_plain_font->Size() * 1.5);
  prefsItems[1]->ResizeTo(bounds.Width() - 3, bounds.Height() - 3);

  prefsItems[1]->Hide();
  prefsItems[2] = new CommandPrefsView (bounds);
  prefsBox->AddChild (prefsItems[2]);
  prefsItems[2]->Hide();
  prefsItems[3] = new EventPrefsView (bounds);
  prefsBox->AddChild (prefsItems[3]);
  prefsItems[3]->Hide();
  
  for (int32 i = 4; i < C_PREFS_COUNT; i++)
    prefsItems[i] = NULL;
}

GeneralPrefsView::~GeneralPrefsView (void)
{
  while (prefsList->CountItems() != 0)
    delete prefsList->RemoveItem (0L);
}

void
GeneralPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow();
}

void
GeneralPrefsView::AllAttached (void)
{
  BView::AllAttached();
  prefsList->SetTarget(this);
  prefsList->Select(0);
}

void
GeneralPrefsView::Show (void)
{
  BView::Show();
}

void
GeneralPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_GENERALPREFS_SELECTION_CHANGED:
    {
      int32 index (msg->FindInt32 ("index"));
      if (index < 0) return;
      prefsItems[lastindex]->Hide();
      BStringItem *item ((BStringItem *)prefsList->ItemAt(index));
      prefsBox->SetLabel (item->Text());
      prefsItems[index]->Show();
      prefsList->MakeFocus (false);
      lastindex = index;
      Invalidate();
    }
    break;
      
    default:
      BView::MessageReceived(msg);
  }
}
