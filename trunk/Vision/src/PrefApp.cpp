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

#include "PrefApp.h"
#include "Vision.h"

#include <stdio.h>

#include <CheckBox.h>

AppWindowPrefsView::AppWindowPrefsView (BRect frame)
  : BView (frame, "App/Window Prefs", B_FOLLOW_NONE, B_WILL_DRAW)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BMessage msg (M_APPWINDOWPREFS_SETTING_CHANGED);
  float maxWidth (0),
    maxHeight (0);
  BRect trackingBoundsRect (0.0, 0.0, 0, 0);
  BRect checkboxRect(Bounds());
  checkboxRect.bottom = checkboxRect.top;
  msg.AddString ("setting", "versionParanoid");
  fVersionParanoid = new BCheckBox (checkboxRect, "version Paranoid",
    S_PREFAPP_VERSION_PARANOID,
    new BMessage (msg));
  fVersionParanoid->SetValue ((!vision_app->GetBool ("versionParanoid")) ? B_CONTROL_ON : B_CONTROL_OFF);
  fVersionParanoid->MoveBy(be_plain_font->StringWidth("S"), 0);
  fVersionParanoid->ResizeToPreferred();
  trackingBoundsRect = fVersionParanoid->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height(); 
  AddChild (fVersionParanoid);
  
  checkboxRect.top += fVersionParanoid->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "catchAltW");
  fCatchAltW = new BCheckBox (checkboxRect, "catch AltW",
    S_PREFAPP_CMDW,
    new BMessage (msg));
  fCatchAltW->SetValue ((!vision_app->GetBool ("catchAltW")) ? B_CONTROL_ON : B_CONTROL_OFF);
  fCatchAltW->MoveBy(be_plain_font->StringWidth("S"), 0);
  fCatchAltW->ResizeToPreferred();
  trackingBoundsRect = fCatchAltW->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.2; 
  AddChild (fCatchAltW);
  
  checkboxRect.top += fCatchAltW->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "stripcolors");
  fStripColors = new BCheckBox (checkboxRect, "stripcolors",
    S_PREFAPP_STRIP_MIRC,
    new BMessage (msg));
  fStripColors->SetValue ((vision_app->GetBool ("stripcolors")) ? B_CONTROL_ON : B_CONTROL_OFF);
  fStripColors->MoveBy(be_plain_font->StringWidth("S"), 0);
  fStripColors->ResizeToPreferred();
  trackingBoundsRect = fStripColors->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  AddChild (fStripColors);

  checkboxRect.top += fStripColors->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "Newbie Spam Mode");
  fSpamMode = new BCheckBox (checkboxRect, "newbiespammode",
    S_PREFAPP_WARN_MULTILINE,
    new BMessage (msg));
  fSpamMode->SetValue ((vision_app->GetBool ("Newbie Spam Mode")) ? B_CONTROL_ON : B_CONTROL_OFF);
  fSpamMode->MoveBy(be_plain_font->StringWidth("S"), 0);
  fSpamMode->ResizeToPreferred();
  trackingBoundsRect = fSpamMode->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  AddChild (fSpamMode);
  
  ResizeTo(maxWidth, maxHeight);
}

AppWindowPrefsView::~AppWindowPrefsView (void)
{
}

void
AppWindowPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow();
}

void
AppWindowPrefsView::AllAttached (void)
{
  fVersionParanoid->SetTarget (this);
  fCatchAltW->SetTarget (this);
#if 0
  fTimeStamp->SetTarget (this);
  fLogEnabled->SetTarget (this);
  fLogFileTimestamp->SetTarget (this);
#endif
  fStripColors->SetTarget (this);
  fSpamMode->SetTarget (this);
  BView::AllAttached();
}

void
AppWindowPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
     case M_APPWINDOWPREFS_SETTING_CHANGED:
       {
         BControl *source (NULL);
         msg->FindPointer ("source", reinterpret_cast<void **>(&source));
         BString setting;
         msg->FindString ("setting", &setting);
         int32 value (source->Value() == B_CONTROL_ON);
         if ((setting.ICompare ("catchAltW") == 0) || (setting.ICompare ("versionParanoid") == 0))
           value = !value;
         vision_app->SetBool (setting.String(), value);
       }
       break;
     default:
       BView::MessageReceived(msg);
       break;
  }
}
