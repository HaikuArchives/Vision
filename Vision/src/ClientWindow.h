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

#ifndef _CLIENTWINDOW_H_
#define _CLIENTWINDOW_H_

#include <Window.h>
#include <String.h>

#define STATUS_SERVER               0
#define STATUS_LAG                    1
#define STATUS_NICK                 2
#define STATUS_USERS                  3
#define STATUS_OPS                  4
#define STATUS_MODES                  5
#define STATUS_META                 6

class BMenu;
class BMenuBar;
class BMenuItem;
class BScrollView;
class BView;
class BMessageRunner;
class ServerAgent;
class StatusView;
class WindowList;
class WindowListItem;
class ClientWindowDock;
class ResizeView;

class ClientWindow : public BWindow
{

  protected:
    BMenuBar              *menubar;
    BMenu                 *mServer,
                          *mEdit,
                          *mTools,
                          *mWindow,
                          *mHelp;
                          
    ServerAgent           *serverAgent;

  public:
                          ClientWindow (BRect);

    virtual void          MessageReceived (BMessage *);
    virtual void          DispatchMessage (BMessage *, BHandler *);
    virtual bool          QuitRequested (void);
    virtual void          ScreenChanged (BRect, color_space);
    virtual void          Show (void);
    
    // accessors that add/remove menus to the main Vision menu bar
    void                  AddMenu (BMenu *);
    void                  RemoveMenu (BMenu *);
    
    ServerAgent           *GetTopServer (WindowListItem *);
    
    bool                  ServerBroadcast (BMessage *);

    BView                 *bgView;
      
    BRect                 *AgentRect (void);
    WindowList            *pWindowList (void);
    StatusView            *pStatusView (void);
    
    BString               joinStrings;  // used to keep track of channel
                                        // keys on u2 ircds

  private:
    void                  Init (void);
    void                  HandleKey (BMessage *);
    
    bool                  shutdown_in_progress;
    bool                  wait_for_quits;
    
    bool                  altw_catch;    
    
    BMessageRunner        *altwRunner;
    
    BRect                 *agentrect;
    
    StatusView            *status;
    
    ClientWindowDock      *cwDock;
    
    ResizeView            *resize;
    
    sem_id                shutdownSem;

};

const uint32 M_UPDATE_STATUS                = 'cwus';
const uint32 M_OBITUARY                     = 'cwob';
const uint32 M_CW_ALTW                      = 'cwaw';
const uint32 M_CW_ALTW_RESET                = 'cwwr';
const uint32 M_CW_ALTP                      = 'cwap';
const uint32 M_OPEN_TERM                    = 'cwot';
const uint32 M_MAKE_NEW_NETWORK             = 'cwmn';

const uint32 M_STATUS_CLEAR                 = 'cwsc';
const uint32 M_STATUS_ADDITEMS              = 'cwsa';



#endif
