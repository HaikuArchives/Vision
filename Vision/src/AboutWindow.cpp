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
 *                 Brian Luft
 *                 Todd Lair
 *                 Rene Gollent
 */ 

#include <Bitmap.h>
#include <Screen.h>
#include <TextView.h>
#include <TranslationUtils.h>

#include "AboutWindow.h"
#include "Vision.h"
#include "ClickView.h"

class ClickView;

AboutWindow::AboutWindow (void)
  : BWindow (
      BRect (0.0, 0.0, 317.0, 225.0),
      "About Vision",
      B_TITLED_WINDOW,
      B_WILL_DRAW | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  BRect bounds (Bounds());
  BBitmap *bmp;

  // what's a program without easter eggs?
  //AddShortcut('O', B_COMMAND_KEY, new BMessage(M_ABOUT_ORGY));
  //AddShortcut('J', B_COMMAND_KEY, new BMessage(M_ABOUT_BUDDYJ));

  background = new BView (
                     bounds,
                     "background",
                     B_FOLLOW_ALL_SIDES,
                     B_WILL_DRAW);
  background->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (background);


  if ((bmp = BTranslationUtils::GetBitmap ('bits', "bits")) != 0)
  {
    BRect bmp_bounds (bmp->Bounds());

    ResizeTo (
      bmp_bounds.Width() + 50,
      bmp_bounds.Height() + 250);

    graphic = new ClickView (
                    bmp->Bounds().OffsetByCopy (25, 25),
                    "image",
                    B_FOLLOW_LEFT | B_FOLLOW_TOP,
                    B_WILL_DRAW,
                    "http://vision.sourceforge.net");
    background->AddChild (graphic);

    graphic->SetViewBitmap (bmp);
    EasterEggOn = false;
    delete bmp;

    bounds.Set (
      0.0,
      graphic->Frame().bottom + 1, 
      Bounds().right,
      Bounds().bottom);
  }

  credits = new BTextView (
                  bounds,
                  "credits",
                  bounds.OffsetToCopy (B_ORIGIN).InsetByCopy (20, 0),
                  B_FOLLOW_LEFT | B_FOLLOW_TOP,
                  B_WILL_DRAW); 

  credits->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  credits->MakeSelectable (false);
  credits->MakeEditable (false);
  credits->SetStylable (true);
  credits->SetAlignment (B_ALIGN_CENTER);
  background->AddChild (credits);

  const char *creditstext =
    "\n\n\n\n\n\n\n\n\n"

    "Unit A\n"
    "[Vision]\n\n"
    "Rene Gollent (AnEvilYak)\n"
    "Wade Majors (kurros)\n\n\n\n"

    "Unit B\n"
    "[Bowser]\n\n"
    "Andrew Bazan (Hiisi)\n"
    "Rene Gollent (AnEvilYak)\n"
    "Todd Lair (tlair)\n"
    "Brian Luft (Electroly)\n"
    "Wade Majors (kurros)\n"
    "Jamie Wilkinson (project)\n\n\n\n"
    
    "Unit C\n"
    "[Support Teams]\n"
    "Assistant to Wade Majors: Patches\n"
    "Music Supervisor: Baron Arnold\n"
    "Assistant to Baron Arnold: Ficus Kirkpatrick\n"
    "Stunt Coordinator: Gilligan\n"
    "Nude Scenes: Natalie Portman\n"
    "Counselors: regurg and helix\n\n\n"
    "No animals were injured during the production of this IRC client\n\n\n"
    "Soundtrack available on Catastrophe Records\n\n\n"

    "Thanks\n\n"
    "Special thanks go out to:\n"
    "Olathe\n"
    "Terminus\n"
    "Bob Maple\n"
    "Gord McLeod\n"
    "Ted Stodgell\n"
    "Seth Flaxman\n"
    "David Aquilina\n"
    "Kurt von Finck\n"
    "Kristine Gouveia\n"
    "Jean-Baptiste Quéru\n"
    "Be, Inc., Menlo Park, CA\n"
    "Pizza Hut, Winter Haven, FL (now give me that free pizza Mike)\n\n\n"
    "http://vision.sourceforge.net\n\n\n\n\n\n"
    
    "\"A human being should be able to change "
    "a diaper, plan an invasion, butcher a "
    "hog, conn a ship, design a building, "
    "write a sonnet, balance accounts, build "
    "a wall, set a bone, comfort the dying, "
    "take orders, give orders, cooperate, act "
    "alone, solve equations, analyze a new "
    "problem, pitch manure, program a com"
    "puter, cook a tasty meal, fight effi"
    "ciently, die gallantly. Specialization "
    "is for insects.\" -- Robert A. Heinlein\n\n\n\n\n\n\n\n"
    "So, like, two guys walk into a bar.\n\n\n\n\nAnd they fell down.\n\n\n\n\nHAW!"
    "\n\n\n";

  rgb_color black = {0, 0, 0, 255};
  BFont font (*be_plain_font);
  text_run_array run;

  font.SetSize (font.Size() * 2.0);
  
  run.count          = 1;
  run.runs[0].offset = 0;
  run.runs[0].font   = font;
  run.runs[0].color  = black;

  credits->Insert ("\n\n\n\n\n\n\n\n\nVision ", &run);
  credits->Insert (vision_app->VisionVersion().String(), &run);
  credits->Insert ("\n", &run);
  run.runs[0].font = *be_plain_font;
  credits->Insert (creditstext, &run);
  
  
  // Center that bad boy
  BRect frame (BScreen().Frame());
  MoveTo (
    frame.Width()/2 - Frame().Width()/2,
    frame.Height()/2 - Frame().Height()/2); 
 

  SetPulseRate (74000);
}


AboutWindow::~AboutWindow (void)
{
  background->RemoveSelf();
  delete graphic;
  graphic = NULL;
  delete credits;
  credits = NULL;
  delete background;
  background = NULL;
}


bool
AboutWindow::QuitRequested (void)
{
  be_app_messenger.SendMessage (M_ABOUT_CLOSE);
  return true;
}


void
AboutWindow::DispatchMessage (BMessage *msg, BHandler *handler)
{
  if (msg->what == B_PULSE)
    Pulse();

  BWindow::DispatchMessage (msg, handler);
}


void
AboutWindow::Pulse (void)
{
  BPoint point (credits->PointAt (credits->TextLength() - 1));
  credits->ScrollBy (0, 1);

  if (credits->Bounds().bottom > point.y + Bounds().Height())
    credits->ScrollTo (0, 0);
}


void
AboutWindow::AboutImage (const char *eggName, bool egg)
{
  BBitmap *bmp;

  if ((bmp = BTranslationUtils::GetBitmap ('bits', eggName)) != 0)
  {
    BRect bmp_bounds (bmp->Bounds());
    graphic->ResizeTo (bmp_bounds.Width(), bmp_bounds.Height());
    graphic->SetViewBitmap (bmp);
    credits->MoveTo (0.0, graphic->Frame().bottom + 1);
    graphic->Invalidate();
    EasterEggOn = egg;
    delete bmp;
  }
}
