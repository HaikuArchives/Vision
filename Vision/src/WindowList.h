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

#ifndef _WINDOWLIST_H_
#define _WINDOWLIST_H_

#include <ListItem.h>
#include <ListView.h>
#include <String.h>


class BPopUpMenu;
class BMenu;
class BList;
class ServerAgent;
class ClientAgent;
class ClientWindow;
class BScrollView;


class WindowListItem : public BListItem
{
  public:
                                    WindowListItem (const char *,
                                                    int32,
                                                    int32,
                                                    int32,
                                                    BView *);
    BString                         Name (void) const;
    int32                           Sid (void) const;
    int32                           Type (void) const;
    int32                           Status (void) const;
    BView                           *pAgent (void) const;

    void                            SetName (const char *);
    void                            SetSid (int32);
    void                            SetStatus (int32);

    virtual void                    DrawItem (BView *,
                                              BRect,
                                              bool complete = false);
                                              
  private:
    BString                         myName;
    int32							mySid;
    int32                           myStatus;
    int32                           myType;
    BView                           *myAgent;
};


class WindowList : public BListView
{
   public:
                                    WindowList (BRect);
    virtual                         ~WindowList (void);
    virtual void                    AllAttached (void);
    virtual void                    MouseDown (BPoint);
    virtual void                    MessageReceived (BMessage *);
    virtual void 					SelectionChanged (void);
    virtual void                    KeyDown (const char *, int32);
	
    void                            SetColor (int32, rgb_color);
    rgb_color                       GetColor (int32) const;
    void                            SetFont (int32, const BFont *);
    void                            ClearList (void);
    void                            Activate (int32);
    void                            CloseActive (void);
    //int32                       	GetActiveAgent (void);
    
    ClientAgent                     *Agent (int32, const char *);
    
    void                            AddAgent (BView *, int32, const char *, int32, bool);
    void                            RemoveAgent (BView *, WindowListItem *);
	
  private:

    BPopUpMenu                      *myPopUp;
    int32                           lastSelected;
    int32                           lastButton;

    rgb_color                       textColor,
                                    newsColor,
                                    nickColor,
                                    selColor,
                                    bgColor;
                                    
    static int                      SortListItems (const void *, const void *);
    
};

const uint32 M_MENU_NUKE            = 'wlmn';
#endif
