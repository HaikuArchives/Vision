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
 
#ifndef _CHANNELAGENT_H_
#define _CHANNELAGENT_H_

#include <Rect.h>
#include <String.h>

#include "ClientAgent.h"

class BScrollView;
class ServerWindow;
class NamesView;

class ChannelAgent : public ClientAgent
{
  public:

                            ChannelAgent (
                              const char *,       // id 
                              int32,              // sid
                              const char *,       // serverName
                              const char *,       // nick
                              BMessenger &, // sMsgr (ServerAgent pointer)
                              BRect &);             // frame
    virtual                 ~ChannelAgent (void);

	virtual void			AttachedToWindow (void);
	virtual void			MessageReceived (BMessage *);
	virtual void            Parser (const char *);
	virtual void            TabExpansion (void);
	
    bool                    RemoveUser (const char *);
    int                     FindPosition (const char *);
    void                    UpdateMode (char, char);
    void                    ModeEvent (BMessage *);

    static int              SortNames (const void *, const void *);
    

    
  private:
	void                    Init();

    BString                 chanMode,
                            chanLimit,
                            chanLimitOld,
                            chanKey,
                            chanKeyOld,
                            lastExpansion;

    rgb_color               ctcpReqColor,
                            ctcpRpyColor,
                            whoisColor,
                            errorColor,
                            quitColor,
                            joinColor,
                            noticeColor;

    int32                   userCount,
                            opsCount;

    friend class            ClientAgent;
    NamesView               *namesList;
    BScrollView             *namesScroll;

};


#endif
