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
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 */
 
#ifndef _NAMES_H_
#define _NAMES_H_

#include <ListItem.h>
#include <ListView.h>
#include <String.h>

class BPopUpMenu;
class BMenu;
class Theme;

class NameItem : public BListItem
{
	public:

																		NameItem (
																			const char *,
																			int32);
		BString												 Name (void) const;
		BString												 Address (void) const;
		int32													 Status (void) const;

		void														SetName (const char *);
		void														SetAddress (const char *);
		void														SetStatus (int32);

		virtual void										DrawItem (BView *, BRect , bool = false);

	private:

		BString												 myName,
																			myAddress;
		int32													 myStatus;
};

class NamesView : public BListView
{
	public:

																		NamesView (BRect);
		virtual												 ~NamesView (void);
		virtual void										AttachedToWindow (void);
		virtual void										DetachedFromWindow (void);
		virtual void										MouseDown (BPoint);
		virtual void										MouseMoved (BPoint, uint32, const BMessage *);
		virtual void										MouseUp (BPoint);
		virtual void										KeyDown (const char *, int32);
		virtual void										MessageReceived (BMessage *);

		void														ClearList (void);

	private:

		bool														fTracking;

		BPopUpMenu											*fMyPopUp;
		BMenu													 *fCTCPPopUp;
		int32													 fLastSelected,
																			fLastButton,
																			fCurrentindex;
		Theme													 *fActiveTheme;
};

#endif
