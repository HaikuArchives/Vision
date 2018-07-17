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
 *                 Todd Lair
 */

#include "NumericFilter.h"
#include "PrefDCC.h"
#include "Vision.h"

#include <Box.h>
#include <CheckBox.h>
#include <LayoutBuilder.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Path.h>
#include <TextControl.h>

#include <ctype.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefDCC"

DCCPrefsView::DCCPrefsView()
	: BView("DCC prefs", 0)
{
	AdoptSystemColors();
	BString text(B_TRANSLATE("DCC block size:"));
	BMenu* menu(new BMenu(text.String()));
	menu->AddItem(new BMenuItem("1024", new BMessage(M_BLOCK_SIZE_CHANGED)));
	menu->AddItem(new BMenuItem("2048", new BMessage(M_BLOCK_SIZE_CHANGED)));
	menu->AddItem(new BMenuItem("4096", new BMessage(M_BLOCK_SIZE_CHANGED)));
	menu->AddItem(new BMenuItem("8192", new BMessage(M_BLOCK_SIZE_CHANGED)));
	fBlockSize = new BMenuField(NULL, text.String(), menu);
	fAutoAccept = new BCheckBox(NULL, B_TRANSLATE("Automatically accept incoming sends"),
								new BMessage(M_AUTO_ACCEPT_CHANGED));
	fPrivateCheck = new BCheckBox(NULL, B_TRANSLATE("Automatically check for NAT IP"),
								  new BMessage(M_DCC_PRIVATE_CHANGED));
	fDefDir = new BTextControl(NULL, B_TRANSLATE("Default path:"), "",
							   new BMessage(M_DEFAULT_PATH_CHANGED));

	fDccPortMin = new BTextControl(NULL, B_TRANSLATE("Min:"), "",
								   new BMessage(M_DCC_MIN_PORT_CHANGED));
	fDccPortMin->TextView()->AddFilter(new NumericFilter());

	fDccPortMax = new BTextControl(NULL, B_TRANSLATE("Max:"), "",
								   new BMessage(M_DCC_MAX_PORT_CHANGED));
	fDccPortMax->TextView()->AddFilter(new NumericFilter());

	fBox = new BBox("box");
	fContainerBox = new BView("containerBox", 0);
	fBox->AddChild(fContainerBox);
	fBox->SetLabel(B_TRANSLATE("DCC port range"));

	BLayoutBuilder::Grid<>(fContainerBox, 0.0, 0.0)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fDccPortMin->CreateLabelLayoutItem(), 0, 0)
			.Add(fDccPortMin->CreateTextViewLayoutItem(), 1, 0)
			.Add(fDccPortMax->CreateLabelLayoutItem(), 0, 1)
			.Add(fDccPortMax->CreateTextViewLayoutItem(), 1, 1)
		.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fDefDir)
			.Add(fPrivateCheck)
			.Add(fAutoAccept)
			.Add(fBlockSize)
			.Add(fBox)
		.End();
}

DCCPrefsView::~DCCPrefsView()
{
}

void DCCPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	fBlockSize->Menu()->SetTargetForItems(this);
	fAutoAccept->SetTarget(this);
	fDefDir->SetTarget(this);
	fPrivateCheck->SetTarget(this);
	fBlockSize->Menu()->SetLabelFromMarked(true);

	const char* defPath(vision_app->GetString("dccDefPath"));
	fDefDir->SetText(defPath);

	if (vision_app->GetBool("dccAutoAccept"))
		fAutoAccept->SetValue(B_CONTROL_ON);
	else
		fDefDir->SetEnabled(false);

	if (vision_app->GetBool("dccPrivateCheck")) fPrivateCheck->SetValue(B_CONTROL_ON);

	fDccPortMin->SetTarget(this);
	fDccPortMax->SetTarget(this);
	fDccPortMin->SetText(vision_app->GetString("dccMinPort"));
	fDccPortMax->SetText(vision_app->GetString("dccMaxPort"));

	const char* dccBlock(vision_app->GetString("dccBlockSize"));

	BMenuItem* item(fBlockSize->Menu()->FindItem(dccBlock));
	if (item) dynamic_cast<BInvoker*>(item)->Invoke();
}

void DCCPrefsView::AllAttached()
{
	BView::AllAttached();
}

void DCCPrefsView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
}

void DCCPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_BLOCK_SIZE_CHANGED: {
		BMenuItem* it(NULL);
		msg->FindPointer("source", reinterpret_cast<void**>(&it));
		if (it) vision_app->SetString("dccBlockSize", 0, it->Label());
	} break;

	case M_DEFAULT_PATH_CHANGED: {
		const char* path(fDefDir->Text());
		BPath testPath(path, NULL, true);
		if (testPath.InitCheck() == B_OK) vision_app->SetString("dccDefPath", 0, path);
	} break;

	case M_AUTO_ACCEPT_CHANGED: {
		int32 val(fAutoAccept->Value());
		fDefDir->SetEnabled(val == B_CONTROL_ON);
		vision_app->SetBool("dccAutoAccept", val);
	} break;

	case M_DCC_MIN_PORT_CHANGED: {
		const char* portMin(fDccPortMin->Text());
		if (portMin != NULL) vision_app->SetString("dccMinPort", 0, portMin);
	} break;
	case M_DCC_MAX_PORT_CHANGED: {
		const char* portMax(fDccPortMax->Text());
		if (portMax != NULL) vision_app->SetString("dccMaxPort", 0, portMax);
	} break;

	case M_DCC_PRIVATE_CHANGED: {
		vision_app->SetBool("dccPrivateCheck", fPrivateCheck->Value() == B_CONTROL_ON);
	} break;

	default:
		BView::MessageReceived(msg);
		break;
	}
}
