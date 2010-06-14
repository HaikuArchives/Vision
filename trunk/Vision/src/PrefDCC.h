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
 */

#ifndef _PREFDCC_H
#define _PREFDCC_H

#include <View.h>

class VTextControl;
class BMenuField;
class BCheckBox;
class BBox;

class DCCPrefsView : public BView
{
	public:
		DCCPrefsView (BRect);
		virtual ~DCCPrefsView (void);
		virtual void MessageReceived (BMessage *);
		virtual void AttachedToWindow (void);
		virtual void AllAttached (void);
		virtual void FrameResized (float, float);
	
	private:
		BMenuField *fBlockSize;
		VTextControl *fDefDir,
								 *fDccPortMin,
								 *fDccPortMax;
		BCheckBox *fAutoAccept,
							*fPrivateCheck;
		BBox *fBox;
};

#endif // _PREFDCC_H
