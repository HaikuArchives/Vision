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

#ifndef _CLIENTWINDOW_H_
#define _CLIENTWINDOW_H_

#include <Window.h>

#define STATUS_SERVER               0
#define STATUS_LAG                    1
#define STATUS_NICK                 2
#define STATUS_MODES                  3
#define STATUS_USERS                4
#define STATUS_OPS                    5
#define STATUS_META                 6

class BMenu;
class BMenuBar;
class BMenuItem;
class BScrollView;
class BView;
class ServerAgent;
class StatusView;
class WindowList;
class WindowListItem;

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
    virtual               ~ClientWindow (void);

    virtual void          MessageReceived (BMessage *);
    virtual void          DispatchMessage (BMessage *, BHandler *);
    virtual bool          QuitRequested (void);
    virtual void          ScreenChanged (BRect, color_space);
    
    void                  UpdateAgentRect (void);

    WindowList            *winList;  
    StatusView            *status;
    BScrollView           *winListScroll;
    WindowListItem        *winListI;
    
    BRect                 *agentrect;
    BView                 *bgView;

  private:
    void                  Init (void);

};

const uint32 M_UPDATE_STATUS				= 'cwus';

#endif
