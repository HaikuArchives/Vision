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
 *                 Jean-Baptiste M. Queru <jbq@be.com>
 *                 Seth Flaxman
 */

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <List.h>

#include "Vision.h"
#include "WindowList.h"
#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ServerAgent.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
/// Begin WindowList functions
//////////////////////////////////////////////////////////////////////////////

WindowList::WindowList (BRect frame)
  : BListView (
    frame,
    "windowList",
    B_SINGLE_SELECTION_LIST,
    B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM)
{
//  myPopUp = new BPopUpMenu("Window Selection", false, false);
//
//  BMenuItem *item;
//  myPopUp->AddItem (item = new BMenuItem("Close", new BMessage (M_MENU_NUKE)));
//  item->SetTarget (this);
//  
//  myPopUp->SetFont (be_plain_font);
  
  textColor   = vision_app->GetColor (C_WINLIST_TEXT);
  newsColor   = vision_app->GetColor (C_WINLIST_NEWS);
  nickColor   = vision_app->GetColor (C_WINLIST_NICK);
  selColor    = vision_app->GetColor (C_WINLIST_SELECTION);
  bgColor     = vision_app->GetColor (C_WINLIST_BACKGROUND);
  
  BListView::SetFont (vision_app->GetClientFont (F_WINLIST));

  SetViewColor (bgColor);
  
  SetTarget (this);
}

WindowList::~WindowList (void)
{
  delete myPopUp;
}

void
WindowList::AllAttached (void)
{
  myPopUp = new BPopUpMenu("Window Selection", false, false);

  BMenuItem *item;
  myPopUp->AddItem (item = new BMenuItem("Close", new BMessage (M_MENU_NUKE)));
//  item->SetTarget (this);
  
  myPopUp->SetFont (be_plain_font);
  myPopUp->SetTargetForItems (this);

}

void
WindowList::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_MENU_NUKE:
    {
      printf ("M_MENU_NUKE\n");
      CloseActive();
      break;
    }
    
    default:
      BListView::MessageReceived (msg);
  }
}

void
WindowList::CloseActive (void)
{
  printf ("CloseActive()\n");
  WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (CurrentSelection())));
  if (myItem)
  {
    BMessage killMsg (M_CLIENT_QUIT);
    killMsg.AddBool ("vision:winlist", true);
        
    BView *killTarget (myItem->pAgent());
        
    if ((killTarget = dynamic_cast<ClientAgent *>(killTarget)))
      dynamic_cast<ClientAgent *>(killTarget)->msgr.SendMessage (&killMsg);
  }
}

void
WindowList::MouseDown (BPoint myPoint)
{
  int32 selected (IndexOf (myPoint));
  bool handled (false);
  
  if (selected >= 0)
  {
    BMessage *inputMsg (Window()->CurrentMessage());
    int32 mousebuttons (0),
          keymodifiers (0);

    inputMsg->FindInt32 ("buttons", &mousebuttons);
    inputMsg->FindInt32 ("modifiers", &keymodifiers);
    
    Select (selected);

    if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
    && (keymodifiers & B_SHIFT_KEY)   == 0
    && (keymodifiers & B_OPTION_KEY)  == 0
    && (keymodifiers & B_COMMAND_KEY) == 0
    && (keymodifiers & B_CONTROL_KEY) == 0)
    {

      BListItem *item = ItemAt(IndexOf(myPoint));
      if (item && !item->IsSelected())
        Select (IndexOf (myPoint));

      myPopUp->Go (
        ConvertToScreen (myPoint),
        true,
        false,
        ConvertToScreen (ItemFrame (selected)));
    }
    handled = true;
  }
  else
  {
    // find the active agent and select it
    WindowListItem *activeagent (0);
    for (int32 i (1); i <= CountItems(); ++i)
    { 
      WindowListItem *aitem ((WindowListItem *)ItemAt (i - 1));
      if (!aitem->pAgent()->IsHidden())
      {
        activeagent = aitem;
        break;
      
      }
    }
    
    if (activeagent)
      Select (IndexOf (activeagent));
      
    handled = true;
  }    

  lastSelected = selected;
  
  if (!handled)
    BListView::MouseDown (myPoint);
}

void 
WindowList::KeyDown (const char * bytes, int32 numBytes) 
{
  if (CurrentSelection() == -1)
    return;
    
  BMessage inputMsg (M_INPUT_FOCUS); 
  BString buffer; 

  buffer.Append (bytes, numBytes); 
  inputMsg.AddString ("text", buffer.String()); 

  WindowListItem *activeitem ((WindowListItem *)ItemAt (CurrentSelection()));
  ClientAgent *activeagent (dynamic_cast<ClientAgent *>(activeitem->pAgent()));
  if (activeagent)
    activeagent->msgr.SendMessage (&inputMsg);
}

void
WindowList::SelectionChanged (void)
{
  int32 currentIndex (CurrentSelection());
  if (currentIndex >= 0) // dont bother casting unless somethings selected
  {
    WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (currentIndex)));
    if (myItem)
      Activate (currentIndex);
  }
}

void
WindowList::SetColor (int32 which, rgb_color color)
{
  switch (which)
  {
    case C_WINLIST_NEWS:
    {
       newsColor = color;

       for (int32 i = 0; i < CountItems(); ++i)
       {
         WindowListItem *item ((WindowListItem *)ItemAt (i));

         if ((item->Status() & WIN_NEWS_BIT) != 0)
           InvalidateItem (i);
       }
       break;
    }

    case C_WINLIST_NICK:
    {
       nickColor = color;

       for (int32 i = 0; i < CountItems(); ++i)
       {
         WindowListItem *item ((WindowListItem *)ItemAt (i));

         if ((item->Status() & WIN_NICK_BIT) != 0)
           InvalidateItem (i);
       }
       break;
    }
  }
}

rgb_color
WindowList::GetColor (int32 which) const
{
  rgb_color color (textColor);

  if (which == C_WINLIST_NEWS)
    color = newsColor;

  if (which == C_WINLIST_NICK)
    color = nickColor;

  if (which == C_WINLIST_BACKGROUND)
    color = bgColor;
  
  if (which == C_WINLIST_SELECTION)
    color = selColor;

  return color;
}


void
WindowList::SetFont (int32 which, const BFont *font)
{
  if (which == F_NAMES)
  {
    BListView::SetFont (font);
    Invalidate();
  }
}

void
WindowList::ClearList (void)
{
  // never ever call this function unless you understand
  // the consequences!
  int32 i,
        all (CountItems());

  for (i = 0; i <= all; i++)
    RemoveItem (0L);
}

//int32
//WindowList::GetActiveAgent (void)
//{
//  return activeindex;
//}

ClientAgent *
WindowList::Agent (int32 serverId, const char *aName)
{
  ClientAgent *agent (0);

  for (int32 i = 0; i < CountItems(); ++i)
  {
    WindowListItem *item ((WindowListItem *)ItemAt (i));
    if ((strcasecmp (aName, reinterpret_cast<ClientAgent *>(item->pAgent())->Id().String()) == 0)
    &&  (item->Sid() == serverId)) 
    {
      agent = reinterpret_cast<ClientAgent *>(item->pAgent());
      break;      
    }
  }
  return agent;
}

void
WindowList::AddAgent (BView *agent, int32 serverId, const char *name, int32 winType, bool activate)
{
  int32 itemindex;
  
  WindowListItem *newagentitem (new WindowListItem(name, serverId, winType, WIN_NORMAL_BIT, agent));
  AddItem (newagentitem);
  BView *newagent;
  newagent = newagentitem->pAgent();
  if (serverId == ID_SERVER)
  {
    int32 newsid (reinterpret_cast<ServerAgent *>(newagent)->Sid());
    newagentitem->SetSid (newsid);
    serverId = newsid;
  }
  SortItems (SortListItems);
  itemindex = IndexOf (newagentitem);

  // give the agent its own pointer to its WinListItem,
  // so it can quickly update it's status entry
  if ((newagent = dynamic_cast<ClientAgent *>(newagent)))
  {
    for (int32 i = 0; i < CountItems(); ++i)
    {
      WindowListItem *item = (WindowListItem *)ItemAt (i);
      if ((strcasecmp (name, item->Name().String()) == 0)
      &&  (item->Sid() == serverId)) 
      {
        dynamic_cast<ClientAgent *>(newagent)->agentWinItem = item;
        break;      
      }
    }
  }
  vision_app->pClientWin()->Lock();
  vision_app->pClientWin()->bgView->AddChild (newagent);
  newagent->Hide(); // get it out of the way
  newagent->Sync(); // clear artifacts
  vision_app->pClientWin()->Unlock();
  
  if (activate)  // if activate is true, show the new view now.
    if (CurrentSelection() == -1)
      Select (itemindex); // first item, let SelectionChanged() activate it
    else
      Activate (itemindex);

}

void
WindowList::Activate (int32 index)
{

  WindowListItem *newagentitem ((WindowListItem *)ItemAt (index));
  BView *newagent (newagentitem->pAgent());

  // find the currently active agent (if there is one)
  BView *activeagent (0);
  for (int32 i (1); i <= CountItems(); ++i)
  { 
    WindowListItem *aitem ((WindowListItem *)ItemAt (i - 1));
    if (!aitem->pAgent()->IsHidden())
    {
      activeagent = aitem->pAgent();
      break;
      
    }
  }
  
   
  if ((activeagent != newagent) && (activeagent != 0))
  {
    vision_app->pClientWin()->Lock();
    
    if (activeagent)
    {
      activeagent->Hide(); // you arent wanted anymore!
      activeagent->Sync(); // and take your damned pixels with you!
    }
  
    newagent->Show();
    
    vision_app->pClientWin()->Unlock();
  }
  if (activeagent == 0)
  {
    vision_app->pClientWin()->Lock();
    newagent->Show();
    vision_app->pClientWin()->Unlock();
  }
 
  // activate the input box (if it has one)
  if ((newagent = dynamic_cast<ClientAgent *>(newagent)))
    reinterpret_cast<ClientAgent *>(newagent)->msgr.SendMessage (M_INPUT_FOCUS);
  
  Select (index);
}

void
WindowList::RemoveAgent (BView *agent, WindowListItem *agentitem)
{
  printf ("ho!\n");
  vision_app->pClientWin()->Lock();
  agent->Hide();
  agent->RemoveSelf();
  RemoveItem (agentitem);
  SortItems (SortListItems);
  delete agent;
  Select (0); // TODO select something more intelligently
  vision_app->pClientWin()->Unlock();
}


int
WindowList::SortListItems (const void *name1, const void *name2)
{
  WindowListItem **firstPtr = (WindowListItem **)name1;
  WindowListItem **secondPtr = (WindowListItem **)name2;

  /* Not sure if this can happen, and we
   * are assuming that if one is NULL
   * we return them as equal.  What if one
   * is NULL, and the other isn't?
   */
  if (!firstPtr
  ||  !secondPtr
  ||  !(*firstPtr)
  ||  !(*secondPtr))
  {
    return 0;
  }
  
  int32 firstSid, secondSid;
  BString firstName, secondName;
  
  firstSid = (*firstPtr)->Sid();
  secondSid = (*secondPtr)->Sid();
  
  firstName = (*firstPtr)->Name();
  secondName = (*secondPtr)->Name();
 
  if (firstSid < secondSid)
    return -1;
  else if ((*firstPtr)->Sid() > (*secondPtr)->Sid())
    return 1;
  else
  {
    // same sid, sort by name
    if ((*firstPtr)->Type() == WIN_SERVER_TYPE)
      return -1;
    return firstName.ICompare (secondName);
  } 
}



//////////////////////////////////////////////////////////////////////////////
/// End WindowList functions
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// Begin WindowListItem functions
//////////////////////////////////////////////////////////////////////////////

WindowListItem::WindowListItem (
  const char *name,
  int32 serverId,
  int32 winType,
  int32 winStatus,
  BView *agent)

  : BListItem (),
    myName (name),
    mySid (serverId),
    myStatus (winStatus),
    myType (winType),
    myAgent (agent)
{

}

WindowListItem::~WindowListItem (void)
{
}

BString
WindowListItem::Name (void) const
{
  return myName;
}
int32
WindowListItem::Sid (void) const
{
  return mySid;
}

int32
WindowListItem::Status() const
{
  return myStatus;
}

int32
WindowListItem::Type() const
{
  return myType;
}

BView *
WindowListItem::pAgent() const
{
  return myAgent;
}

void
WindowListItem::SetName (const char *name)
{
  myName = name;
}

void
WindowListItem::SetSid (int32 newSid)
{
  mySid = newSid;
}

void
WindowListItem::SetStatus (int32 winStatus)
{
  myStatus = winStatus;
}

void
WindowListItem::DrawItem (BView *passedFather, BRect frame, bool complete)
{
  WindowList *father (static_cast<WindowList *>(passedFather));

  if (IsSelected())
  {
    father->SetHighColor (father->GetColor (C_WINLIST_SELECTION));
    father->SetLowColor (father->GetColor (C_WINLIST_BACKGROUND));    
    father->FillRect (frame);
  }
  else if (complete)
  {
    father->SetLowColor (father->GetColor (C_WINLIST_BACKGROUND));
    father->FillRect (frame, B_SOLID_LOW);
  }

  font_height fh;
  father->GetFontHeight (&fh);

  father->MovePenTo (
    frame.left + 4,
    frame.bottom - fh.descent);

  BString drawString (myName);
  rgb_color color = father->GetColor (C_WINLIST_TEXT);

  if ((myStatus & WIN_NEWS_BIT) != 0)
    color = father->GetColor (C_WINLIST_NEWS);

  else if ((myStatus & WIN_NICK_BIT) != 0)
    color = father->GetColor (C_WINLIST_NICK);

  if ((myType & WIN_CHANNEL_TYPE) != 0)
    drawString.Prepend ("  ");

  if ((myType & WIN_MESSAGE_TYPE) != 0)
    drawString.Prepend ("  ");

  if (IsSelected())
  {
    color.red   = 0;
    color.green = 0;
    color.blue  = 0;
  }

  father->SetHighColor (color);

  father->SetDrawingMode (B_OP_OVER);
  father->DrawString (drawString.String());
  father->SetDrawingMode (B_OP_COPY);
}

//////////////////////////////////////////////////////////////////////////////
/// End WindowListItem functions
//////////////////////////////////////////////////////////////////////////////

