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
 * Contributor(s): Wade Majors <guru@startrek.com>
 */

// Class description:
// VTextControl is a derivative of BTextControl which adds context menus

// it's intention is to be fully compliant and portable, so it can easily
// be dropped into other applications as well.

#include <PopUpMenu.h>
#include <MenuItem.h>

#include <Window.h>

#include <stdio.h>

#include "VTextControl.h"

VTextControl::VTextControl (BRect frame, const char *name, const char *label,
                            const char *text, BMessage *vtcmessage,
                            uint32 resizingMode, uint32 flags)
  : BTextControl(
    frame,
    name,
    label,
    text,
    vtcmessage,
    resizingMode,
    flags)
{
}

VTextControl::VTextControl (BMessage *data)
  : BTextControl(
    data)
{

}

void
VTextControl::AllAttached (void)
{

  myPopUp = new BPopUpMenu("Context Menu", false, false);

  BMenuItem *item;
  myPopUp->AddItem (item = new BMenuItem("Copy", new BMessage (B_COPY)));
  
  myPopUp->SetFont (be_plain_font);
  myPopUp->SetTargetForItems (this);

}

void
VTextControl::MouseDown (BPoint myPoint)
{
  printf ("click\n");
  bool handled (false);

  BMessage *inputMsg (Window()->CurrentMessage());
  int32 mousebuttons (0),
        keymodifiers (0);

  inputMsg->FindInt32 ("buttons", &mousebuttons);
  inputMsg->FindInt32 ("modifiers", &keymodifiers);  

  if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
  && (keymodifiers & B_SHIFT_KEY)   == 0
  && (keymodifiers & B_OPTION_KEY)  == 0
  && (keymodifiers & B_COMMAND_KEY) == 0
  && (keymodifiers & B_CONTROL_KEY) == 0)
  {
    myPopUp->Go (
      ConvertToScreen (myPoint),
      true,
      false);
    handled = true;
  }
  
  if (!handled)
    BTextControl::MouseDown (myPoint); 

}

VTextControl::~VTextControl (void)
{
  delete myPopUp;
}

