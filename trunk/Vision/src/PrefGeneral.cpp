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
#include "PrefDCC.h"
#include "PrefFont.h"
#include "PrefCommand.h"
#include "PrefEvent.h"
#include "PrefLog.h"

#include <stdio.h>

#include <Box.h>
#include <ListView.h>
#include <ScrollView.h>

GeneralPrefsView::GeneralPrefsView (BRect frame, const char *title, uint32 redraw, uint32 flags)
  : BView (frame, title, redraw, flags),
              fLastindex (0)
{
  SetViewColor (ui_color(B_PANEL_BACKGROUND_COLOR));
  fPrefsItems[0] = new AppWindowPrefsView (BRect(0,0,0,0));
  fPrefsItems[1] = new ColorPrefsView (BRect (0,0,0,0));
  fPrefsBox = new BBox (BRect (0.0, 0.0, fPrefsItems[1]->Bounds().right + 10 + be_plain_font->StringWidth("W"), fPrefsItems[0]->Bounds().bottom + be_plain_font->Size() / 2), "Bleat", B_FOLLOW_ALL_SIDES);
  fPrefsBox->AddChild(fPrefsItems[0]);
  fPrefsBox->AddChild(fPrefsItems[1]);
  fPrefsList = new BListView (BRect (0.0, 0.0, fPrefsItems[0]->Bounds().right / 2, fPrefsItems[0]->Bounds().bottom), "PrefsList", B_SINGLE_SELECTION_LIST, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM);
  fPrefsList->MoveTo(5, 5);
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_APP_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_COLOR_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_FONT_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_COMMAND_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_EVENT_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_DCC_ITEM));
  fPrefsList->AddItem (new BStringItem (S_PREFGEN_LOG_ITEM));
  fPrefsList->SetSelectionMessage (new BMessage (M_GENERALPREFS_SELECTION_CHANGED));
  BScrollView *scroller (new BScrollView("list scroller", fPrefsList, B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM, 0, false, true));
  AddChild(scroller);
  ResizeTo(scroller->Frame().right + fPrefsBox->Bounds().Width() + 10, fPrefsBox->Bounds().Height() + 5);
  fPrefsBox->MoveTo(scroller->Frame().right + 5, 5);
  AddChild (fPrefsBox);
  BRect bounds (fPrefsBox->Bounds());
  bounds.left += 3;
  bounds.right -= 3;
  bounds.top += 12;
  bounds.bottom -= 5;

  fPrefsItems[0]->MoveTo(be_plain_font->StringWidth("i"), be_plain_font->Size() * 1.5);
  fPrefsItems[0]->ResizeBy(be_plain_font->StringWidth("i") * 3, -1.2 * (be_plain_font->Size()));
  fPrefsItems[1]->MoveTo(be_plain_font->StringWidth("i"), be_plain_font->Size() * 1.5);
  fPrefsItems[1]->ResizeTo(bounds.Width() - 3, bounds.Height() - 3);

  fPrefsItems[1]->Hide();
  fPrefsItems[2] = new FontPrefsView (bounds);
  fPrefsBox->AddChild (fPrefsItems[2]);
  fPrefsItems[2]->Hide();
  fPrefsItems[3] = new CommandPrefsView (bounds);
  fPrefsBox->AddChild (fPrefsItems[3]);
  fPrefsItems[3]->Hide();
  fPrefsItems[4] = new EventPrefsView (bounds);
  fPrefsBox->AddChild (fPrefsItems[4]);
  fPrefsItems[4]->Hide();
  
  fPrefsItems[5] = new DCCPrefsView (bounds);
  fPrefsBox->AddChild (fPrefsItems[5]);
  fPrefsItems[5]->Hide();
  
  fPrefsItems[6] = new LogPrefsView (bounds);
  fPrefsBox->AddChild (fPrefsItems[6]);
  fPrefsItems[6]->Hide();
}

GeneralPrefsView::~GeneralPrefsView (void)
{
  while (fPrefsList->CountItems() != 0)
    delete fPrefsList->RemoveItem (0L);
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
  fPrefsList->SetTarget(this);
  fPrefsList->Select(0);
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
      fPrefsItems[fLastindex]->Hide();
      BStringItem *item ((BStringItem *)fPrefsList->ItemAt(index));
      fPrefsBox->SetLabel (item->Text());
      fPrefsItems[index]->Show();
      fPrefsList->MakeFocus (false);
      fLastindex = index;
      Invalidate();
    }
    break;
      
    default:
      BView::MessageReceived(msg);
  }
}
