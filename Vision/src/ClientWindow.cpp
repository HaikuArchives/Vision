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

#include "ChannelAgent.h"
#include "ClientAgent.h"
#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "ListAgent.h"
#include "Names.h"
#include "NetworkMenu.h"
#include "NotifyList.h"
#include "ResizeView.h"
#include "ServerAgent.h"
#include "SettingsFile.h"
#include "StatusView.h"
#include "Vision.h"
#include "VTextControl.h"
#include "WindowList.h"


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

ClientWindow::~ClientWindow (void)
{
  delete fAgentrect;
}

bool
ClientWindow::QuitRequested (void)
{
  if (!fShutdown_in_progress)
  {
    fShutdown_in_progress = true;
    BMessage killMsg (M_CLIENT_QUIT);
    killMsg.AddBool ("vision:winlist", true);
    killMsg.AddBool ("vision:shutdown", fShutdown_in_progress);

    if (ServerBroadcast (&killMsg))
      fWait_for_quits = true;
      
    if (fWait_for_quits)
      return false;
  }
  be_app_messenger.SendMessage (B_QUIT_REQUESTED);

  return true;
}

void
ClientWindow::FrameMoved (BPoint origin)
{
  BWindow::FrameMoved (origin);
  vision_app->SetRect ("windowDockRect", fCwDock->Bounds());
  vision_app->SetRect ("clientWinRect", Frame());
}

void
ClientWindow::FrameResized (float width, float height)
{
  BWindow::FrameResized (width, height);
  vision_app->SetRect ("windowDockRect", fCwDock->Bounds());
  vision_app->SetRect ("clientWinRect", Frame());
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
      
      case 'U':
        pWindowList()->MoveCurrentUp();
        break;
        
      case 'D':
        pWindowList()->MoveCurrentDown();
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
    fMenuBar->AddItem (menu);
}

void
ClientWindow::RemoveMenu (BMenu *menu)
{
  if (menu != NULL)
    fMenuBar->RemoveItem (menu);
}

void
ClientWindow::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    
    case M_CW_UPDATE_STATUS:
      {
         WindowListItem *item (NULL),
                         *superItem (NULL);
         int32 newstatus;
      
         if ((msg->FindPointer ("item", reinterpret_cast<void **>(&item)) != B_OK)
         || (msg->FindInt32 ("status", ((int32 *)&newstatus)) != B_OK))
         {
           printf (":ERROR: no valid pointer and int found in M_CW_UPDATE_STATUS, bailing...\n");
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
        fStatus->Clear();
        SetEditStates();
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
        
        pWindowList()->RemoveAgent (agentview, agentitem);

        if (fShutdown_in_progress && pWindowList()->CountItems() == 0)
          PostMessage (B_QUIT_REQUESTED);
          
      }  
      break;
 
    
    case M_CW_ALTW:
      {
        if (!fAltw_catch && vision_app->GetBool ("catchAltW"))
        {
           fAltw_catch = true;
           
           fAltwRunner = new BMessageRunner (
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
        fAltw_catch = false;
        if (fAltwRunner)
          delete fAltwRunner;
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
          netName.String(),
          WIN_SERVER_TYPE,
          pWindowList()->CountItems() > 0 ? false : true); // grab focus if none present
      }
      break;
    
    case M_RESIZE_VIEW:
      {
        BView *view (NULL);
        msg->FindPointer ("view", reinterpret_cast<void **>(&view));
        WindowListItem *item (dynamic_cast<WindowListItem *>(pWindowList()->ItemAt (pWindowList()->CurrentSelection())));
        BView *agent (item->pAgent());
        if (dynamic_cast<ClientWindowDock *>(view))
        {
          BPoint point;
          msg->FindPoint ("loc", &point);
          fResize->MoveTo (point.x, fResize->Frame().top);
          fCwDock->ResizeTo (point.x - 1, fCwDock->Frame().Height());
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
    {
      status_t result = be_roster->Launch ("application/x-vnd.Be-SHEL", 0, NULL);
      if (result != B_OK)
      {
        BMessage errMsg (M_DISPLAY);
        BString errString ("Launch failed! Error code: ");
        errString << result;
        errString += "\n";
        ClientAgent::PackDisplay(&errMsg, errString.String(), C_ERROR, C_BACKGROUND, F_TEXT);
        WindowListItem *item(dynamic_cast<WindowListItem *>(pWindowList()->ItemAt (pWindowList()->CurrentSelection())));
        if (item)
        {
          BMessenger (item->pAgent()).SendMessage (&errMsg);
        }
      }
    }
    break;
    
    case M_LIST_COMMAND:
    {
      WindowListItem *item ((WindowListItem *)pWindowList()->ItemAt (pWindowList()->CurrentSelection()));
      BView *view (NULL);
      if (item)
        view = item->pAgent();
      if ((view == NULL) || (dynamic_cast<ListAgent *>(view) != NULL))
        break;
      dynamic_cast<ClientAgent *>(view)->ParseCmd ("/LIST");
    }
    break;

    case M_NOTIFYLIST_UPDATE:
    {
      BObjectList<NotifyListItem> *nickList (NULL);
      BView *msgSource (NULL);
      int32 hasChanged (0);
      
      // whether we're the active one or not, no difference in state, ignore
      if (msg->HasInt32 ("change") && (hasChanged = msg->FindInt32("change")) == 0)
        break;
        
      msg->FindPointer("source", reinterpret_cast<void **>(&msgSource));
      msg->FindPointer("list", reinterpret_cast<void **>(&nickList));

      WindowListItem *item ((WindowListItem *)pWindowList()->ItemAt(pWindowList()->CurrentSelection()));
      if (item != NULL)
      {
        if (item->pAgent() == msgSource)
          pNotifyList()->UpdateList (nickList);
        else
        {
          item = (WindowListItem *)(pWindowList()->Superitem(item));
          if ((item != NULL) && (item->pAgent() == msgSource))
            pNotifyList()->UpdateList (nickList);
        }
        pWindowList()->BlinkNotifyChange(hasChanged, (ServerAgent *)msgSource);
      }

    }
    break;
        
    default:
      BWindow::MessageReceived (msg);
  }
}

ServerAgent *
ClientWindow::GetTopServer (WindowListItem *request) const
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

void
ClientWindow::SetEditStates (void)
{
  WindowListItem *item (dynamic_cast<WindowListItem *>(pWindowList()->ItemAt(pWindowList()->CurrentSelection())));
  
  if (item != NULL)
  {
    ClientAgent *agent (dynamic_cast<ClientAgent *>(item->pAgent()));
    VTextControl *input (NULL);
    if (agent != NULL)
      input = agent->pInput();
    BMenuItem *menuItem (fEdit->FindItem(S_CW_EDIT_CUT));
    if (input != NULL)
      menuItem->SetTarget (input->TextView());
    int32 start (0), finish (0);
    if (input != NULL)
      input->TextView()->GetSelection(&start, &finish);
    menuItem->SetEnabled (start != finish);
    menuItem = fEdit->FindItem(S_CW_EDIT_COPY);
    menuItem->SetEnabled (start != finish);
    if (input != NULL)
      menuItem->SetTarget (input->TextView());
    menuItem = fEdit->FindItem(S_CW_EDIT_PASTE);
    if (input != NULL)
      menuItem->SetTarget (input->TextView());
    BClipboard clipboard("system");
    BMessage *clip ((BMessage *)NULL);
    if (clipboard.Lock()) {
      if ((clip = clipboard.Data()))
      if (clip->HasData ("text/plain", B_MIME_TYPE))
        menuItem->SetEnabled(true);
      else
        menuItem->SetEnabled(false);
      clipboard.Unlock();
    }
  }
}

BRect *
ClientWindow::AgentRect (void) const
{
  fAgentrect->left = fResize->Frame().right - fCwDock->Frame().left + 1;
  fAgentrect->top = Bounds().top + 1;
  fAgentrect->right = Bounds().Width() - 1;
  fAgentrect->bottom = fCwDock->Frame().Height();
  return fAgentrect;
}


WindowList *
ClientWindow::pWindowList (void) const
{
  return fCwDock->pWindowList();
}

NotifyList *
ClientWindow::pNotifyList (void) const
{
  return fCwDock->pNotifyList();
}

ClientWindowDock *
ClientWindow::pCwDock (void) const
{
  return fCwDock;
}


StatusView *
ClientWindow::pStatusView (void) const
{
  return fStatus;
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
ClientWindow::ServerBroadcast (BMessage *outmsg_) const
{
  bool reply (false);
  
  for (int32 i (0); i < pWindowList()->CountItems(); i++)
    { 
      WindowListItem *aitem ((WindowListItem *)pWindowList()->ItemAt (i));
      if (aitem->Type() == WIN_SERVER_TYPE)
      {
        dynamic_cast<ServerAgent *>(aitem->pAgent())->fMsgr.SendMessage (outmsg_);
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

  fShutdown_in_progress = false;
  fWait_for_quits = false;
  fAltw_catch = false;

  AddShortcut ('W', B_COMMAND_KEY, new BMessage(M_CW_ALTW));
  AddShortcut ('Q', B_COMMAND_KEY, new BMessage(M_CW_ALTW));
  
  BRect frame (Bounds());
  fMenuBar = new BMenuBar (frame, "menu_bar");
  
  BMenuItem *item;
  BMenu *menu;
  // Server menu
  fServer = new BMenu (S_CW_SERVER_MENU);
  fServer->AddItem (menu = new NetworkMenu (S_CW_SERVER_CONNECT B_UTF8_ELLIPSIS, M_CONNECT_NETWORK, BMessenger (vision_app)));
  
  fServer->AddItem (item = new BMenuItem (S_CW_SERVER_SETUP B_UTF8_ELLIPSIS,
                    new BMessage (M_SETUP_SHOW), '/', B_SHIFT_KEY));
  item->SetTarget (vision_app);
  fMenuBar->AddItem (fServer);
  
  
  // Edit menu
  fEdit = new BMenu (S_CW_EDIT_MENU);

  fEdit->AddItem (item = new BMenuItem (S_CW_EDIT_CUT, new BMessage (B_CUT)));
  fEdit->AddItem (item = new BMenuItem (S_CW_EDIT_COPY, new BMessage (B_COPY)));
  fEdit->AddItem (item = new BMenuItem (S_CW_EDIT_PASTE, new BMessage (B_PASTE)));
  fEdit->AddSeparatorItem();
  fEdit->AddItem (item = new BMenuItem (S_CW_EDIT_PREFS B_UTF8_ELLIPSIS, new BMessage (M_PREFS_SHOW)));
  item->SetTarget (vision_app);
 
  fMenuBar->AddItem (fEdit);
  
  // Tools menu
  fTools = new BMenu (S_CW_TOOLS_MENU);

  fTools->AddItem (item = new BMenuItem (S_CW_TOOLS_CHANLIST,
                    new BMessage (M_LIST_COMMAND), 'L'));
  fTools->AddItem (item = new BMenuItem (S_CW_TOOLS_IGNORELIST B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'I'));
  item->SetTarget (vision_app);
  fTools->AddItem (item = new BMenuItem (S_CW_TOOLS_NOTIFYLIST B_UTF8_ELLIPSIS,
                    new BMessage (B_ABOUT_REQUESTED), 'N'));
  item->SetTarget (vision_app);
  
  fTools->AddItem (item = new BMenuItem (S_CW_TOOLS_TERMINAL, new BMessage (M_OPEN_TERM),
                    'T', B_OPTION_KEY));


  fMenuBar->AddItem (fTools);
  
  // Window menu
  fWindow = new BMenu (S_CW_WINDOW_MENU);
  
  fWindow->AddItem (item = new BMenuItem (S_CW_WINDOW_PART, new BMessage (M_CW_ALTP), 'P'));
  
  item->SetTarget (this);
  fMenuBar->AddItem (fWindow);  
  
  AddChild (fMenuBar);
  
  // add objects
  frame.top = fMenuBar->Frame().bottom + 1;
  bgView = new BView (frame,
                      "Background",
                      B_FOLLOW_ALL_SIDES,
                      0);

  bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  AddChild (bgView);
   

  frame = bgView->Bounds();

  fStatus = new StatusView (frame);
  bgView->AddChild (fStatus);
  
  fStatus->AddItem (new StatusItem (
    "irc.elric.net", 0),
    true);
  
  BRect cwDockRect (vision_app->GetRect ("windowDockRect"));
  fCwDock = new ClientWindowDock (BRect (0, frame.top, (cwDockRect.Width() == 0.0) ? 130 : cwDockRect.Width(), fStatus->Frame().top - 1));
  
  bgView->AddChild (fCwDock);
  
  fResize = new ResizeView (fCwDock, BRect (fCwDock->Frame().right + 1,
    Bounds().top + 1, fCwDock->Frame().right + 3, fStatus->Frame().top - 1));
  
  bgView->AddChild (fResize);

  fAgentrect = new BRect (
    (fResize->Frame().right - fCwDock->Frame().left) + 1,
    Bounds().top + 1,
    Bounds().Width() - 1,
    fCwDock->Frame().Height());
}

//////////////////////////////////////////////////////////////////////////////
/// End Private Functions
//////////////////////////////////////////////////////////////////////////////
