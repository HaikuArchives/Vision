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

#ifndef _STATUSVIEW_H_
#define _STATUSVIEW_H_

#define STATUS_ALIGN_RIGHT					0
#define STATUS_ALIGN_LEFT					 1

#define STATUS_HEIGHT							 13

#include <View.h>
#include <List.h>
#include <String.h>

class StatusItem;

class StatusView : public BView
{
	BList								 items;

	public:
												StatusView (BRect);
		virtual						 ~StatusView (void);

		void								AddItem (StatusItem *, bool);
		StatusItem					*ItemAt (int32) const;

		void								SetItemValue (int32, const char *, bool = true);
		virtual void				Draw (BRect);
		virtual void		MouseDown (BPoint);
		void								Clear (void);

	protected:
		void								DrawSplit (float);
};

class StatusItem
{
	public:
												StatusItem (const char *,
																		const char *,
										int32 = STATUS_ALIGN_RIGHT);

		virtual							 ~StatusItem (void);
		
		void								GeneratePopUp(BPoint, BRect);
	
	private:
		BString							 label,
												value;
		BRect								 frame;
		int32								 alignment;

		friend class							 StatusView;
};

#endif
