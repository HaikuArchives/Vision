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

#include <Window.h>

class BTextView;
class BView;

class AboutWindow : public BWindow
{
  BTextView             *credits;

  public:
                          AboutWindow (void);
    virtual               ~AboutWindow (void);
  
    virtual void          MessageReceived (BMessage *);
    virtual bool          QuitRequested (void);
    virtual void          DispatchMessage (BMessage *, BHandler *);
    void                  Pulse (void);
    void                  AboutImage (const char *, bool);

  private:
    BView                 *background;
    bool                  EasterEggOn;
    BView                 *graphic;
};


const uint32 M_ABOUT_CLOSE           = 'abcl'; // The party has to stop sometime
const uint32 M_ABOUT_ORGY            = 'abor'; // Gummy Orgy
const uint32 M_ABOUT_BUDDYJ          = 'abbj'; // Buddy Jesus!

#endif