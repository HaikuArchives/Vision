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
 * Contributor(s): Todd Lair
 *								 
 */
 
#ifndef URLCRUNCH_H_
#define URLCRUNCH_H_

#include <Locker.h>
#include <String.h>
#include <TypeConstants.h>

#ifndef B_URL_MIME_PREFIX
#define B_URL_MIME_PREFIX	"application/x-vnd.Be.URL."
#endif


class URLCrunch
{
	BString			buffer;
	int32				current_pos;

	public:

						URLCrunch (const char *, int32);
						~URLCrunch (void);
	int32				Crunch (BString *);

	static status_t		UpdateTagList(void);

private:
	static BLocker		fLocker; // protects members below
	static const char	**fTags;
};

#endif
