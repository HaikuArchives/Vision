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
 
#ifndef _MESSAGEWINDOW_H_
#define _MESSAGEWINDOW_H_

#include <Rect.h>
#include <String.h>

#include "ClientAgent.h"

class MessageAgent : public ClientAgent
{
  public:
  
                         MessageAgent (BRect &,
                                       const char *,
                                       int32,
                                       const char *,
                                       const BMessenger &,
                                       const char *,
                                       const char *,
                                       bool = false,
                                       bool = false,
                                       const char * = "",
                                       const char * = "");
                         
                         ~MessageAgent (void);

    virtual void         MessageReceived (BMessage *);
    virtual void         Show (void);
    virtual void         Parser (const char *);
    virtual void         DroppedFile (BMessage *);
    virtual void         TabExpansion (void);


  private:
    BString                     chatAddy,
                                chatee,
                                dIP,
                                dPort;

    bool                        dChat,
                                dInitiate,
                                dConnected;

    int                         mySocket,
                                acceptSocket;

    thread_id                   dataThread;

};


#endif