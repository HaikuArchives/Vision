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

#include <ScrollView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <Rect.h>
#include <MessageRunner.h>
#include <Roster.h>

#include <stdio.h>

#include "NetworkMenu.h"
#include "WindowList.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "ResizeView.h"
#include "Vision.h"
#include "SettingsFile.h"
#include "ClientWindow.h"
#include "ChannelAgent.h"
#include "ClientAgent.h"
#include "ClientWindowDock.h"
#include "ListAgent.h"
#include "Names.h"


/*
  -- #beos was here --
  <Electroly> kurros has a l33t ass projector, I saw a picture of it just once
              and I've been drooling ever since
  <T-ball> nice...
  <T-ball> I don't have room for a projector... :/
  <Brazilian> I have a monkey who draws on my wall really fast
*/

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
  vision_app->SetRect ("windowDockRect", cwDock->Bounds());
  vision_app->SetRect ("clientWinRect", Frame());
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


  vision_app->PostMessage (B_QUIT_REQUESTED);
  delete_sem(shutdownSem);

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
ClientWindow::HandleKey (BMessage *keyMsg)
{
  const char *bytes;
  int32 mod;
  
  keyMsg->FindString ("bytes", &bytes);
  keyMsg->FindInt32 ("modifiers", &mod);

  if ((mod & B_OPTION_KEY)  == 0
  &&  (mod & B_COMMAND_KEY) != 0
  &&  (mod & B_CONTROL_KEY) == 0
  &&  (mod & B_SHIFT_KEY) != 0)
  {
    /////////////////////
    /// Shift+Command ///
    /////////////////////
    switch (bytes[0])
    {
      case '0':
      case B_INSERT:
        // switch to last active agent
        pWindowList()->SelectLast();
        break;
      
      case B_UP_ARROW:
      case B_LEFT_ARROW: // baxter muscle memory
      case ',': // bowser muscle memory
        pWindowList()->ContextSelectUp();
        break;
      
      case B_DOWN_ARROW: //
      case B_RIGHT_ARROW: // baxter muscle memory
      case '.': // bowser muscle memory
        pWindowList()->ContextSelectDown();
        break;
    }
  }

  else if ((mod & B_OPTION_KEY)  == 0
       &&  (mod & B_COMMAND_KEY) != 0
       &&  (mod & B_CONTROL_KEY) == 0
       &&  (mod & B_SHIFT_KEY) == 0)
  {
    ///////////////
    /// Command ///
    ///////////////
    switch (bytes[0])
    {
      case B_UP_ARROW:
      case ',': // bowser muscle memory
        // move up one agent
        pWindowList()->Select (pWindowList()->CurrentSelection() - 1);
        pWindowList()->ScrollToSelection();
        break;

      case B_DOWN_ARROW:
      case '.': // bowser muscle memory
        // move down one agent
        pWindowList()->Select (pWindowList()->CurrentSelection() + 1);
        pWindowList()->ScrollToSelection();
        break;
        
      case B_LEFT_ARROW: // collapse current server (if expanded)
        pWindowList()->CollapseCurrentServer();
        break;

      case B_RIGHT_ARROW: // expand current server (if collapsed)
        pWindowList()->ExpandCurrentServer();
        break;

      case '/': // bowser muscle memory
        // move to the agents parent ServerAgent
        // XXX move to WindowList ?
        pWindowList()->SelectServer();
        break;
    }
  }
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
    
    case B_KEY_DOWN:
      {
        HandleKey (msg);
        BWindow::DispatchMessage (msg, handler);
      }
      break;
    
    default:
      BWindow::DispatchMessage (msg, handler);
  }
}

void
ClientWindow::AddMenu (BMenu *menu)
{
  if (menu != NULL)
    menubar->AddItem (menu);
}

void
ClientWindow::RemoveMenu (BMenu *menu)
{
  if (menu != NULL)
    menubar->RemoveItem (menu);
}

void
ClientWindow::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    
    case M_UPDATE_STATUS:
      {
         WindowListItem *item (NULL),
                         *superItem (NULL);
         int32 newstatus;
      
         if ((msg->FindPointer ("item", reinterpret_cast<void **>(&item)) != B_OK)
         || (msg->FindInt32 ("status", ((int32 *)&newstatus)) != B_OK))
         {
           printf (":ERROR: no valid pointer and int found in M_UPDATE_STATUS, bailing...\n");
           return;
         }
         
         superItem = (WindowListItem *)pWindowList()->Superitem(item);

         if (superItem && (!superItem->IsExpanded()))
         {
           int32 srvstatus ((superItem->SubStatus()));
           if (srvstatus < newstatus)
             superItem->SetSubStatus (newstatus);
         }
         
         if (!pWindowList()->FullListHasItem (item))
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
        if (!altw_catch && vision_app->GetBool ("catchAltW"))
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
    
    case M_STATE_CHANGE:
      {
        ServerBroadcast (msg);
      }
      break;
      
    case M_MAKE_NEW_NETWORK:
      {
        BMessage network;
        msg->FindMessage ("network", &network);
        BString netName (network.FindString ("name"));
        pWindowList()->AddAgent (
          new ServerAgent (netName.String(),
            network,
            *AgentRect()),
          ID_SERVER,
          netName.String(),
          WIN_SERVER_TYPE,
          true); // bring to front
      }
      break;
    
    case M_RESIZE_VIEW:
      {
        BView *view (NULL);
        msg->FindPointer ("view", reinterpret_cast<void **>(&view));
        WindowListItem *item ((WindowListItem *)pWindowList()->ItemAt (pWindowList()->CurrentSelection()));
        BView *agent (item->pAgent());
        if (dynamic_cast<ClientWindowDock *>(view))
        {
          BPoint point;
          msg->FindPoint ("loc", &point);
          resize->MoveTo (point.x, resize->Frame().top);
          cwDock->ResizeTo (point.x - 1, cwDock->Frame().Height());
          BRect *agRect (AgentRect());
          if (agent)
          {
            agent->ResizeTo (agRect->Width(), agRect->Height());
            agent->MoveTo (agRect->left, agRect->top);
          }
        }
        else
            DispatchMessage (msg, agent);
      }
      break;
    
    case M_OPEN_TERM:
      be_roster->Launch ("application/x-vnd.Be-SHEL", 0, NULL);
      break;
    
    case M_LIST_COMMAND:
    {
      WindowListItem *item ((WindowListItem *)pWindowList()->ItemAt (pWindowList()->CurrentSelection()));
      BView *view (NULL);
      if (item)
        view = item->pAgent();
      if ((view == NULL) || (dynamic_cast<ListAgent *>(view) != NULL))
        break;
      dynamic_cast<ClientAgent *>(view)->ParseCmd ("/LIST");;
    }
    break;

        
    default:
      BWindow::MessageReceived (msg);
  }
}

ServerAgent *
ClientWindow::GetTopServer (WindowListItem *request)
{
  ServerAgent *target (NULL);
  if (pWindowList()->FullListHasItem (request))
  {
    // if the item has no super, it is a server agent, return it.
    if (pWindowList()->Superitem(request) == NULL)
      target = dynamic_cast<ServerAgent *>(request->pAgent());
    else
      target = dynamic_cast<ServerAgent *>(((WindowListItem *)pWindowList()->Superitem(request))->pAgent());
  }
  return target;
}

BRect *
ClientWindow::AgentRect (void)
{
  agentrect->left = resize->Frame().right - cwDock->Frame().left + 1;
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

/*
  -- #beos was here --
  <AnEvilYak> regurg, you don't like my monkey??
  <regurg> Oh, I don't like your monkey?
  <AnEvilYak> regurg: no :( *sob*
  <regurg> Why 'no'?
  <AnEvilYak> regurg: you tell me
  <regurg> What makes you think I tell you?
*/

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

  shutdown_in_progress = false;
  wait_for_quits = false;
  altw_catch = false;
  shutdownSem = vision_app->GetShutdownSem();

  AddShortcut ('W', B_COMMAND_KEY, new BMessage(M_CW_ALTW));
  
  BRect frame (Bounds());
  menubar = new BMenuBar (frame, "menu_bar");
  
  BMenuItem *item;
  BMenu *menu;
  // Server menu
  mServer = new BMenu ("Server");
  mServer->AddItem (menu = new NetworkMenu ("Connect to" B_UTF8_ELLIPSIS, M_CONNECT_NETWORK, BMessenger (vision_app)));
  
  mServer->AddItem (item = new BMenuItem ("Setup" B_UTF8_ELLIPSIS,
                    new BMessage (M_SETUP_SHOW), '/', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  menubar->AddItem (mServer);
  
  
  // Edit menu
  mEdit = new BMenu ("Edit");
  menubar->AddItem (mEdit);
  
  // Tools menu
  mTools = new BMenu ("Tools");

  mTools->AddItem (item = new BMenuItem ("Channel List" B_UTF8_ELLIPSIS,
                    new BMessage (M_LIST_COMMAND), 'L'));
  
  mTools->AddItem (item = new BMenuItem ("Ignore List" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'I'));
  item->SetTarget (vision_app);
  mTools->AddItem (item = new BMenuItem ("Notify List" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'N'));
  item->SetTarget (vision_app);
  
  mTools->AddItem (item = new BMenuItem ("New Terminal", new BMessage (M_OPEN_TERM),
                    'T', B_OPTION_KEY));


  menubar->AddItem (mTools);
  
  // Window menu
  mWindow = new BMenu ("Window");
  
  mWindow->AddItem (item = new BMenuItem ("Part Agent", new BMessage (M_CW_ALTP), 'P'));
  
  item->SetTarget (this);
  menubar->AddItem (mWindow);  
  
  AddChild (menubar);
  
  // add objects
  frame.top = menubar->Frame().bottom + 1;
  bgView = new BView (frame,
                      "Background",
                      B_FOLLOW_ALL_SIDES,
                      0);

  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (bgView);
   

  frame = bgView->Bounds();

  status = new StatusView (frame);
  bgView->AddChild (status);
  
  status->AddItem (new StatusItem (
    "irc.elric.net", 0),
    true);
  
  BRect cwDockRect (vision_app->GetRect ("windowDockRect"));
  cwDock = new ClientWindowDock (BRect (0, frame.top, (cwDockRect.Width() == 0.0) ? 130 : cwDockRect.Width(), status->Frame().top - 1));
  
  bgView->AddChild (cwDock);
  
  resize = new ResizeView (cwDock, BRect (cwDock->Frame().right + 1,
    Bounds().top + 1, cwDock->Frame().right + 3, status->Frame().top - 1));
  
  bgView->AddChild (resize);

  agentrect = new BRect (
    (resize->Frame().right - cwDock->Frame().left) + 1,
    Bounds().top + 1,
    Bounds().Width() - 1,
    cwDock->Frame().Height());
}

//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////
