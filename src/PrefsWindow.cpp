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

#include "PrefsWindow.h"
#include "PrefGeneral.h"
#include "Vision.h"

#include <Box.h>
#include <GroupLayout.h>
#include <LayoutBuilder.h>
#include <OutlineListView.h>
#include <View.h>

#include <ScrollView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefsWindow"

PrefsWindow::PrefsWindow()
	: BWindow(BRect(88.0, 108.0, 0.0, 0.0), B_TRANSLATE("Preferences"), B_TITLED_WINDOW,
		  /* B_NOT_ZOOMABLE | B_NOT_RESIZABLE  | */ B_ASYNCHRONOUS_CONTROLS
			  | B_AUTO_UPDATE_SIZE_LIMITS)
{
	GeneralPrefsView* generalView = new GeneralPrefsView("view");
	/*
		static const float spacing = B_USE_WINDOW_INSETS; //be_control_look->DefaultLabelSpacing();

		BGroupLayout* topBox = BLayoutBuilder::Group<>(B_VERTICAL)
			.SetInsets(B_USE_WINDOW_INSETS, spacing / 4,
				B_USE_WINDOW_INSETS, B_USE_WINDOW_INSETS)
			.Add(generalView);

		BBox* box = new BBox("box");
		box->AddChild(topBox->View()); */

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(generalView);
	// clang-format on

	BRect prefsRect(vision_app->GetRect("GenPrefWinRect"));
	if (prefsRect.Width() != 0.0 && prefsRect.Height() != 0.0) {
		ResizeTo(prefsRect.Width(), prefsRect.Height());
		MoveTo(prefsRect.left, prefsRect.top);
	}
}

PrefsWindow::~PrefsWindow() {}

bool
PrefsWindow::QuitRequested()
{
	vision_app->SetRect("GenPrefWinRect", Frame());
	be_app_messenger.SendMessage(M_PREFS_CLOSE);
	return true;
}
