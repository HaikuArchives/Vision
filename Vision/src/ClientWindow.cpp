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

#ifdef GNOME_BUILD
#  include "gnome/ScrollView.h"
#  include "gnome/Menu.h"
#  include "gnome/MenuItem.h"
#  include "gnome/MenuBar.h"
#  include "gnome/Rect.h"
#elif BEOS_BUILD
#  include <ScrollView.h>
#  include <Menu.h>
#  include <MenuItem.h>
#  include <MenuBar.h>
#  include <Rect.h>
#  include <MessageRunner.h>
#endif

#include <stdio.h>

#include "WindowList.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "Vision.h"
#include "SettingsFile.h"
#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ClientWindowDock.h"


//////////////////////////////////////////////////////////////////////////////
/// Begin BWindow functions
//////////////////////////////////////////////////////////////////////////////

ClientWindow::ClientWindow (BRect frame)
  : BWindow (
      frame,
      "Vision",
      B_DOCUMENT_WINDOW,
      B_ASYNCHRONOUS_CONTROLS)
{
  Init();
}

bool
ClientWindow::QuitRequested (void)
{
  if (!shutdown_in_progress)
  {
    shutdown_in_progress = true;
    BMessage killMsg (M_CLIENT_QUIT);
    killMsg.AddBool ("vision:winlist", true);
    killMsg.AddBool ("vision:shutdown_in_progress", shutdown_in_progress);
    if (ServerBroadcast (&killMsg))
      wait_for_quits = true;
      
    if (wait_for_quits)
      return false;
  }
  else
  {
    for (int32 o (1); o <= pWindowList()->CountItems(); ++o)
    { 
      WindowListItem *aitem ((WindowListItem *)pWindowList()->ItemAt (o - 1));
      if (aitem->Type() == WIN_SERVER_TYPE)
      {
        // wait some more
        return false;
      }
    }
  }


  vision_app->SetRect ("clientWinRect", Frame());
  
  BMessage killMeNow (B_QUIT_REQUESTED);
  killMeNow.AddBool ("real_thing", true);  
  vision_app->PostMessage (&killMeNow);

  return true;
}


void
ClientWindow::ScreenChanged (BRect screenframe, color_space mode)
{

  // user might have lowered resolution, may need to
  // move the window to a visible pos.

  if (Frame().top > screenframe.bottom)
    MoveTo (Frame().left, 110); // move up
  if (Frame().left > screenframe.right)
    MoveTo (110, Frame().top);  // move left
    
  BWindow::ScreenChanged (screenframe, mode);

}

void
ClientWindow::DispatchMessage (BMessage *msg, BHandler *handler)
{
  switch (msg->what)
  {
    case M_SEND_TO_AGENT:
      {
        uint32 newWhat;
        BView *target;

        if (msg->FindInt32 ("covertops", ((int32 *) &newWhat)) != B_OK)
        {
          printf (":ERROR: no valid covertops found in M_SEND_TO_AGENT, bailing...\n");
          return;
        }
        if (msg->FindPointer ("pointer", reinterpret_cast<void **>(&target)) != B_OK)
        {
          printf (":ERROR: no valid pointer found in M_SEND_TO_AGENT, bailing...\n");
          return;
        }

        msg->what = newWhat;
        target->MessageReceived (msg);
      }
      break;
    
    default:
      BWindow::DispatchMessage (msg, handler);
  }
}

void
ClientWindow::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_MOVE_DOWN:
      {
        pWindowList()->Select (pWindowList()->CurrentSelection() + 1);
        pWindowList()->ScrollToSelection();
      }
      break;
      
    case M_MOVE_UP:
      {
        pWindowList()->Select (pWindowList()->CurrentSelection() - 1);
        pWindowList()->ScrollToSelection();
      }
      break;

    case M_MOVE_UP_SHIFT:
      {
        int32 currentsel (pWindowList()->CurrentSelection());
        if (currentsel < 0)
          break;
          
        WindowListItem *aitem;
        bool foundone (false);
        int iloop;
        
        // try to find a WIN_NICK_BIT item first
        for (iloop = currentsel; iloop > -1; --iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_NICK_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;        
        
        // try to find a WIN_NEWS_BIT item
        for (iloop = currentsel; iloop > -1; --iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_NEWS_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;

        // try to find a WIN_PAGESIX_BIT item
        for (iloop = currentsel; iloop > -1; --iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_PAGESIX_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;
          
        // just select the previous item then.
        pWindowList()->Select (currentsel - 1);
        pWindowList()->ScrollToSelection();        
      }
      break;

    case M_MOVE_DOWN_SHIFT:
      {
        int32 currentsel (pWindowList()->CurrentSelection());
        if (currentsel < 0)
          break;
          
        WindowListItem *aitem;
        bool foundone (false);
        int iloop;
        
        // try to find a WIN_NICK_BIT item first
        for (iloop = currentsel; iloop < pWindowList()->CountItems(); ++iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_NICK_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;        
        
        // try to find a WIN_NEWS_BIT item
        for (iloop = currentsel; iloop < pWindowList()->CountItems(); ++iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_NEWS_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;

        // try to find a WIN_PAGESIX_BIT item
        for (iloop = currentsel; iloop < pWindowList()->CountItems(); ++iloop)
        {
          aitem = (WindowListItem *)pWindowList()->ItemAt (iloop);
          if ((aitem->Status() == WIN_PAGESIX_BIT))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            foundone = true;
            break; 
          }
        }
        
        if (foundone)
          break;
          
        // just select the previous item then.
        pWindowList()->Select (currentsel + 1);
        pWindowList()->ScrollToSelection();        
      }
      break;
    
    case M_MOVE_TOP_SERVER:
      {
        int32 currentsel (pWindowList()->CurrentSelection());
        if (currentsel < 0)
          break;
      
        int32 currentsid;
      
        WindowListItem *citem ((WindowListItem *)pWindowList()->ItemAt (currentsel));
      
        if (citem)
          currentsid = citem->Sid();
        else
          break;
      
        for (int32 i (1); i <= pWindowList()->CountItems(); ++i)
        { 
          WindowListItem *aitem ((WindowListItem *)pWindowList()->ItemAt (i - 1));
          if ((aitem->Type() == WIN_SERVER_TYPE) && (aitem->Sid() == currentsid))
          {
            pWindowList()->Select (pWindowList()->IndexOf (aitem));
            break; 
          }
        }
      }
      break;
    
    case M_UPDATE_STATUS:
      {
         WindowListItem *item;
         int32 newstatus;
      
         if ((msg->FindPointer ("item", reinterpret_cast<void **>(&item)) != B_OK)
         || (msg->FindInt32 ("status", ((int32 *)&newstatus)) != B_OK))
         {
           printf (":ERROR: no valid pointer and int found in M_UPDATE_STATUS, bailing...\n");
           return;
         }
       
         if (!pWindowList()->HasItem (item))
           return;
       
         if (msg->HasBool("hidden"))
         {
           item->SetStatus (newstatus);
           return;
         }
         
         if (item->Status() >= newstatus)
           return;
         else
           item->SetStatus (newstatus);
       } 
       break;
    
    case M_STATUS_CLEAR:
      {
        status->Clear();
      }
      break;
    
    case M_OBITUARY:
      {
        WindowListItem *agentitem;
        BView *agentview;
        if ((msg->FindPointer ("agent", reinterpret_cast<void **>(&agentview)) != B_OK)
        || (msg->FindPointer ("item", reinterpret_cast<void **>(&agentitem)) != B_OK))
        {
          printf (":ERROR: no valid pointers found in M_OBITUARY, bailing...\n");
          return;
        } 
       
        int32 agentType (agentitem->Type());
        pWindowList()->RemoveAgent (agentview, agentitem);
        
        if ((shutdown_in_progress) && (agentType == WIN_SERVER_TYPE))
          PostMessage (B_QUIT_REQUESTED);
      }  
      break;
 
    
    case M_CW_ALTW:
      {
        if (!altw_catch)
        {
           altw_catch = true;
           
           altwRunner = new BMessageRunner (
             this,
             new BMessage (M_CW_ALTW_RESET),
             400000, // 0.4 seconds
             1);           
        }
        else   
          PostMessage (B_QUIT_REQUESTED);
      }
      break;

    case M_CW_ALTW_RESET:
      {
        altw_catch = false;
        if (altwRunner)
          delete altwRunner;
      }
      break;
    
    case M_CW_ALTP:
      {
        pWindowList()->CloseActive();
      }
      break;
    
    case M_MAKE_NEW_SERVER:
      {
        const char *hostname, *port, *autoexec;
        bool enidentd;
      
        if ((msg->FindString ("hostname", &hostname) != B_OK)
        ||  (msg->FindString ("port", &port) != B_OK)
        ||  (msg->FindString ("autoexec", &autoexec) != B_OK)
        ||  (msg->FindBool   ("enidentd", &enidentd) != B_OK))
        {
          printf (":ERROR: recieved incomplete data to M_MAKE_NEW_SERVER -- bailing\n");
          return;
        }
       
        pWindowList()->AddAgent (
          new ServerAgent (
            const_cast<const char *>(hostname),
            const_cast<const char *>(port),
            enidentd,
            const_cast<const char *>(autoexec),
            *AgentRect()),
          ID_SERVER,
          hostname,
          WIN_SERVER_TYPE,
          true); // bring to front
      }
      break;
        
    default:
      BWindow::MessageReceived (msg);
  }
}

ServerAgent *
ClientWindow::GetTopServer (WindowListItem *request)
{
  int32 requestindex;
  int32 requestsid;

  if (pWindowList()->HasItem (request))
  {
    requestindex = pWindowList()->IndexOf (request);
    requestsid = request->Sid();
  }
  else
  {
    // can't find requesting agent in the Window List!
    // make sure you check for this NULL your calls to GetTopServer!
    return NULL;
  }
  
  for (int32 i (1); i <= pWindowList()->CountItems(); ++i)
  { 
    WindowListItem *aitem ((WindowListItem *)pWindowList()->ItemAt (i - 1));
    if ((aitem->Type() == WIN_SERVER_TYPE) && (aitem->Sid() == requestsid))
    {
      return (ServerAgent *)aitem->pAgent();
    }
  }
}

BRect *
ClientWindow::AgentRect (void)
{
  agentrect->left = cwDock->Frame().right - cwDock->Frame().left + 2;
  agentrect->top = Bounds().top + 1;
  agentrect->right = Bounds().Width() - 1;
  agentrect->bottom = cwDock->Frame().Height();
  return agentrect;
}


WindowList *
ClientWindow::pWindowList (void)
{
  return cwDock->pWindowList();
}


StatusView *
ClientWindow::pStatusView (void)
{
  return status;
}


bool
ClientWindow::ServerBroadcast (BMessage *outmsg_)
{
  bool reply (false);
  
  for (int32 i (1); i <= pWindowList()->CountItems(); ++i)
    { 
      WindowListItem *aitem ((WindowListItem *)pWindowList()->ItemAt (i - 1));
      if (aitem->Type() == WIN_SERVER_TYPE)
      {
        dynamic_cast<ServerAgent *>(aitem->pAgent())->msgr.SendMessage (outmsg_);
        reply = true;
      }
    }
    
  return reply;
}

void
ClientWindow::Show(void)
{
  // nothing yet
  BWindow::Show();
}

//////////////////////////////////////////////////////////////////////////////
/// End BWindow functions
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// Begin Private Functions
//////////////////////////////////////////////////////////////////////////////

void
ClientWindow::Init (void)
{
  SetSizeLimits (330,2000,150,2000);

  AddShortcut ('W', B_COMMAND_KEY, new BMessage(M_CW_ALTW));
 
  // logical nav shortcuts
  // (moved to Window menu below)

  // baxter-habit-friendly nav shortcuts
  AddShortcut (B_LEFT_ARROW, B_COMMAND_KEY, new BMessage (M_MOVE_UP));
  AddShortcut (B_RIGHT_ARROW, B_COMMAND_KEY, new BMessage (M_MOVE_DOWN));

  // bowser-habit-friendly nav shortcuts
  AddShortcut (',', B_COMMAND_KEY, new BMessage (M_MOVE_UP));
  AddShortcut ('.', B_COMMAND_KEY, new BMessage (M_MOVE_DOWN));
  
  AddShortcut (B_UP_ARROW, B_COMMAND_KEY && B_SHIFT_KEY, new BMessage (M_MOVE_UP_SHIFT));
  AddShortcut (B_DOWN_ARROW, B_COMMAND_KEY && B_SHIFT_KEY, new BMessage (M_MOVE_DOWN_SHIFT));

  AddShortcut (B_LEFT_ARROW, B_COMMAND_KEY && B_SHIFT_KEY, new BMessage (M_MOVE_UP_SHIFT));
  AddShortcut (B_RIGHT_ARROW, B_COMMAND_KEY && B_SHIFT_KEY, new BMessage (M_MOVE_DOWN_SHIFT));
   
  shutdown_in_progress = false;
  wait_for_quits = false;
  altw_catch = false;
  
  BRect frame (Bounds());
  menubar = new BMenuBar (frame, "menu_bar");
  
  BMenuItem *item;
  
  
  // Server menu
  mServer = new BMenu ("Server");
  mServer->AddItem (item = new BMenuItem ("Setup" B_UTF8_ELLIPSIS,
                    new BMessage (M_SETUP_SHOW), '/', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  mServer->AddItem (item = new BMenuItem ("Options" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'O'));
  item->SetTarget (vision_app);
  mServer->AddSeparatorItem();
  mServer->AddItem (item = new BMenuItem ("Channel List" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'L'));
  item->SetTarget (vision_app);
  mServer->AddItem (item = new BMenuItem ("Ignore List" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'I'));
  item->SetTarget (vision_app);
  mServer->AddItem (item = new BMenuItem ("Notify List" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'N'));
  item->SetTarget (vision_app);
  menubar->AddItem (mServer);
  
  
  // Edit menu
  mEdit = new BMenu ("Edit");
  menubar->AddItem (mEdit);
  
  // Tools menu
  mTools = new BMenu ("Tools");
  menubar->AddItem (mTools);
  
  // Window menu
  mWindow = new BMenu ("Window");
  mWindow->AddItem (item = new BMenuItem ("Close Window",
                    new BMessage (M_CW_ALTP), 'P'));
  mWindow->AddSeparatorItem();
  mWindow->AddItem (item = new BMenuItem ("Next Window",
                    new BMessage (M_MOVE_DOWN), B_DOWN_ARROW));
  mWindow->AddItem (item = new BMenuItem ("Previous Window",
                    new BMessage (M_MOVE_UP), B_UP_ARROW));
  mWindow->AddItem (item = new BMenuItem ("Server Window",
                    new BMessage (M_MOVE_TOP_SERVER), '/'));
  menubar->AddItem (mWindow);  
  
  AddChild (menubar);
  
  // add objects
  frame.top = menubar->Frame().bottom + 1;
  bgView = new BView (frame,
                      "Background",
                      B_FOLLOW_ALL_SIDES,
                      B_WILL_DRAW);

  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (bgView);
   

  frame = bgView->Bounds();

  status = new StatusView (frame);
  bgView->AddChild (status);
  
  status->AddItem (new StatusItem (
    "irc.elric.net", 0),
    true);
  
  cwDock = new ClientWindowDock (BRect (0, frame.top, 130, status->Frame().top - 1));
  
  bgView->AddChild (cwDock);

  agentrect = new BRect (
    cwDock->Frame().right - cwDock->Frame().left + 2,
    Bounds().top + 1,
    Bounds().Width() - 1,
    cwDock->Frame().Height());
}

//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////

