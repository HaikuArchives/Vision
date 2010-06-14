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
 *								 Alan Ellis <alan@cgsoftware.org>
 */

//------------------------------------------------------------------------------
// IconMenu.h
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
//
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

#ifndef ICON_MENU_H
#define ICON_MENU_H

//------------------------------------------------------------------------------
// I N C L U D E S
//------------------------------------------------------------------------------

#include <interface/MenuItem.h>
#include <interface/Rect.h>

//------------------------------------------------------------------------------
// D E C L A R A T I O N S
//------------------------------------------------------------------------------

class BBitmap;

class TIconMenu : public BMenuItem
{
public:
		// Constructor
		// Description:	Creates a menu item with the provided icon as its
		//				label and menu as its content.
		// Requires:		Both icon and menu must be valid.
		// Result:		If icon is valid, then the icon it points to will
		//				be used as the label for the menu. If icon is NULL
		//				then the menu's name will be used instead. If icon
		//				is invalid, a crash occurs. If menu is invalid or
		//				NULL, a crash occurs.
		// Notes:			Client code is still responsible for deleting the
		//				data icon points to. IconMenu only copies the data,
		//				it does not take posession of it.
		TIconMenu(BBitmap* icon, BMenu* menu);

		// Constructor
		// Description:	Creates a menu item with the application's mini
		//				icon as its label and menu as its content.
		// Requires:		A valid BMenu pointer. The application should also
		//				have a mini icon.
		// Result:		If the application's mini icon is found then the
		//				menu will be created with the icon as the label.
		//				Otherwise, the name of menu will be used as the
		//				label instead. If menu is invalid or NULL, a
		//				crash will occur.
		TIconMenu(BMenu* menu);

		// Destructor
		virtual ~TIconMenu();

protected:
		// Overridden Hook Functions
		virtual void GetContentSize(float* width, float* height);
		virtual void DrawContent();

private:
		// Disabled Copy Constructor
		TIconMenu(const TIconMenu& iconMenu);

		// Disabled Assignment Operator
		TIconMenu& operator=(const TIconMenu& iconMenu);

		BRect	bounds;
		BBitmap* iconLabel;
};

//---------------------------------------------------------------- IconMenu.h --

#endif
