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
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Brian Luft
 *								 Todd Lair
 *								 Rene Gollent
 */

#include <Bitmap.h>
#include <MessageRunner.h>
#include <Screen.h>
#include <TextView.h>
#include <TranslationUtils.h>

#include "AboutWindow.h"
#include "Vision.h"
#include "ClickView.h"

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
	BBitmap *bmp (NULL);

	fBackground = new BView (
										 bounds,
										 "background",
										 B_FOLLOW_ALL_SIDES,
										 B_WILL_DRAW);
	fBackground->SetViewColor (255, 255, 255);
	AddChild (fBackground);


	if ((bmp = BTranslationUtils::GetBitmap ('bits', "vision")) != 0)
	{
		//BRect logo_bounds (bmp->Bounds());

		fLogo = new ClickView (
										bmp->Bounds().OffsetByCopy (16, 16),
										"image",
										B_FOLLOW_LEFT | B_FOLLOW_TOP,
										B_WILL_DRAW,
										"http://vision.sourceforge.net");
		fBackground->AddChild (fLogo);
		fLogo->SetViewBitmap (bmp);
		delete bmp;

		bounds.Set (
			0.0,
			fLogo->Frame().bottom + 12, 
			Bounds().right,
			Bounds().bottom);
	}

	fCredits = new BTextView (
									bounds,
									"credits",
									bounds.OffsetToCopy (B_ORIGIN).InsetByCopy (20, 0),
									B_FOLLOW_LEFT | B_FOLLOW_TOP,
									B_WILL_DRAW); 

	fCredits->MakeSelectable (false);
	fCredits->MakeEditable (false);
	fCredits->SetStylable (true);
	fCredits->SetAlignment (B_ALIGN_CENTER);
	fBackground->AddChild (fCredits);


	fCreditsText =
		"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"
		"Unit A\n[Vision]\n"
		"{A-Z}\n"
		"Alan Ellis (voidref)\n"
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
		
		"send all complaints and nipple pictures to kaye\n\n\n"

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

	rgb_color myBlack = {0,0,0,255};
	fTextRun.count					= 1;
	fTextRun.runs[0].offset = 0;
	fTextRun.runs[0].font	 = *be_fixed_font;
	fTextRun.runs[0].color	= myBlack;

	fCredits->Insert (fCreditsText, &fTextRun);

	// Center window
	BRect frame (BScreen().Frame());
	MoveTo (
		frame.Width()/2 - Frame().Width()/2,
		frame.Height()/2 - Frame().Height()/2);
	fScrollRunner = new BMessageRunner (BMessenger (this), new BMessage (M_ABOUT_SCROLL),
		 50000);
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
	delete fScrollRunner;
	vision_app->PostMessage (M_ABOUT_CLOSE);
	
	return true;
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

void
AboutWindow::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_ABOUT_SCROLL:
		{
#if B_BEOS_VERSION_DANO 
			fCredits->SetDoubleBuffering(0xf);
#endif

			BPoint point (fCredits->PointAt (fCredits->TextLength() - 1));
			fCredits->ScrollBy (0, 1);

			if (fCredits->Bounds().bottom > point.y + Bounds().Height())
				fCredits->ScrollTo (0, 0);
		}
		break;
		
		default:
			BWindow::MessageReceived (msg);
			break;
	}
}
