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
 *                 Todd Lair
 */

#include "PrefDCC.h"
#include "Vision.h"
#include "VTextControl.h"

#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>

#include <ctype.h>
#include <stdlib.h>

const uint32 M_BLOCK_SIZE_CHANGED   = 'mdsc';
const uint32 M_DEFAULT_PATH_CHANGED = 'mdpc';
const uint32 M_AUTO_ACCEPT_CHANGED  = 'mdac';
 
DCCPrefsView::DCCPrefsView (BRect frame)
  : BView (frame, "DCC prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BMenu *menu (new BMenu ("DCC Block Size"));
  menu->AddItem (new BMenuItem ("1024", new BMessage (M_BLOCK_SIZE_CHANGED)));
  menu->AddItem (new BMenuItem ("2048", new BMessage (M_BLOCK_SIZE_CHANGED)));
  menu->AddItem (new BMenuItem ("4096", new BMessage (M_BLOCK_SIZE_CHANGED)));
  menu->AddItem (new BMenuItem ("8192", new BMessage (M_BLOCK_SIZE_CHANGED)));
  blockSize = new BMenuField (BRect (0,0,0,0), NULL, "DCC Block size: ", menu);
  AddChild (blockSize);
  autoAccept = new BCheckBox (BRect (0,0,0,0), NULL, "Automatically accept incoming sends",
    new BMessage (M_AUTO_ACCEPT_CHANGED));
  AddChild (autoAccept);
  defDir = new VTextControl (BRect (0,0,0,0), NULL, "Default path: ", "", new BMessage (M_DEFAULT_PATH_CHANGED));
  defDir->SetDivider (defDir->StringWidth ("Default path: " + 5));
  AddChild (defDir);  
}

DCCPrefsView::~DCCPrefsView (void)
{
}

void
DCCPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow ();
  blockSize->Menu()->SetTargetForItems (this);
  autoAccept->SetTarget (this);
  defDir->SetTarget (this);
  defDir->ResizeToPreferred ();
  defDir->ResizeTo (Bounds().Width() - 15, defDir->Bounds().Height());
  defDir->MoveTo (10, 10);
  autoAccept->ResizeToPreferred ();
  autoAccept->MoveTo (defDir->Frame().left, defDir->Frame().bottom + 5);
  blockSize->ResizeToPreferred ();
  blockSize->ResizeTo (Bounds().Width() - 15, blockSize->Bounds().Height());
  blockSize->SetDivider (blockSize->StringWidth ("DCC Block size: ") + 5);
  blockSize->MoveTo (autoAccept->Frame().left, autoAccept->Frame().bottom + 5);
  blockSize->Menu()->SetLabelFromMarked (true);
  
  const char *defPath (vision_app->GetString ("dccDefPath"));
  defDir->SetText (defPath);
  
  if (vision_app->GetBool ("dccAutoAccept"))
    autoAccept->SetValue (B_CONTROL_ON);
  
  else
    defDir->SetEnabled (false);
  
  const char *dccBlock (vision_app->GetString ("dccBlockSize"));
  
  BMenuItem *item (blockSize->Menu()->FindItem (dccBlock));
  if (item)
    dynamic_cast<BInvoker *>(item)->Invoke();
}

void
DCCPrefsView::AllAttached (void)
{
  BView::AllAttached ();
}

void
DCCPrefsView::FrameResized (float width, float height)
{
  BView::FrameResized (width, height);
}

void
DCCPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_BLOCK_SIZE_CHANGED:
      {
        BMenuItem *it (NULL);
        msg->FindPointer ("source", reinterpret_cast<void **>(&it));
        if (it)
          vision_app->SetString ("dccBlockSize", 0, it->Label());
      }
      break;
      
    case M_DEFAULT_PATH_CHANGED:
      {
        const char *path (defDir->Text());
        if (path)
          vision_app->SetString ("dccDefPath", 0, path);
      }
      break;
      
    case M_AUTO_ACCEPT_CHANGED:
      {
        int32 val (autoAccept->Value());
        defDir->SetEnabled (val == B_CONTROL_ON);
        vision_app->SetBool ("dccAutoAccept", val);
      }
      break;
      
    default:
      BView::MessageReceived (msg);
      break;
  }
}
