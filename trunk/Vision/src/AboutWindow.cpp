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
 * Contributor(s): Wade Majors <wade@ezri.org>
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


#define PULSE_RATE 100000

class ClickView;

AboutWindow::AboutWindow (void)
  : BWindow (
      BRect (0.0, 0.0, 420.0, 266.0),
      "About Vision",
      B_TITLED_WINDOW,
      B_WILL_DRAW | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  /*
   * Function purpose: Construct
   */

  
  BRect bounds (Bounds());
  BBitmap *bmp;

  background = new BView (
                     bounds,
                     "background",
                     B_FOLLOW_ALL_SIDES,
                     B_WILL_DRAW);
  background->SetViewColor (255, 255, 255);
  AddChild (background);


  if ((bmp = BTranslationUtils::GetBitmap ('bits', "vision")) != 0)
  {
    //BRect logo_bounds (bmp->Bounds());

    logo = new ClickView (
                    bmp->Bounds().OffsetByCopy (16, 16),
                    "image",
                    B_FOLLOW_LEFT | B_FOLLOW_TOP,
                    B_WILL_DRAW,
                    "http://vision.sourceforge.net");
    background->AddChild (logo);
    logo->SetViewBitmap (bmp);
    delete bmp;

    bounds.Set (
      0.0,
      logo->Frame().bottom + 12, 
      Bounds().right,
      Bounds().bottom);
  }

  credits = new BTextView (
                  bounds,
                  "credits",
                  bounds.OffsetToCopy (B_ORIGIN).InsetByCopy (20, 0),
                  B_FOLLOW_LEFT | B_FOLLOW_TOP,
                  B_WILL_DRAW); 

  //credits->SetViewColor (myBlack);
  credits->MakeSelectable (false);
  credits->MakeEditable (false);
  credits->SetStylable (true);
  credits->SetAlignment (B_ALIGN_CENTER);
  background->AddChild (credits);

  creditsText =
    "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
    "Unit A\n[Vision]\n"
    "{A-Z}\n"
    "Rene Gollent (AnEvilYak)\n"
    "Todd Lair (tlair)\n"
    "Wade Majors (kurros)\n\n\n\n"

    "\n\n\n\nUnit B\n[Bowser]\n"
    "{A-Z}\n"
    "Andrew Bazan (Hiisi)\n"
    "Rene Gollent (AnEvilYak)\n"
    "Todd Lair (tlair)\n"
    "Brian Luft (Electroly)\n"
    "Wade Majors (kurros)\n"
    "Jamie Wilkinson (project)\n\n\n\n"
  
    "\n\n\n\nBrought To You In Part By Contributions From\n"
    "{A-Z}\n"
    "Seth Flaxman (Flax)\n"
    "Joshua Jensen\n"
    "Gord McLeod (G_McLeod)\n"
    "John Robinson ([geo])\n"
    "Bjorn Oksholen (GuinnessM)\n"
    "Jean-Baptiste M. Quéru (jbq)\n"
    "\n\n\n"

    "\n\n\n\nUnit C\n[Support Crew]\n"
    "Assistant to Wade Majors: Patches\n"
    "Music Supervisor: Baron Arnold\n"
    "Assistant to Baron Arnold: Ficus Kirkpatrick\n"
    "Stunt Coordinator: Gilligan\n"
    "Nude Scenes: Natalie Portman\n"
    "Counselors: regurg and helix\n\n\n"
    "No animals were injured during the production of this IRC client\n\n\n"
    "Soundtrack available on Catastrophe Records\n\n\n\n"
    
    "\n\n\n\nSpecial Thanks\n\n"
    "Olathe\n"
    "Terminus\n"
    "Bob Maple\n"
    "Ted Stodgell\n"
    "Seth Flaxman\n"
    "David Aquilina\n"
    "Kurt von Finck\n"
    "Kristine Gouveia\n"
    "Be, Inc., Menlo Park, CA\n"
    "Pizza Hut, Winter Haven, FL (now give me that free pizza Mike)\n\n\n"

    "\n\n\n\n\n"
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
    "is for insects.\" -- Robert A. Heinlein"
  ;

  fixedFont = *be_fixed_font;
  rgb_color myBlack = {0,0,0,255};
  textRun.count          = 1;
  textRun.runs[0].offset = 0;
  textRun.runs[0].font   = fixedFont;
  textRun.runs[0].color  = myBlack;

  credits->Insert (creditsText, &textRun);

  // Center window
  BRect frame (BScreen().Frame());
  MoveTo (
    frame.Width()/2 - Frame().Width()/2,
    frame.Height()/2 - Frame().Height()/2); 
 

  SetPulseRate (PULSE_RATE);
}


AboutWindow::~AboutWindow (void)
{
  //
}


bool
AboutWindow::QuitRequested (void)
{
  /*
   * Function purpose: Tell vision_app about our death
   */
  
  vision_app->PostMessage (M_ABOUT_CLOSE);
  
  return true;
}


void
AboutWindow::DispatchMessage (BMessage *msg, BHandler *handler)
{
  /*
   * Function purpose: Call Pulse() on B_PULSE messages
   */
   
  if (msg->what == B_PULSE)
    Pulse();

  // pass the message on to the parent class' DispatchMessage()
  BWindow::DispatchMessage (msg, handler);
}


void
AboutWindow::Pulse (void)
{
  /*
   * Function purpose: Scroll the credits BTextView by 1 unit;
   *                   If we are at the bottom, scroll to the top
   */
   
  BPoint point (credits->PointAt (credits->TextLength() - 1));
  credits->ScrollBy (0, 1);

  if (credits->Bounds().bottom > point.y + Bounds().Height())
    credits->ScrollTo (0, 0);
}


void
AboutWindow::AboutImage (const char *, bool)
{
  /*
   * Function purpose: Read image resource bits::{eggName} and display
   */

/* TODO : get this working  
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
*/
}
