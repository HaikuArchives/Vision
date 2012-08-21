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
 *								 Todd Lair
 */

#include "NumericFilter.h"
#include "PrefDCC.h"
#include "Vision.h"
#include "VTextControl.h"

#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Path.h>

#include <ctype.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DCCPrefs"
 
DCCPrefsView::DCCPrefsView (BRect frame)
	: BView (frame, "DCC prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BMenu *menu (new BMenu (B_TRANSLATE("DCC Block Size")));
	menu->AddItem (new BMenuItem ("1024", new BMessage (M_BLOCK_SIZE_CHANGED)));
	menu->AddItem (new BMenuItem ("2048", new BMessage (M_BLOCK_SIZE_CHANGED)));
	menu->AddItem (new BMenuItem ("4096", new BMessage (M_BLOCK_SIZE_CHANGED)));
	menu->AddItem (new BMenuItem ("8192", new BMessage (M_BLOCK_SIZE_CHANGED)));
	BString itemText = B_TRANSLATE("DCC Block Size");
	itemText += ": ";
	fBlockSize = new BMenuField (BRect (0,0,0,0), NULL, itemText.String(), menu);
	fAutoAccept = new BCheckBox (BRect (0,0,0,0), NULL, B_TRANSLATE("Automatically accept incoming sends"),
		new BMessage (M_AUTO_ACCEPT_CHANGED));
	fPrivateCheck = new BCheckBox (BRect (0,0,0,0), NULL, B_TRANSLATE("Automatically check for NAT IP"),
		new BMessage (M_DCC_PRIVATE_CHANGED));
	itemText = B_TRANSLATE("Default path");
	itemText += ": ";
	fDefDir = new VTextControl (BRect (0,0,0,0), NULL, itemText.String(), "", new BMessage (M_DEFAULT_PATH_CHANGED));
	fBox = new BBox (BRect (0,0,0,0), NULL);
	fBox->SetLabel (B_TRANSLATE("DCC Port Range"));
	AddChild (fDefDir);
	AddChild (fPrivateCheck);
	AddChild (fAutoAccept);
	AddChild (fBlockSize);
	AddChild (fBox);
	fDefDir->SetDivider (fDefDir->StringWidth (itemText.String()) + 5);
	itemText = B_TRANSLATE("Min");
	itemText += ": ";
	fDccPortMin = new VTextControl (BRect (0,0,0,0), NULL, itemText.String(), "",
		new BMessage (M_DCC_MIN_PORT_CHANGED));
	fDccPortMin->TextView()->AddFilter (new NumericFilter());
	fDccPortMin->SetDivider (fDccPortMin->StringWidth (itemText.String()) + 5);
	fBox->AddChild (fDccPortMin);
	itemText = B_TRANSLATE("Max");
	itemText += ": ";
	fDccPortMax = new VTextControl (BRect (0,0,0,0), NULL, itemText.String(), "",
		new BMessage (M_DCC_MAX_PORT_CHANGED));
	fDccPortMax->SetDivider (fDccPortMax->StringWidth (itemText) + 5);
	fDccPortMax->TextView()->AddFilter (new NumericFilter());
	fBox->AddChild (fDccPortMax);
}

DCCPrefsView::~DCCPrefsView (void)
{
}

void
DCCPrefsView::AttachedToWindow (void)
{
	BView::AttachedToWindow ();
	fBlockSize->Menu()->SetTargetForItems (this);
	fAutoAccept->SetTarget (this);
	fDefDir->SetTarget (this);
	fDefDir->SetDivider(fDefDir->StringWidth(fDefDir->Label()) + 5.0);
	fDefDir->ResizeToPreferred ();
	fDefDir->ResizeTo (Bounds().Width() - 15, fDefDir->Bounds().Height());
	fDefDir->MoveTo (10, 10);
	fAutoAccept->ResizeToPreferred ();
	fAutoAccept->MoveTo (fDefDir->Frame().left, fDefDir->Frame().bottom + 5);
	fPrivateCheck->SetTarget (this);
	fPrivateCheck->ResizeToPreferred ();
	fPrivateCheck->MoveTo (fAutoAccept->Frame().left, fAutoAccept->Frame().bottom + 5);
	fBlockSize->ResizeToPreferred ();
	fBlockSize->ResizeTo (Bounds().Width() - 15, fBlockSize->Bounds().Height());
	fBlockSize->SetDivider (fBlockSize->StringWidth (fBlockSize->Label()) + 5);
	fBlockSize->MoveTo (fPrivateCheck->Frame().left, fPrivateCheck->Frame().bottom + 5);
	fBlockSize->Menu()->SetLabelFromMarked (true);
	
	const char *defPath (vision_app->GetString ("dccDefPath"));
	fDefDir->SetText (defPath);
	
	if (vision_app->GetBool ("dccAutoAccept"))
		fAutoAccept->SetValue (B_CONTROL_ON);
	else
		fDefDir->SetEnabled (false);
	
	if (vision_app->GetBool ("dccPrivateCheck"))
		fPrivateCheck->SetValue (B_CONTROL_ON);
		
	fDccPortMin->SetDivider(fDccPortMin->StringWidth(fDccPortMin->Label()) + 5.0);
	fDccPortMin->ResizeToPreferred();
	fDccPortMax->SetDivider(fDccPortMax->StringWidth(fDccPortMax->Label()) + 5.0);
	fDccPortMax->ResizeToPreferred();
	fDccPortMin->SetTarget (this);
	fDccPortMax->SetTarget (this);
	
	fBox->ResizeTo (Bounds().Width() - 20, fDccPortMin->Bounds().Height()+30);
	fBox->MoveTo (fBlockSize->Frame().left, fBlockSize->Frame().bottom + 25);
	fDccPortMin->ResizeTo ((fBox->Bounds().Width() / 2.0) - 15, fDccPortMin->Bounds().Height());
	fDccPortMax->ResizeTo (fDccPortMin->Bounds().Width(), fDccPortMin->Bounds().Height());

	fDccPortMin->MoveTo (5,20);
	fDccPortMax->MoveTo (fDccPortMin->Frame().right + 5, fDccPortMin->Frame().top);
		
	fDccPortMin->SetText (vision_app->GetString ("dccMinPort"));
	fDccPortMax->SetText (vision_app->GetString ("dccMaxPort"));
	
	const char *dccBlock (vision_app->GetString ("dccBlockSize"));
	
	BMenuItem *item (fBlockSize->Menu()->FindItem (dccBlock));
	if (item)
		dynamic_cast<BInvoker *>(item)->Invoke();
}

void
DCCPrefsView::AllAttached (void)
{
	BView::AllAttached ();
}

void
DCCPrefsView::FrameResized (float width, float height)
{
	BView::FrameResized (width, height);
}

void
DCCPrefsView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_BLOCK_SIZE_CHANGED:
			{
				BMenuItem *it (NULL);
				msg->FindPointer ("source", reinterpret_cast<void **>(&it));
				if (it)
					vision_app->SetString ("dccBlockSize", 0, it->Label());
			}
			break;
			
		case M_DEFAULT_PATH_CHANGED:
			{
				const char *path (fDefDir->Text());
				BPath testPath (path, NULL, true);
				if (testPath.InitCheck() == B_OK)
					vision_app->SetString ("dccDefPath", 0, path);
			}
			break;
			
		case M_AUTO_ACCEPT_CHANGED:
			{
				int32 val (fAutoAccept->Value());
				fDefDir->SetEnabled (val == B_CONTROL_ON);
				vision_app->SetBool ("dccAutoAccept", val);
			}
			break;
		
		case M_DCC_MIN_PORT_CHANGED:
			{
				const char *portMin (fDccPortMin->Text());
				if (portMin != NULL)
					vision_app->SetString ("dccMinPort", 0, portMin);
			}
			break;
		case M_DCC_MAX_PORT_CHANGED:
			{
				const char *portMax (fDccPortMax->Text());
				if (portMax != NULL)
					vision_app->SetString ("dccMaxPort", 0, portMax);
			}
			break;
		
		case M_DCC_PRIVATE_CHANGED:
			{
				vision_app->SetBool("dccPrivateCheck", fPrivateCheck->Value() == B_CONTROL_ON);
			}
			break;
			
		default:
			BView::MessageReceived (msg);
			break;
	}
}
