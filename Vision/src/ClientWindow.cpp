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
#endif

#include <stdio.h>

#include "WindowList.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "Vision.h"
#include "SettingsFile.h"
#include "ClientWindow.h"
#include "ClientAgent.h"


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
    
    if (ServerBroadcast (&killMsg))
      wait_for_quits = true;
      
    if (wait_for_quits)
      return false;
  }
  else
  {
    for (int32 o (1); o <= winList->CountItems(); ++o)
    { 
      WindowListItem *aitem ((WindowListItem *)winList->ItemAt (o - 1));
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
      break;
    }
    
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
      winList->Select (winList->CurrentSelection() + 1);
      winList->ScrollToSelection();
      break;
    }
    
    case M_MOVE_UP:
    {
      winList->Select (winList->CurrentSelection() - 1);
      winList->ScrollToSelection();
      break;
    }
    
    case M_MOVE_TOP_SERVER:
    {
      
      int32 currentsel (winList->CurrentSelection());
      if (currentsel < 0)
        break;
      
      int32 currentsid;
      
      WindowListItem *citem ((WindowListItem *)winList->ItemAt (currentsel));
      
      if (citem)
        currentsid = citem->Sid();
      else
        break;
      
      for (int32 i (1); i <= winList->CountItems(); ++i)
      { 
        WindowListItem *aitem ((WindowListItem *)winList->ItemAt (i - 1));
        if ((aitem->Type() == WIN_SERVER_TYPE) && (aitem->Sid() == currentsid))
        {
          winList->Select (winList->IndexOf (aitem));
          break; 
        }
      }
      break;
    }
    
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
       
       if (!winList->HasItem (item))
         break;
       
       if (msg->HasBool("hidden"))
       {
       	 item->SetStatus (newstatus);
         break;
       }
           
       if (item->Status() != WIN_NICK_BIT)
       {
       	 item->SetStatus (newstatus);
       }
       
       break;
    }
    
    case M_STATUS_CLEAR:
    {
      status->Clear();
      break;
    }
    
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
       winList->RemoveAgent (agentview, agentitem);
       
       if ((shutdown_in_progress) && (agentType == WIN_SERVER_TYPE))
         PostMessage (B_QUIT_REQUESTED);
       
       break;
    }
    
    case M_CW_ALTW:
    {
      if (vision_app->GetBool("catchAltW"))
        printf (":TODO: Alert box baby!\n");
      else
        PostMessage (B_QUIT_REQUESTED);
      break;
    }
    
    case M_CW_ALTP:
    {
      winList->CloseActive();
      break;
    }
    
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
      
      winList->AddAgent (
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
      
      break;
    }
        
    default:
      BWindow::MessageReceived (msg);
  }
}

BRect *
ClientWindow::AgentRect (void)
{
  agentrect->left = winListScroll->Frame().right - winListScroll->Frame().left + 1;
  agentrect->top = Bounds().top + 1;
  agentrect->right = Bounds().Width() - 1;
  agentrect->bottom = winListScroll->Frame().Height();
  return agentrect;
}


WindowList *
ClientWindow::pWindowList (void)
{
  return winList;
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
  
  for (int32 i (1); i <= winList->CountItems(); ++i)
    { 
      WindowListItem *aitem ((WindowListItem *)winList->ItemAt (i - 1));
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
   
  shutdown_in_progress = false;
  wait_for_quits = false;
  
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
    
  winList = new WindowList (BRect (0, frame.top, 100, status->Frame().top - 1));
  
  winListScroll = new BScrollView (
		"winListScroll",
		winList,
		B_FOLLOW_TOP_BOTTOM,
		0,
		false,
		true,
		B_NO_BORDER);
  bgView->AddChild (winListScroll);
  
  agentrect = new BRect (
    winListScroll->Frame().right - winListScroll->Frame().left + 1,
    Bounds().top + 1,
    Bounds().Width() - 1,
    winListScroll->Frame().Height());
}

//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////

