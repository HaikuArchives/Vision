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
 */

#ifndef _VISION_H_
#define _VISION_H_

#include <Application.h>
#include <String.h>

#include "VisionBase.h"

class BFont;
class AboutWindow;
class SetupWindow;
class ClientWindow;
class SettingsFile;

extern class VisionApp * vision_app;

class VisionApp : public BApplication
{
  public:
                            VisionApp (void);

    virtual void            MessageReceived (BMessage *);
    virtual void            AboutRequested (void);
    virtual bool            QuitRequested (void);
    virtual void            ArgvReceived (int32, char **);
    virtual void            ReadyToRun (void);
    
    void                    LoadURL (const char *);

    BString                 VisionVersion (void);
    
    void                    LoadDefaults (int32);

    void                    ClientFontFamilyAndStyle (int32, const char *,
                              const char *);
    void                    ClientFontSize (int32, float);
    const BFont             *GetClientFont (int32) const;
    
    status_t                SetRect (const char *, BRect);
    
    const char              *GetString (const char *) const; 
    status_t                SetString (const char *, const char *); 
    
    rgb_color               GetColor (int32) const;
    void                    SetColor (int32, const rgb_color);

    BString                 GetEvent (int32) const;
    void                    SetEvent (int32, const char *);

    BString                 GetCommand (int32);
    void                    SetCommand (int32, const char *);

    bool                    GetBool (const char *);
    status_t                SetBool (const char *, bool);
    
    const char              *GetThreadName (void);

    void                    Broadcast (BMessage *);
    void                    Broadcast (BMessage *, const char *, bool = false);
    	
	BString					events[MAX_EVENTS];
	
	bool                    debugsettings;
	bool                    debugsend;
	bool                    debugrecv;
	bool                    settingsloaded;
	

	ClientWindow            *pClientWin (void) const;

  private:
	void					InitSettings(void);
    AboutWindow             *aboutWin;
    SetupWindow             *setupWin;
    ClientWindow            *clientWin;
    SettingsFile			*visionSettings;
    
    rgb_color               colors[MAX_COLORS];
    BFont                   *client_font[MAX_FONTS];
	BString					commands[MAX_COMMANDS];

};

const uint32 M_SETUP_CLOSE           = 'vasc';
const uint32 M_SETUP_SHOW            = 'vass';

const int SET_SERVER    = 1;
const int SET_GENERAL   = 2;
const int SET_WINDOW    = 3;

#endif
