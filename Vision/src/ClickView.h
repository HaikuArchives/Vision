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
 */

#ifndef _CLICKVIEW_H
#define _CLICKVIEW_H

#include <View.h>
#include <String.h>

class ClickView : public BView
{
	public:
		BString fLaunchUrl;

		ClickView (BRect frame,
							 const char *name,
							 uint32 resizeMask,
							 uint32 flags,
							 const char *url) 
			: BView (frame, name, resizeMask, flags)
				{
					fLaunchUrl = url;
				};


		virtual void MouseDown (BPoint);
};

#endif
