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
#include "PrefAppearance.h"
#include "PrefCommand.h"
#include "PrefEvent.h"

#include <stdio.h>

#include <ListView.h>
#include <Box.h>
#include <ScrollView.h>

GeneralPrefsView::GeneralPrefsView (BRect frame, const char *title, uint32 redraw, uint32 flags)
  : BView (frame, title, redraw, flags)
{
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  prefsItems[0] = new AppWindowPrefsView (BRect(0,0,0,0));
  prefsBox = new BBox (BRect (0.0, 0.0, prefsItems[0]->Bounds().right + 10 + be_plain_font->StringWidth("W"), prefsItems[0]->Bounds().bottom + be_plain_font->Size() / 2), "Bleat");
  ResizeTo(prefsBox->Bounds().right * 1.5 + 30, prefsBox->Bounds().bottom + 10);
  prefsBox->MoveTo((prefsBox->Bounds().Width() / 2) + B_V_SCROLL_BAR_WIDTH + 5, 5);
  prefsBox->AddChild(prefsItems[0]);
  prefsList = new BListView (BRect (0.0, 0.0, prefsItems[0]->Bounds().right / 2, prefsItems[0]->Bounds().bottom), "PrefsList");
  prefsList->MoveTo(5, 5);
  prefsList->AddItem (new BStringItem ("Application"));
  prefsList->AddItem (new BStringItem ("Appearance"));
  prefsList->AddItem (new BStringItem ("Commands"));
  prefsList->AddItem (new BStringItem ("Events"));
  prefsList->SetSelectionMessage (new BMessage (M_GENERALPREFS_SELECTION_CHANGED));
  BScrollView *scroller (new BScrollView("list scroller", prefsList, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));
  AddChild(scroller);
  AddChild (prefsBox);
  prefsBox->MoveBy(0, -3);
  prefsItems[0]->MoveTo(be_plain_font->StringWidth("i"), be_plain_font->Size() * 1.5);
  prefsItems[0]->ResizeBy(be_plain_font->StringWidth("i") * 3, -1.2 * (be_plain_font->Size()));
  BRect bounds (prefsBox->Bounds());
  bounds.left += 3;
  bounds.right -= 3;
  bounds.top += 12;
  bounds.bottom -= 5;

  prefsItems[1] = new AppearancePrefsView (bounds);
  prefsBox->AddChild (prefsItems[1]);
  prefsItems[1]->Hide();
  prefsItems[2] = new CommandPrefsView (bounds);
  prefsBox->AddChild (prefsItems[2]);
  prefsItems[2]->Hide();
  prefsItems[3] = new EventPrefsView (bounds);
  prefsBox->AddChild (prefsItems[3]);
  prefsItems[3]->Hide();
  
  for (int32 i = 4; i < C_PREFS_COUNT; i++)
    prefsItems[i] = NULL;
  prefsList->Select(0);
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
  if (prefsList->Target() != this)
  {
    prefsList->SetTarget(this);
    prefsList->Select(0);
  }
}

void
GeneralPrefsView::Show (void)
{
  BView::Show();
  lastindex = 0;
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
