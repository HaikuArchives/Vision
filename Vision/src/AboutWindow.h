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
 *                 Brian Luft
 *                 Todd Lair
 *                 Rene Gollent
 */ 

#ifndef _ABOUTWINDOW_H_
#define _ABOUTWINDOW_H_

#ifdef GNOME_BUILD
#  include "Window.h"
#elif BEOS_BUILD
#  include <Window.h>
#endif

class BTextView;
class BView;

class AboutWindow : public BWindow
{
  BTextView             *credits;

  public:
                          AboutWindow (void);
    virtual               ~AboutWindow (void);
  
    virtual bool          QuitRequested (void);
    virtual void          DispatchMessage (BMessage *, BHandler *);
    void                  Pulse (void);

  private:
    BView                 *background;
    bool                  EasterEggOn;
    BView                 *graphic;
    void                  AboutImage (const char *, bool);
};


const uint32 M_ABOUT_CLOSE           = 'abcl'; // The party has to stop sometime

#endif
