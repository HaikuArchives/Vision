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
 */

#ifndef _VISION_H_
#define _VISION_H_

#include <Application.h>
#include <String.h>
#include <Locker.h>

#include "VisionBase.h"

class BFont;
class Theme;
class AboutWindow;
class SetupWindow;
class PrefsWindow;
class ClientWindow;
class NetworkWindow;
class SettingsFile;
class BLocker;
class BNetEndpoint;

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

    BString                 VisionVersion (int);
    
    void                    InitDefaults (void);
    
    void                    LoadDefaults (int32);

    void                    ClientFontFamilyAndStyle (int32, const char *,
                              const char *);
    void                    ClientFontSize (int32, float);
    const BFont             *GetClientFont (int32) const;
    
    const BRect             GetRect (const char *);
    status_t                SetRect (const char *, BRect);
    
    const char              *GetString (const char *) const; 
    status_t                SetString (const char *, int32 index, const char *); 
    
    rgb_color               GetColor (int32) const;
    void                    SetColor (int32, const rgb_color);

    BString                 GetEvent (int32) const;
    void                    SetEvent (int32, const char *);

    BString                 GetCommand (int32);
    void                    SetCommand (int32, const char *);

    bool                    GetBool (const char *);
    status_t                SetBool (const char *, bool);
    
    BMessage                GetNetwork (const char *);
    BMessage				GetNetwork (int32);
    status_t                SetNetwork (const char *, BMessage *);
    status_t                RemoveNetwork (const char *);
    bool                    CheckNetworkValid (const char *);

    sem_id                  GetShutdownSem(void);
    
    Theme *                 ActiveTheme(void);
    
    const char              *GetThreadName (int);
    
    void                    BenchOut (const char *);

    void                    Broadcast (BMessage *);
    void                    Broadcast (BMessage *, const char *, bool = false);
    
    void                    AddIdent (const char *, const char *);
    void                    RemoveIdent (const char *);
    const char *            GetIdent (const char *);
    static int32            Identity (void *);

    BString                 events[MAX_EVENTS];

    bool                    debugsettings;
    bool                    debugsend;
    bool                    debugrecv;
    bool                    settingsloaded;
    bool                    numBench;

    // used for benchmarking
    int32                   bench1;
    int32                   bench2;

    
    ClientWindow            *pClientWin (void) const;

  private:
	void					InitSettings (void);
    void                    ThreadStates (void);
    bool                    CheckStartupNetworks (void);
    
    volatile bool           ShuttingDown;

    AboutWindow             *aboutWin;
    SetupWindow             *setupWin;
    ClientWindow            *clientWin;
    PrefsWindow             *prefsWin;
    NetworkWindow           *netWin;
    
    SettingsFile			*visionSettings;
    
    rgb_color               colors[MAX_COLORS];
    BFont                   *client_font[MAX_FONTS];
	BString					commands[MAX_COMMANDS];
	BMessage                idents;
	BLocker                 identLock,
	                          settingsLock;
	thread_id               identThread;
	int32                   identSocket;
	Theme                   *activeTheme;
	sem_id                  shutdownSem;
};

const uint32 M_SETUP_CLOSE           = 'vasc';
const uint32 M_SETUP_SHOW            = 'vass';
const uint32 M_NETWORK_SHOW          = 'vans';
const uint32 M_NETWORK_CLOSE         = 'vanc';
const uint32 M_PREFS_SHOW            = 'vaps';
const uint32 M_PREFS_CLOSE           = 'vapc';

const uint32 M_CONNECT_NETWORK       = 'vacn';

const uint32 VIS_NETWORK_DATA        = 'vndc';
const uint32 VIS_NETWORK_DEFAULTS    = 'vndd';

const int SET_SERVER    = 1;
const int SET_GENERAL   = 2;
const int SET_WINDOW    = 3;
const int SET_NOTIFY    = 4;
const int SET_FONT      = 5;
const int SET_COLOR     = 6;
const int SET_STRINGS   = 7;

const int VERSION_VERSION = 1;
const int VERSION_DATE    = 2;

const int THREAD_S = 1;  // socket
const int THREAD_L = 2;  // thread

#endif
