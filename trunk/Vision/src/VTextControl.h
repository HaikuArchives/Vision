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
 * Contributor(s): Wade Majors <guru@startrek.com>
 */
 

// Class description:
// VTextControl is a derivative of BTextControl which adds context menus

// it's intention is to be fully compliant and portable, so it can easily
// be dropped into other applications as well.
 
#ifndef _VTEXTCONTROL_H_
#define _VTEXTCONTROL_H_

#include <TextControl.h>

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
                                         uint32 = B_WILL_DRAW | B_NAVIGABLE);
                           VTextControl (BMessage *);
    virtual                ~VTextControl (void);
    virtual void           AllAttached (void);
    virtual void           MouseDown (BPoint);

  private:
    BPopUpMenu                      *myPopUp;
};

#endif
