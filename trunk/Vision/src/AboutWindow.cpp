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

#ifdef GNOME_BUILD
#  include "gnome/Bitmap.h"
#  include "gnome/Screen.h"
#  include "gnome/TextView.h"
#  include "gnome/TranslationUtils.h"
#elif BEOS_BUILD
#  include <Bitmap.h>
#  include <Screen.h>
#  include <TextView.h>
#  include <TranslationUtils.h>
#endif

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
  /*
   * Function purpose: Construct
   */

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
  rgb_color myBlack = {0, 0, 0, 255};
  background->SetViewColor (myBlack);
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

  credits->SetViewColor (myBlack);
  credits->MakeSelectable (false);
  credits->MakeEditable (false);
  credits->SetStylable (true);
  credits->SetAlignment (B_ALIGN_CENTER);
  background->AddChild (credits);

  const char *unita =
    "{A-Z}\n"
    "Rene Gollent (AnEvilYak)\n"
    "Wade Majors (kurros)\n\n\n\n"
  ;
  
  const char *unitb =
    "{A-Z}\n"
    "Andrew Bazan (Hiisi)\n"
    "Rene Gollent (AnEvilYak)\n"
    "Todd Lair (tlair)\n"
    "Brian Luft (Electroly)\n"
    "Wade Majors (kurros)\n"
    "Jamie Wilkinson (project)\n\n\n\n"
  ;
  
  const char *unitc =
    "Assistant to Wade Majors: Patches\n"
    "Music Supervisor: Baron Arnold\n"
    "Assistant to Baron Arnold: Ficus Kirkpatrick\n"
    "Stunt Coordinator: Gilligan\n"
    "Nude Scenes: Natalie Portman\n"
    "Counselors: regurg and helix\n\n\n"
    "No animals were injured during the production of this IRC client\n\n\n"
    "Soundtrack available on Catastrophe Records\n\n\n\n"
  ;
  
  const char *contrib =
    "{A-Z}\n"
    "Seth Flaxman (Flax)\n"
    "Joshua Jensen\n"
    "Gord McLeod (G_McLeod)\n"
    "John Robinson ([geo])\n"
    "Bjorn Oksholen (GuinessM)\n"
    "Jean-Baptiste M. Quéru (jbq)\n"
    "\n\n\n"
  ;

  
  const char *thanks =
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
    "^^^ Click The Logo ^^^\nhttp://vision.sourceforge.net\n^^^ Click The Logo ^^^\n\n\n\n\n\n"
  ;
  
  const char *quote =
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

  rgb_color myBlue   = {56, 172, 236, 255};
  rgb_color myWhite  = {255, 255, 255, 255};
  rgb_color myHeader = {119, 119, 255, 255};
  rgb_color myQuote  = {153, 204, 153, 255};
  BFont font (*be_plain_font);
  text_run_array run;

  font.SetSize (24);
  font.SetFace (B_BOLD_FACE);
  
  run.count          = 1;
  run.runs[0].offset = 0;
  run.runs[0].font   = font;
  run.runs[0].color  = myBlue;

  credits->Insert ("\n\n\n\n\n\n\nVision\n", &run);
  credits->Insert (vision_app->VisionVersion (VERSION_VERSION).String(), &run);
  credits->Insert ("\n\n\n\n", &run);
  
  font = *be_plain_font;
  font.SetSize (11);
  
  run.runs[0].font = font;
  run.runs[0].color = myWhite;
  
  text_run_array runhead;
  runhead = run;
  
  font = *be_plain_font;
  font.SetSize (16);
  font.SetFace (B_BOLD_FACE);  
  runhead.runs[0].color = myHeader;
  runhead.runs[0].font  = font;
  
  credits->Insert ("A Vision Team Production\n\n© 1999, 2000, 2001\n\n\n\n\n\n", &run);
  
  credits->Insert ("Unit A\n[Vision]\n", &runhead);      
  credits->Insert (unita, &run);
  credits->Insert ("\n\n\n\nUnit B\n[Bowser]\n", &runhead);      
  credits->Insert (unitb, &run);
  credits->Insert ("\n\n\n\nBrought To You In Part By Contributions From\n", &runhead);
  credits->Insert (contrib, &run); 
  credits->Insert ("\n\n\n\nUnit C\n[Support Crew]\n", &runhead);
  credits->Insert (unitc, &run);        
  credits->Insert ("\n\n\n\nSpecial Thanks\n\n", &runhead);      
  credits->Insert (thanks, &run);
  
  run.runs[0].color = myQuote;
  credits->Insert (quote, &run);
  
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
AboutWindow::AboutImage (const char *eggName, bool egg)
{
  /*
   * Function purpose: Read image resource bits::{eggName} and display
   */
   
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
