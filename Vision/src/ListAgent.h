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

#ifndef _LISTAGENT_H_
#define _LISTAGENT_H_

#include <View.h>
#include <String.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <regex.h>

class BListView;
class BScrollView;
class BMenuItem;
class StatusView;
class WindowSettings;
class WindowListItem;

class ListAgent : public BView
{
  public:

                            ListAgent (BRect, const char *, BMessenger *);
    virtual                 ~ListAgent (void);
    virtual void            MessageReceived (BMessage *);
    virtual void            FrameResized (float, float);
    virtual void            AttachedToWindow (void);
	virtual void			AllAttached (void);
    static int              SortChannels (const void *, const void *);
    static int              SortUsers (const void *, const void *);
    virtual void			Show(void);

    float                   ChannelWidth (void) const;
    WindowListItem          *agentWinItem;
    BMessenger              msgr;
    
  private:
    BMessenger              *sMsgr;
    BMessageRunner          *listUpdateTrigger;
	BMenuBar				*mBar;
    BListView               *listView;
    BScrollView             *scroller;
    StatusView              *status;
    BList                   list,
                              showing,
                              nextbatch;
    BString                 filter,
                              find,
                              statusStr;
    regex_t                 re,
                              fre;
                              
    bool                    processing;
    float                   channelWidth,
                              topicWidth,
                              sChannelWidth,
                              sTopicWidth,
                              sLineWidth;

    BMenuItem               *mChannelSort,
                              *mUserSort,
                              *mFilter,
                              *mFind,
                              *mFindAgain;

    WindowSettings          *settings;
};

const uint32 M_LIST_FIND               = 'lalf';
const uint32 M_LIST_FAGAIN             = 'lafa';
const uint32 M_LIST_SORT_CHANNEL       = 'lasc';
const uint32 M_LIST_SORT_USERS         = 'lasu';
const uint32 M_LIST_FILTER             = 'lafr';
const uint32 M_LIST_INVOKE             = 'lali';
const uint32 M_LIST_UPDATE             = 'lalu';
#endif
