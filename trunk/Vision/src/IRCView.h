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
 
#ifndef _IRCVIEW_H_
#define _IRCVIEW_H_

#ifdef GNOME_BUILD
#  include "gnome/TextView.h"
#elif BEOS_BUILD
#  include <TextView.h>
#endif

class VTextControl;
class BFont;
class BPopUpMenu;
class ClientAgent;

struct IRCViewSettings;

class IRCView : public BTextView
{
  public:
                            IRCView (
                              BRect,
                              BRect,
                              VTextControl *,
                              ClientAgent *);

    virtual                 ~IRCView (void);

    virtual void            MouseDown (BPoint);
    virtual void            KeyDown (
                              const char * bytes,
                              int32 numBytes);
    virtual void            FrameResized (float, float);

    int32                   DisplayChunk (
                              const char *,
                              const rgb_color *,
                              const BFont *);

    void                    ClearView (bool);
    void                    SetColor (int32, rgb_color);
    void                    SetFont  (int32, const BFont *);
    
  private:
    IRCViewSettings         *settings;
    BPopUpMenu              *myPopUp;
    void                    BuildPopUp (void);
    int32                   URLLength (const char *);
    int32                   FirstURLMarker (const char *);
    bool                    IsAnUpperOrLowerOrUnderbarOrNumericDigit(const char thedigit);
    int32                   CreateSelection(int32 &start);
    bool     tracking;
    float    lasty;


};

#endif
