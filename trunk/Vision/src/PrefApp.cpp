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
  versionParanoid = new BCheckBox (checkboxRect, "version Paranoid",
    "Show OS information in version reply",
    new BMessage (msg));
  versionParanoid->SetValue ((!vision_app->GetBool ("versionParanoid")) ? B_CONTROL_ON : B_CONTROL_OFF);
  versionParanoid->MoveBy(be_plain_font->StringWidth("S"), 0);
  versionParanoid->ResizeToPreferred();
  trackingBoundsRect = versionParanoid->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height(); 
  AddChild (versionParanoid);
  
  checkboxRect.top += versionParanoid->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "catchAltW");
  catchAltW = new BCheckBox (checkboxRect, "catch AltW",
    "Cmd+W closes Vision",
    new BMessage (msg));
  catchAltW->SetValue ((!vision_app->GetBool ("catchAltW")) ? B_CONTROL_ON : B_CONTROL_OFF);
  catchAltW->MoveBy(be_plain_font->StringWidth("S"), 0);
  catchAltW->ResizeToPreferred();
  trackingBoundsRect = catchAltW->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.2; 
  AddChild (catchAltW);
  
  checkboxRect.top += catchAltW->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "timestamp");
  timeStamp = new BCheckBox (checkboxRect, "timeStamp",
    "Show timestamps in IRC window",
    new BMessage (msg));
  timeStamp->SetValue ((vision_app->GetBool ("timestamp")) ? B_CONTROL_ON : B_CONTROL_OFF);
  timeStamp->MoveBy(be_plain_font->StringWidth("S"), 0);
  timeStamp->ResizeToPreferred();
  trackingBoundsRect = timeStamp->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  AddChild (timeStamp);
  
  checkboxRect.top += timeStamp->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "log_enabled");
  logEnabled = new BCheckBox (checkboxRect, "logEnabled",
    "Enable logging",
    new BMessage (msg));
  logEnabled->SetValue ((vision_app->GetBool ("log_enabled")) ? B_CONTROL_ON : B_CONTROL_OFF);
  logEnabled->MoveBy(be_plain_font->StringWidth("S"), 0);
  logEnabled->ResizeToPreferred();
  trackingBoundsRect = logEnabled->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  AddChild (logEnabled);
  
  checkboxRect.top += logEnabled->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "log_filetimestamp");
  logFileTimestamp = new BCheckBox (checkboxRect, "logFileTimestamp",
    "Append timestamp to log filenames",
    new BMessage (msg));
  logFileTimestamp->SetValue (vision_app->GetBool (("log_filetimestamp")) ? B_CONTROL_ON : B_CONTROL_OFF);
  logFileTimestamp->MoveBy(be_plain_font->StringWidth("S"), 0);
  logFileTimestamp->ResizeToPreferred();
  trackingBoundsRect = logFileTimestamp->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxWidth *= 1.2;
  maxHeight += trackingBoundsRect.Height() * 1.5;
  AddChild (logFileTimestamp);
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
  versionParanoid->SetTarget (this);
  catchAltW->SetTarget (this);
  timeStamp->SetTarget (this);
  logEnabled->SetTarget (this);
  logFileTimestamp->SetTarget (this);

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
