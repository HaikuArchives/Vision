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

#include <ScrollView.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuBar.h>
#include <Rect.h>

// :TODO: wade 020201: remove once serverwindow is spawning serveragents.
#include <List.h>

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


ClientWindow::~ClientWindow (void)
{
 //
}


bool
ClientWindow::QuitRequested (void)
{
  vision_app->visionSettings->ReplaceRect ("clientWinRect", Frame());

  vision_app->PostMessage (B_QUIT_REQUESTED);

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
    case M_UPDATE_STATUS:
    {
      WindowListItem *item;
      int32 newstatus;
      bool hidden (true);
      
      if (msg->FindPointer ("item", reinterpret_cast<void **>(&item)) != B_OK)
      {
        printf (":ERROR: no valid pointer found in M_UPDATE_STATUS, bailing...\n");
        return;
      }

      if (msg->FindInt32 ("status", ((int32 *) &newstatus)) != B_OK)
      {
        printf (":ERROR: no valid newstatus found in M_UPDATE_STATUS, bailing...\n");
        return;
      }
      
      msg->FindBool ("hidden", &hidden);
      
      if (!winList->HasItem (item))
        break;

      if (!hidden)
      {
        item->SetStatus (newstatus);
        winList->Invalidate();
        break;
      }
            
      if (item->Status() != WIN_NICK_BIT)
      {
        item->SetStatus (newstatus);
        winList->Invalidate();
      }
      
      break;
    }
        
    default:
      BWindow::MessageReceived (msg);
  }
}

void
ClientWindow::UpdateAgentRect (void)
{
  agentrect->left = winListScroll->Frame().right - winListScroll->Frame().left + 1;
  agentrect->top = Bounds().top + 1;
  agentrect->right = Bounds().Width() - 1;
  agentrect->bottom = winListScroll->Frame().Height();
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
  
  BRect frame (Bounds());
  menubar = new BMenuBar (frame, "menu_bar");
  
  BMenuItem *item;
  //BMessage *msg;
  
  
  // Server menu
  mServer = new BMenu ("Server");
  mServer->AddItem (item = new BMenuItem ("Setup" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), '/', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  mServer->AddItem (item = new BMenuItem ("Preferences" B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'P'));
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
  mWindow->AddItem (item = new BMenuItem ("Next Window",
                    new BMessage (B_ABOUT_REQUESTED), ','));
  item->SetTarget (vision_app);
  mWindow->AddItem (item = new BMenuItem ("Previous Window",
                    new BMessage (B_ABOUT_REQUESTED), '.'));
  item->SetTarget (vision_app);
  mWindow->AddItem (item = new BMenuItem ("Next Server",
                    new BMessage (B_ABOUT_REQUESTED), ',', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  mWindow->AddItem (item = new BMenuItem ("Previous Server",
                    new BMessage (B_ABOUT_REQUESTED), '.', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  mWindow->AddSeparatorItem();
  mWindow->AddItem (item = new BMenuItem ("Server",
                    new BMessage (B_ABOUT_REQUESTED), '/'));
  item->SetTarget (vision_app);
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
    
    
  // :TODO: wade 020201: make dynamic
  // most of this stuff is temporary, to get us going.
  // eventually we will have setupwindow which will pass all
  // this stuff.
  BList *nicklist (new BList);
  BString nick ("vision");
  nicklist->AddItem (strcpy (new char [nick.Length() + 1], nick.String()));
  
  BString *serverhost (new BString ("irc.inetking.com")),
          *serverport (new BString ("6667")),
          *username (new BString ("Vision User")),
          *userident (new BString ("vision")),
          *servercmds (new BString ("")),
          *events (vision_app->events);
          
  
  winList->AddAgent (
    new ServerAgent (
      serverhost->String(),
      nicklist,
      serverport->String(),
      username->String(),
      userident->String(),
      events,
      true,  // show motd
      true, // enable identd
      servercmds->String(),
      *agentrect),
    ID_SERVER,
    serverhost->String(),
    WIN_SERVER_TYPE,
    true); // activate
    
  
//  int32 x (0);
//  serverhost = new BString ("irc.inetking.com");
//
//  while (x <= 1)
//  {
//    winList->AddAgent (
//    new ServerAgent (
//      serverhost->String(),
//      nicklist,
//      serverport->String(),
//      username->String(),
//      userident->String(),
//      const_cast<const char **>(events),
//      true,  // show motd
//      false, // enable identd,
//      servercmds->String(),
//      *agentrect),
//    serverhost->String(),
//    false); // activate
//    x++;
//  }    
    
 
}

//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////

