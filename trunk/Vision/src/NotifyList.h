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
 * Contributor(s): Rene Gollent
 *
 */

#ifndef _NOTIFYLIST_H_
#define _NOTIFYLIST_H_

#include <ListView.h> 

class Theme;

class NotifyList : public BListView
{
  public:
    NotifyList (BRect);
    virtual ~NotifyList (void);
    
    void UpdateList (BList *);
    
    virtual void AttachedToWindow (void);
    virtual void MessageReceived (BMessage *);
  
  private:
    Theme * fActiveTheme;
};

class NotifyListItem : public BStringItem
{
  public:
    NotifyListItem (const char *, bool);
    virtual ~NotifyListItem (void);
    void SetState (bool);
    bool GetState (void);
    
    virtual void DrawItem (BView *, BRect, bool);
  
  private:
    bool fNotifyState;
};

#endif // _NOTIFYLIST_H_
