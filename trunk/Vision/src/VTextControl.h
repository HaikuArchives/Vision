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
 
#ifndef _VTEXTCONTROL_H_
#define _VTEXTCONTROL_H_

#	include <TextControl.h>
#	include <MessageFilter.h>

class BPopUpMenu;
class BMenu;

class VTextControl : public BTextControl
{
	public:
													 VTextControl (BRect,
																				 const char *,
																				 const char *,
																				 const char *,
																				 BMessage *,
																				 uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP,
																				 uint32 = B_WILL_DRAW | B_NAVIGABLE,
																				 bool = false);
													 VTextControl (BMessage *, bool = false);
		virtual void					 AllAttached (void);
		void									 BuildPopUp (void);

		BPopUpMenu						 *myPopUp;
		
		bool									 nomenu;
};

class VTextControlFilter : public BMessageFilter
{

	public:
																		VTextControlFilter (VTextControl *);
		virtual filter_result					 Filter (BMessage *, BHandler **);

	private:
		VTextControl										*parent;

};

#endif
