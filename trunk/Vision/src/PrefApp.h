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
 
#ifndef _PREFAPP_H
#define _PREFAPP_H

#include <View.h>

const uint32 M_APPWINDOWPREFS_SETTING_CHANGED = 'awsc';

class BCheckBox;

class AppWindowPrefsView : public BView
{
  public:
    AppWindowPrefsView (BRect);
    virtual ~AppWindowPrefsView (void);
    virtual void MessageReceived (BMessage *);
    virtual void AttachedToWindow (void);
    virtual void AllAttached (void);
  private:
    BCheckBox *versionParanoid,
              *catchAltW,
              *timeStamp,
              *logEnabled,
              *logFileTimestamp;
};

#endif // _PREFAPP_H
