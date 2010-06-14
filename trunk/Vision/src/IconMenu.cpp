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
// IconMenu.cpp
//------------------------------------------------------------------------------
// A menu item implementation that displays an icon as its label.
//
// IconMenu implementation Copyright (C) 1998 Tyler Riti <fizzboy@mail.utexas.edu>
// Based on code Copyright (C) 1997 Jens Kilian
// This code is free to use in any way so long as the credits above remain intact.
// This code carries no warranties or guarantees of any kind. Use at your own risk.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// I N C L U D E S
//------------------------------------------------------------------------------
#include <storage/AppFileInfo.h>
#include <interface/Bitmap.h>
#include <app/Application.h>
#include <app/Roster.h>



#include "IconMenu.h"

//------------------------------------------------------------------------------
// I M P L E M E N T A T I O N
//------------------------------------------------------------------------------

#ifdef __HAIKU__
const color_space kIconColorSpace = B_RGBA32;
#else
const color_space kIconColorSpace = B_CMAP8;
#endif

TIconMenu::TIconMenu(BBitmap* icon, BMenu* menu) :
				BMenuItem(menu),
				bounds(),
				iconLabel(NULL)
{
		if (icon) {
				bounds = icon->Bounds();
				iconLabel = new BBitmap(bounds, kIconColorSpace);
				iconLabel->SetBits(icon->Bits(), icon->BitsLength(), 0, kIconColorSpace);
		}
}

TIconMenu::TIconMenu(BMenu* menu) :
				BMenuItem(menu),
				bounds(0.0, 0.0, 15.0, 15.0),
				iconLabel(NULL)
{
		app_info info;
		if (be_app->GetAppInfo(&info) == B_NO_ERROR) {
				BFile appFile(&(info.ref), O_RDONLY);
				BAppFileInfo appFileInfo(&appFile);

				iconLabel = new BBitmap(bounds, kIconColorSpace);

				if (appFileInfo.GetIcon(iconLabel, B_MINI_ICON) != B_NO_ERROR) {
						delete iconLabel;
						iconLabel = NULL;
				}
		}
}

TIconMenu::~TIconMenu()
{
		delete iconLabel;
		iconLabel = NULL;
}

void TIconMenu::GetContentSize(float* width, float* height)
{
		if (iconLabel) {
				*width = bounds.Width();
				*height = bounds.Height();
		}
		else
				BMenuItem::GetContentSize(width, height);
}

void TIconMenu::DrawContent()
{
		if (iconLabel) {
				Menu()->SetDrawingMode(B_OP_OVER);

				float width, height;

				Menu()->GetPreferredSize(&width, &height);

				BRect destBounds = bounds;
				destBounds.OffsetBy(8.0, ((height - bounds.Height()) * 0.5) - 1);

				// Scaling the icon is left as an exercise for the reader :)
				Menu()->DrawBitmap(iconLabel, bounds, destBounds);
		}
		else
				BMenuItem::DrawContent();
}

//-------------------------------------------------------------- IconMenu.cpp --
