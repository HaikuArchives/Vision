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
 * Contributor(s): Rene Gollent
 */
 
#ifndef _PREFGENERAL_H
#define _PREFGENERAL_H

#include <View.h>

const uint32 M_GENERALPREFS_SELECTION_CHANGED = 'gpsc';
const int32 C_PREFS_COUNT = 5;

class BListView;
class BBox;

class GeneralPrefsView : public BView
{
  public:
    GeneralPrefsView (BRect, const char *, uint32, uint32);
    virtual ~GeneralPrefsView (void);
    virtual void MessageReceived (BMessage *);
    virtual void AttachedToWindow (void);
    virtual void AllAttached (void);
    virtual void Show (void);
  
  private:
    BListView *prefsList;
    BBox *prefsBox;
    BView *prefsItems[C_PREFS_COUNT];
    int32 lastindex;
};

#endif // _PREFGENERAL_H
