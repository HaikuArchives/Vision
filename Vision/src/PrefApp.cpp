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
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <UTF8.h>

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

  checkboxRect.top += fSpamMode->Bounds().Height() * 1.2;
  msg.ReplaceString ("setting", "queryOnMsg");
  fQueryMsg = new BCheckBox (checkboxRect, "queryOnMsg",
    S_PREFAPP_QUERY_MSG,
    new BMessage (msg));
  fQueryMsg->SetValue ((vision_app->GetBool ("queryOnMsg")) ? B_CONTROL_ON : B_CONTROL_OFF);
  fQueryMsg->MoveBy(be_plain_font->StringWidth("S"), 0);
  fQueryMsg->ResizeToPreferred();
  trackingBoundsRect = fSpamMode->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  AddChild (fQueryMsg);

  checkboxRect.top += fQueryMsg->Bounds().Height() * 1.2;
  
  BMenu *encMenu(CreateEncodingMenu());
  
  fEncodings = new BMenuField(checkboxRect, "encoding", "Encoding: ", encMenu);

  AddChild (fEncodings);
  fEncodings->Menu()->SetLabelFromMarked(true);

  trackingBoundsRect = fEncodings->Bounds();
  maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
  maxHeight += trackingBoundsRect.Height() * 1.5; 
  
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
  fStripColors->SetTarget (this);
  fSpamMode->SetTarget (this);
  fQueryMsg->SetTarget (this);
  fEncodings->SetDivider(StringWidth("Encoding: ") + 5);
  fEncodings->ResizeToPreferred();
  fEncodings->Menu()->SetTargetForItems (this);
  fEncodings->MoveBy(5.0, 0.0);
  SetEncodingItem(vision_app->GetInt32("encoding"));
  BView::AllAttached();
}

BMenu *
AppWindowPrefsView::CreateEncodingMenu(void)
{
  BMessage msg(M_APPWINDOWPREFS_ENCODING_CHANGED);
  BMenu *encMenu (new BMenu("Encodings"));
  msg.AddInt32("encoding", B_ISO1_CONVERSION);
  encMenu->AddItem (fEnc1 = new BMenuItem("Western (ISO 8859-1)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_ISO2_CONVERSION);
  encMenu->AddItem (fEnc2 = new BMenuItem("Central European (ISO 8859-2)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_ISO5_CONVERSION);
  encMenu->AddItem (fEnc3 = new BMenuItem("Cyrillic (ISO 8859-5)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_KOI8R_CONVERSION);
  encMenu->AddItem (fEnc4 = new BMenuItem("Cyrillic (KOI8-R)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_MS_DOS_866_CONVERSION);
  encMenu->AddItem (fEnc5 = new BMenuItem("Cyrillic (MS-DOS 866)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_MS_WINDOWS_1251_CONVERSION);
  encMenu->AddItem (fEnc6 = new BMenuItem("Cyrillic (Windows 1251)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_ISO7_CONVERSION);
  encMenu->AddItem (fEnc7 = new BMenuItem("Greek (ISO 8859-7)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_SJIS_CONVERSION);
  encMenu->AddItem (fEnc8 = new BMenuItem("Japanese (Shift-JIS)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_EUC_CONVERSION);
  encMenu->AddItem (fEnc9 = new BMenuItem("Japanese (EUC)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_EUC_KR_CONVERSION);
  encMenu->AddItem (fEnc10 = new BMenuItem("Korean (EUC)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_UNICODE_CONVERSION);
  encMenu->AddItem (fEnc11 = new BMenuItem("Unicode", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_MAC_ROMAN_CONVERSION);
  encMenu->AddItem (fEnc12 = new BMenuItem("Western (Mac Roman)", new BMessage(msg)));
  msg.ReplaceInt32("encoding", B_MS_WINDOWS_CONVERSION);
  encMenu->AddItem (fEnc13 = new BMenuItem("Western (Windows)", new BMessage(msg)));
  return encMenu;
}

void
AppWindowPrefsView::SetEncodingItem(int32 encoding)
{
  switch (encoding)
  {
    case B_ISO1_CONVERSION:
      fEnc1->SetMarked (true);
      break;
       
    case B_ISO2_CONVERSION:
      fEnc2->SetMarked (true);
      break;
       
    case B_ISO5_CONVERSION:
      fEnc3->SetMarked (true);
      break;
       
    case B_KOI8R_CONVERSION:
      fEnc4->SetMarked (true);
      break;
       
    case B_MS_DOS_866_CONVERSION:
      fEnc5->SetMarked (true);
      break;
       
    case B_MS_WINDOWS_1251_CONVERSION:
      fEnc6->SetMarked (true);
      break;
       
    case B_ISO7_CONVERSION:
      fEnc7->SetMarked (true);
      break;
       
    case B_SJIS_CONVERSION:
      fEnc8->SetMarked (true);
      break;
       
    case B_EUC_CONVERSION:
      fEnc9->SetMarked (true);
      break;
       
    case B_EUC_KR_CONVERSION:
      fEnc10->SetMarked (true);
      break;
       
    case B_UNICODE_CONVERSION:
      fEnc11->SetMarked (true);
      break;
       
    case B_MAC_ROMAN_CONVERSION:
      fEnc12->SetMarked (true);
      break;
       
    case B_MS_WINDOWS_CONVERSION:
      fEnc13->SetMarked (true);
      break;
       
    default:
      break;
  }
}

void
AppWindowPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_APPWINDOWPREFS_ENCODING_CHANGED:
      {
        BMenuItem *source (NULL);
        msg->FindPointer ("source", reinterpret_cast<void **>(&source));
        source->SetMarked(true);
        int32 encoding (msg->FindInt32("encoding"));
        SetEncodingItem(encoding);
        vision_app->SetInt32("encoding", encoding);
        
      }
      break;
      
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
