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
 *                 Jean-Baptiste M. Queru <jbq@be.com>
 *                 Seth Flaxman
 *                 Alan Ellis <void@be.com>
 */
 

#  include <PopUpMenu.h>
#  include <MenuItem.h>
#  include <List.h>

#include "Vision.h"
#include "WindowList.h"
#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ServerAgent.h"
#include "ListAgent.h"

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
  textColor   = vision_app->GetColor (C_WINLIST_NORMAL);
  newsColor   = vision_app->GetColor (C_WINLIST_NEWS);
  nickColor   = vision_app->GetColor (C_WINLIST_NICK);
  sixColor    = vision_app->GetColor (C_WINLIST_PAGESIX);
  selColor    = vision_app->GetColor (C_WINLIST_SELECTION);
  bgColor     = vision_app->GetColor (C_WINLIST_BACKGROUND);
  
  BListView::SetFont (vision_app->GetClientFont (F_WINLIST));

  SetViewColor (bgColor);
  
  SetTarget (this);
}

WindowList::~WindowList (void)
{
  //
}

void
WindowList::AllAttached (void)
{
  //
}

void
WindowList::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_MENU_NUKE:
      {
        CloseActive();
      }
      break;
    
    default:
      BListView::MessageReceived (msg);
  }
}

void
WindowList::CloseActive (void)
{
  WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (CurrentSelection())));
  if (myItem)
  {
    BMessage killMsg (M_CLIENT_QUIT);
    killMsg.AddBool ("vision:winlist", true);
        
    BView *killTarget (myItem->pAgent());
        
    if (dynamic_cast<ClientAgent *>(killTarget))
      dynamic_cast<ClientAgent *>(killTarget)->msgr.SendMessage (&killMsg);
    else if (dynamic_cast<ListAgent *>(killTarget))
      dynamic_cast<ListAgent *>(killTarget)->msgr.SendMessage(&killMsg);
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

      BuildPopUp();
      myPopUp->Go (
        ConvertToScreen (myPoint),
        true,
        false,
        ConvertToScreen (ItemFrame (selected)),
        true);
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

  if (!handled)
    BListView::MouseDown (myPoint);
}

void 
WindowList::KeyDown (const char *, int32) 
{
  // TODO: WindowList never gets keyboard focus (?)
  // if you have to uncomment this, either fix WindowList so it
  // doesn't retain keyboard focus, or update this code to work
  // like IRCView::KeyDown()  --wade 20010506
  #if 0
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
  #endif
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
  // TODO: should we maybe be locking first on these invalidates?
  
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
       }
       break;

    case C_WINLIST_NICK:
      {
         nickColor = color;

         for (int32 i = 0; i < CountItems(); ++i)
         {
           WindowListItem *item ((WindowListItem *)ItemAt (i));

           if ((item->Status() & WIN_NICK_BIT) != 0)
             InvalidateItem (i);
         }
       }
       break;

    case C_WINLIST_NORMAL:
      {
         textColor = color;

         for (int32 i = 0; i < CountItems(); ++i)
         {
           WindowListItem *item ((WindowListItem *)ItemAt (i));
           int32 mask (WIN_NICK_BIT | WIN_PAGESIX_BIT | WIN_NEWS_BIT);
           if ((item->Status() & mask) == 0)
             InvalidateItem (i);
         }
       }
       break;
       
    case C_WINLIST_PAGESIX:
      {
         sixColor = color;

         for (int32 i = 0; i < CountItems(); ++i)
         {
           WindowListItem *item ((WindowListItem *)ItemAt (i));

           if ((item->Status() & WIN_PAGESIX_BIT) != 0)
             InvalidateItem (i);
         }
       }
       break;

    case C_WINLIST_SELECTION:
      {
         selColor = color;
         InvalidateItem (CurrentSelection());
      }
      break;

    case C_WINLIST_BACKGROUND:
      {
         bgColor = color;
         SetViewColor (bgColor);
         Invalidate();
      }
      break;    
  }
}

rgb_color
WindowList::GetColor (int32 which) const
{
  rgb_color color (textColor);

  if (which == C_WINLIST_NEWS)
    color = newsColor;

  else if (which == C_WINLIST_NICK)
    color = nickColor;
    
  else if (which == C_WINLIST_PAGESIX)
    color = sixColor;

  else if (which == C_WINLIST_BACKGROUND)
    color = bgColor;
  
  else if (which == C_WINLIST_SELECTION)
    color = selColor;

  return color;
}


void
WindowList::SetFont (int32 which, const BFont *font)
{
  if (which == F_WINLIST)
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

void
WindowList::SelectLast (void)
{
  /*
   * Function purpose: Select the last active agent
   */

  LockLooper();
  int32 lastInt (IndexOf (lastSelected));
  if (lastInt >= 0)
    Select (lastInt);
  else
    Select (0);
    
  ScrollToSelection();
  UnlockLooper();
  
}

void
WindowList::SelectServer (void)
{
  int32 currentsel (CurrentSelection());
  if (currentsel < 0)
    return;
      
  int32 currentsid;
       
  WindowListItem *citem ((WindowListItem *)ItemAt (currentsel));
      
  if (citem)
    currentsid = citem->Sid();
  else
    return;
      
  for (int32 i (1); i <= CountItems(); ++i)
  { 
    WindowListItem *aitem ((WindowListItem *)ItemAt (i - 1));
    if ((aitem->Type() == WIN_SERVER_TYPE) && (aitem->Sid() == currentsid))
    {
      Select (IndexOf (aitem));
      break; 
    }
  }
}

void
WindowList::ContextSelectUp (void)
{
  int32 currentsel (CurrentSelection());
  if (currentsel < 0)
    return;
          
  WindowListItem *aitem;
  bool foundone (false);
  int iloop;
        
  // try to find a WIN_NICK_BIT item first
  for (iloop = currentsel; iloop > -1; --iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_NICK_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;        
        
  // try to find a WIN_NEWS_BIT item first
  for (iloop = currentsel; iloop > -1; --iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_NEWS_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;

  // try to find a WIN_PAGESIX_BIT item first
  for (iloop = currentsel; iloop > -1; --iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_PAGESIX_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;
          
  // just select the previous item then.
  Select (currentsel - 1);
  ScrollToSelection();        
}

void
WindowList::ContextSelectDown (void)
{
  int32 currentsel (CurrentSelection());
  if (currentsel < 0)
    return;
          
  WindowListItem *aitem;
  bool foundone (false);
  int iloop;
        
  // try to find a WIN_NICK_BIT item first
  for (iloop = currentsel; iloop < CountItems(); ++iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_NICK_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;        
        
  // try to find a WIN_NEWS_BIT item first
  for (iloop = currentsel; iloop < CountItems(); ++iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_NEWS_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;   

  // try to find a WIN_PAGESIX_BIT item first
  for (iloop = currentsel; iloop < CountItems(); ++iloop)
  {
    aitem = (WindowListItem *)ItemAt (iloop);
    if ((aitem->Status() == WIN_PAGESIX_BIT))
    {
      Select (IndexOf (aitem));
      foundone = true;
      break; 
    }
  }
        
  if (foundone)
    return;   
          
  // just select the previous item then.
  Select (currentsel + 1);
  ScrollToSelection();      
}

ClientAgent *
WindowList::Agent (int32 serverId, const char *aName)
{
  ClientAgent *agent (0);

  for (int32 i = 0; i < CountItems(); ++i)
  {
    WindowListItem *item ((WindowListItem *)ItemAt (i));
    if (item->Sid() == serverId)
    {
      if (dynamic_cast<ClientAgent *>(item->pAgent()))
      {
        if (strcasecmp (aName, reinterpret_cast<ClientAgent *>(item->pAgent())->Id().String()) == 0)
        {
          agent = reinterpret_cast<ClientAgent *>(item->pAgent());
          break;      
        }
      }
      else if (dynamic_cast<ListAgent *>(item->pAgent()) && !strcmp(aName, "Channels"))
      {
          agent = reinterpret_cast<ClientAgent *>(item->pAgent());
          break;
      }
    }
  }
  return agent;
}

/*
  -- #beos was here --
  <kurros> main toaster turn on!
  <Scott> we get toast
  <Scott> all your butter are belong to us
  <bullitB> someone set us up the jam!
  <Scott> what you say
  <bullitB> you have no chance make your breakfast
  <bullitB> ha ha ha
*/

void
WindowList::AddAgent (BView *agent, int32 serverId, const char *name, int32 winType, bool activate)
{
  int32 itemindex;
  WindowListItem *currentitem ((WindowListItem *)ItemAt (CurrentSelection()));
  
  WindowListItem *newagentitem (new WindowListItem (name, serverId, winType, WIN_NORMAL_BIT, agent));
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
  
  // reset newagent
  newagent = newagentitem->pAgent();
  
  if (dynamic_cast<ListAgent *>(newagent))
  {
     for (int32 i = 0; i < CountItems(); ++i)
     {
       WindowListItem *item = (WindowListItem *)ItemAt (i);
       if ((item->Sid() == serverId) && (item->Name().ICompare("Channels") == 0))
       {
         dynamic_cast<ListAgent *>(newagent)->agentWinItem = item;
         break;
       }
     }
  }
  
  // reset newagent again
  newagent = newagentitem->pAgent();
  
  if (!(newagent = dynamic_cast<BView *>(newagent)))
  {
    // stop crash
    printf ("no newagent!?\n");
    return;
  }
  
  LockLooper();
  vision_app->pClientWin()->bgView->AddChild (newagent);
  newagent->Hide(); // get it out of the way
  newagent->Sync(); // clear artifacts
  UnlockLooper();
  
  if (activate)  // if activate is true, show the new view now.
    if (CurrentSelection() == -1)
      Select (itemindex); // first item, let SelectionChanged() activate it
    else
      Activate (itemindex);
  else
    Select (IndexOf (currentitem));

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
      lastSelected = aitem;
      break;
    }
  }
  
  if (!(newagent = dynamic_cast<BView *>(newagent)))
  {
    // stop crash
    printf ("no newagent!?\n");
    return;
  }
  
   
  if ((activeagent != newagent) && (activeagent != 0))
  {
    LockLooper();

    newagent->Show();
    
    if (activeagent)
    {
      activeagent->Hide(); // you arent wanted anymore!
      activeagent->Sync(); // and take your damned pixels with you!
    }
    
    UnlockLooper();
  }
  if (activeagent == 0)
  {
    LockLooper();
    newagent->Show();
    UnlockLooper();
  }
 
  // activate the input box (if it has one)
  if ( (newagent = dynamic_cast<ClientAgent *>(newagent)) )
    reinterpret_cast<ClientAgent *>(newagent)->msgr.SendMessage (M_INPUT_FOCUS);

  // set ClientWindow's title
  BString agentid;
  agentid += newagentitem->Name().String();
  agentid.Append (" - Vision");
  vision_app->pClientWin()->SetTitle (agentid.String());
  
  Select (index);
}

void
WindowList::RemoveAgent (BView *agent, WindowListItem *agentitem)
{
  LockLooper();
  agent->Hide();
  agent->RemoveSelf();
  RemoveItem (agentitem);
  SortItems (SortListItems);
  delete agent;
  
  SelectLast();
  UnlockLooper();
}


int
WindowList::SortListItems (const void *name1, const void *name2)
{
  WindowListItem *firstPtr (*(WindowListItem * const *)name1);
  WindowListItem *secondPtr (*(WindowListItem * const *)name2);

  /* Not sure if this can happen, and we
   * are assuming that if one is NULL
   * we return them as equal.  What if one
   * is NULL, and the other isn't?
   */
  if (!firstPtr
  ||  !secondPtr)
  {
    return 0;
  }
  
  int32 firstSid, secondSid;
  BString firstName, secondName;
  
  firstSid = (firstPtr)->Sid();
  secondSid = (secondPtr)->Sid();
  
  firstName = (firstPtr)->Name();
  secondName = (secondPtr)->Name();
 
  if (firstSid < secondSid)
    return -1;
  else if ((firstPtr)->Sid() > (secondPtr)->Sid())
    return 1;
  else
  {
    // same sid, sort by name
    if ((firstPtr)->Type() == WIN_SERVER_TYPE)
      return -1;
    return firstName.ICompare (secondName);
  } 
}

void
WindowList::BuildPopUp (void)
{
  myPopUp = new BPopUpMenu("Window Selection", false, false);
  BMenuItem *item;
  
  WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (CurrentSelection())));
  if (myItem)
  {
    ClientAgent *activeagent (dynamic_cast<ClientAgent *>(myItem->pAgent()));
    if (activeagent)
      activeagent->AddMenuItems (myPopUp);
  }
  
  item = new BMenuItem("Close", new BMessage (M_MENU_NUKE));
  item->SetTarget (this);
  myPopUp->AddItem (item);
  
  myPopUp->SetFont (be_plain_font);
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
  vision_app->pClientWin()->Lock();
  myName = name;
  int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
  vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
  
  // if the agent is active, update ClientWindow's titlebar
  if (IsSelected())
  {
    BString agentid (name);
    agentid.Append (" - Vision");
    vision_app->pClientWin()->SetTitle (agentid.String());
  }
    
  vision_app->pClientWin()->Unlock();
}

void
WindowListItem::SetSid (int32 newSid)
{
  mySid = newSid;
}

void
WindowListItem::SetStatus (int32 winStatus)
{
  vision_app->pClientWin()->Lock();
  myStatus = winStatus;
  int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
  vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
  vision_app->pClientWin()->Unlock();
}

void
WindowListItem::ActivateItem (void)
{
  int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
  vision_app->pClientWin()->pWindowList()->Activate (myIndex);
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
  rgb_color color = father->GetColor (C_WINLIST_NORMAL);

  if ((myStatus & WIN_NEWS_BIT) != 0)
    color = father->GetColor (C_WINLIST_NEWS);

  else if ((myStatus & WIN_PAGESIX_BIT) != 0)
    color = father->GetColor (C_WINLIST_PAGESIX);

  else if ((myStatus & WIN_NICK_BIT) != 0)
    color = father->GetColor (C_WINLIST_NICK);

  if ((myType & WIN_CHANNEL_TYPE) != 0)
    drawString.Prepend ("  ");

  if ((myType & WIN_MESSAGE_TYPE) != 0)
    drawString.Prepend ("  ");

  if (IsSelected())
    color = father->GetColor (C_WINLIST_NORMAL);
  
  father->SetHighColor (color);

  father->SetDrawingMode (B_OP_OVER);
  father->DrawString (drawString.String());
  father->SetDrawingMode (B_OP_COPY);
}

//////////////////////////////////////////////////////////////////////////////
/// End WindowListItem functions
//////////////////////////////////////////////////////////////////////////////

