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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
 
#ifndef _HISTORYMENU_H_
#define _HISTORYMENU_H_

#define BACK_BUFFER_SIZE                20

class VTextControl;
class BString;

class HistoryMenu : public BView
{
	BString                 backBuffer[BACK_BUFFER_SIZE];
	int32                   bufferFree,
	                        bufferPos;
	int32                   i;

	bool						tracking;
	bigtime_t				mousedown;
	rgb_color				tricolor;

	public:

							HistoryMenu (BRect);
	virtual void			Draw (BRect);
	virtual void			AttachedToWindow (void);
	virtual void			MouseDown (BPoint);
	virtual void			MouseMoved (BPoint, uint32, const BMessage *);
	virtual void			MouseUp (BPoint);

	void						PreviousBuffer (VTextControl *);
	void						NextBuffer (VTextControl *);
	BString					Submit (const char *);
	bool						HasHistory (void) const;
	void						DoPopUp (bool);
};

#endif
