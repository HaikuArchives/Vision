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
 */

#ifndef _AGENTDOCK_H_
#define _AGENTDOCK_H_

#ifdef GNOME_BUILD
#  include "gnome/View.h"
#elif BEOS_BUILD
#  include <View.h>
#endif

class WindowList;
class BScrollView;
class BStringView;

class AgentDockHeader : public BView
{
  public:
                           AgentDockHeader (BRect, const char *, uint32);
    virtual                ~AgentDockHeader (void);
  
  private:
    
    BStringView           *headerView;

};

class AgentDockWinList : public BView
{
  public:
                          AgentDockWinList (BRect);
    virtual               ~AgentDockWinList (void);
    
    WindowList            *pWindowList (void);

  private:
    WindowList            *winList;
    BScrollView           *winListScroll;
    
    AgentDockHeader       *aHeader;

};

class AgentDockNotifyList : public BView
{
  public:
                          AgentDockNotifyList (BRect);
    virtual               ~AgentDockNotifyList (void);

  private:
    
    AgentDockHeader       *aHeader;

};

class AgentDock : public BView
{
  public:
                            AgentDock (BRect);
    virtual                 ~AgentDock (void);
    
    void                    AddWinList (void);
    void                    AddNotifyList (void);
    WindowList              *pWindowList (void); 
    	
  private:
    
    BRect                   workingFrame;
    
    AgentDockWinList        *winListAgent;
    AgentDockNotifyList     *notifyAgent;
    
};



#endif
