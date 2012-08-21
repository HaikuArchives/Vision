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
 * Contributor(s): Rene Gollent
 */

#include "PrefsWindow.h"
#include "PrefGeneral.h"
#include "Vision.h"

#include <Box.h>
#include <Catalog.h>
#include <ScrollView.h>
#include <View.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefWindow"

PrefsWindow::PrefsWindow(void)
	: BWindow (BRect (88.0, 108.0, 0.0, 0.0),
			B_TRANSLATE("Preferences"),
			B_TITLED_WINDOW,
			B_ASYNCHRONOUS_CONTROLS)
{
	GeneralPrefsView *generalView = new GeneralPrefsView(BRect(0.0, 0.0, 0.0, 0.0),
		 "view", B_FOLLOW_ALL_SIDES, B_WILL_DRAW);

	ResizeTo(generalView->Bounds().Width(), generalView->Bounds().Height());

	BBox *box = new BBox (Bounds().InsetByCopy(-1,-1), "box", B_FOLLOW_ALL_SIDES);
	
	AddChild(box);

	box->AddChild(generalView);
	generalView->MoveTo ((box->Bounds().Width() - generalView->Bounds().Width()) / 2,
		(box->Bounds().Height() - generalView->Bounds().Height()) / 2);

	BRect prefsRect (vision_app->GetRect ("GenPrefWinRect"));
	if (prefsRect.Width() != 0.0 && prefsRect.Height() != 0.0)
	{
		ResizeTo (prefsRect.Width(), prefsRect.Height());
		MoveTo (prefsRect.left, prefsRect.top);
	}
}

PrefsWindow::~PrefsWindow(void)
{
}

bool
PrefsWindow::QuitRequested(void)
{
	vision_app->SetRect ("GenPrefWinRect", Frame());
	be_app_messenger.SendMessage (M_PREFS_CLOSE);
	return true;	
}
