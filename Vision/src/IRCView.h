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
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
 
#ifndef _IRCVIEW_H_
#define _IRCVIEW_H_

#include <TextView.h>

class VTextControl;
class BFont;
class ClientAgent;

struct IRCViewSettings;

class IRCView : public BTextView
{
  bool     tracking;
  float    lasty;

  public:
    IRCView (
      BRect,
      BRect,
      VTextControl *,
      ClientAgent *);

    ~IRCView (void);

    virtual void            MouseDown (BPoint);
    virtual void            KeyDown (
                              const char * bytes,
                              int32 numBytes);
    virtual void            FrameResized (float, float);

    int32                   DisplayChunk (
                              const char *,
                              const rgb_color *,
                              const BFont *);
    int32                   URLLength (const char *);
    int32                   FirstMarker (const char *);

    void                    ClearView (bool);
    void                    SetColor (int32, rgb_color);
    void                    SetFont  (int32, const BFont *);

  private:
    IRCViewSettings         *settings;

};

#endif
