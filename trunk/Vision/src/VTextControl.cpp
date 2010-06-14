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
 */

// Class description:
// VTextControl is a derivative of BTextControl which adds a standard clipboard
// related context menu (eg: Every TextControl in Windows has a right click menu
// with Cut/Copy/Paste/Select All)
 
// it's intention is to be fully compliant and portable, so it can easily
// be dropped into other applications as well.

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Clipboard.h>

#include "VTextControl.h"

VTextControl::VTextControl (BRect frame, const char *name, const char *label,
														const char *text, BMessage *vtcmessage,
														uint32 resizingMode, uint32 flags, bool nomenu_)
	: BTextControl(
		frame,
		name,
		label,
		text,
		vtcmessage,
		resizingMode,
		flags),
	nomenu (nomenu_)
{
	// Usage is identical to BTextControl 
}

VTextControl::VTextControl (BMessage *data, bool nomenu_)
	: BTextControl(
		data),
	nomenu (nomenu_)
{
	// Usage is identical to BTextControl 
}

void
VTextControl::AllAttached (void)
{
	// This function adds the BMessageFilter which catches B_MOUSE_DOWNs
	
	TextView()->AddFilter (new VTextControlFilter (this));
}

void
VTextControl::BuildPopUp (void)
{
	// This function checks certain criteria (text is selected,
	// TextView is editable, etc) to determine which MenuItems
	// to enable and disable
	
	bool enablecopy (true),
			 enablepaste (false), // check clipboard contents first
			 enablecut (true),
			 enableselectall (true);
			 
	int32 selstart, selfinish;
	TextView()->GetSelection (&selstart, &selfinish);
	if (selstart == selfinish)
		enablecopy = false; // no selection
	
	if (!TextView()->IsEditable() || !enablecopy)
		enablecut = false; // no selection or not editable
	
	if (!TextView()->IsSelectable() || (TextView()->TextLength() == 0))
		enableselectall = false; // not selectable
	
	if (TextView()->IsEditable())
	{
		BClipboard clipboard("system");
		BMessage *clip ((BMessage *)NULL);
		if (clipboard.Lock()) {
			if ((clip = clipboard.Data()))
				if (clip->HasData ("text/plain", B_MIME_TYPE))
					enablepaste = true; // has text on clipboard
		}
		clipboard.Unlock();
	}
	
	myPopUp = new BPopUpMenu ("Context Menu", false, false); 

	BMenuItem *item; 
	item = new BMenuItem("Cut", new BMessage (B_CUT), 'X');
	item->SetEnabled (enablecut);
	myPopUp->AddItem (item);
	
	item = new BMenuItem("Copy", new BMessage (B_COPY), 'C');
	item->SetEnabled (enablecopy);
	myPopUp->AddItem (item);

	item = new BMenuItem("Paste", new BMessage (B_PASTE), 'V');
	item->SetEnabled (enablepaste);
	myPopUp->AddItem (item);
	
	myPopUp->AddSeparatorItem(); 
	
	item = new BMenuItem("Select All", new BMessage (B_SELECT_ALL), 'A');
	item->SetEnabled (enableselectall);
	myPopUp->AddItem (item);	
	
	myPopUp->SetFont (be_plain_font); 
	myPopUp->SetTargetForItems (TextView()); 

}

///////////////////////////
// Filter //

VTextControlFilter::VTextControlFilter (VTextControl *parentcontrol)
 : BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE),

	parent (parentcontrol)
{
}

filter_result
VTextControlFilter::Filter (BMessage *msg, BHandler **)
{
	filter_result result (B_DISPATCH_MESSAGE);
	switch (msg->what)
	{
		case B_MOUSE_DOWN:
		{
			bool handled (false);
			
			if (!parent->nomenu)
			{
				BPoint myPoint;
				uint32 mousebuttons;
				int32	keymodifiers (0);
				parent->Parent()->GetMouse (&myPoint, &mousebuttons);
			

				msg->FindInt32 ("modifiers", &keymodifiers);	
 
				if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
				&& (keymodifiers & B_SHIFT_KEY)	 == 0
				&& (keymodifiers & B_OPTION_KEY)	== 0
				&& (keymodifiers & B_COMMAND_KEY) == 0
				&& (keymodifiers & B_CONTROL_KEY) == 0)
				{
					parent->TextView()->MakeFocus(true);
					parent->BuildPopUp();
					parent->myPopUp->Go (
						parent->Parent()->ConvertToScreen (myPoint),
						true,
						false,
						true);
					handled = true;
				}
			}
			
			if (handled)
				result = B_SKIP_MESSAGE;
			
			break;	 
		}
	}

	return result;

}
