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
 
#ifndef _PREFAPPEARANCE_H
#define _PREFAPPEARANCE_H

#include <View.h>
#include <Messenger.h>
#include "VisionBase.h"

const uint32 M_ELEMENT_SELECTION_CHANGED = 'aesc';
const uint32 M_FONT_CHANGE = 'fpfc';
const uint32 M_FONT_SIZE_CHANGE = 'fpsc';
const uint32 M_COLOR_INVOKED = 'cpci';
const uint32 M_COLOR_CHANGED = 'cpcc';

class BMenuField;
class BaseColorControl;
class BStringView;
class ColorLabel;

class AppearancePrefsView : public BView
{
  public:
    AppearancePrefsView (BRect);
    virtual ~AppearancePrefsView (void);
    virtual void MessageReceived (BMessage *);
    virtual void AttachedToWindow (void);
    virtual void AllAttached (void);
  private:
    void UpdateSampleFont(int32);
    void SetSampleColors (int32);
    void SetFontControlState (int32);

    BMenuField  *clientFont[7],
                *fontSize[7],
                *fontMenu;
    BaseColorControl *sample;
    BStringView *sampleText;
    ColorLabel *label;
    rgb_color colors [MAX_COLORS];
    BMessenger picker;
    int32      lastwhich;

};

#endif // _PREFAPPEARANCE_H
