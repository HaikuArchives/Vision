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
 */
 

#include <ScrollView.h>
#include <StringView.h>

#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "NotifyList.h"
#include "Theme.h"
#include "Vision.h"
#include "WindowList.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDock functions
//////////////////////////////////////////////////////////////////////////////

ClientWindowDock::ClientWindowDock (BRect frame)
  : BView (
    frame,
    "agentDock",
    B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,
    B_WILL_DRAW | B_FRAME_EVENTS),
    fNotifyExpanded (false)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  
  fWorkingFrame = Bounds();
  
  // add collapsed agent first
  AddNotifyList();
  
  // add "focused" agent
  AddWinList();
}

ClientWindowDock::~ClientWindowDock (void)
{
  //
}

void
ClientWindowDock::AddWinList (void)
{
  fWinListAgent = new AgentDockWinList (fWorkingFrame);
  AddChild (fWinListAgent);
}

void
ClientWindowDock::AddNotifyList (void)
{
  BRect notifyFrame (fWorkingFrame);
  notifyFrame.top = fWorkingFrame.bottom - 15;
  
  fWorkingFrame.bottom = fWorkingFrame.bottom - (notifyFrame.Height() + 1);
  
  fNotifyAgent = new AgentDockNotifyList (notifyFrame);
  AddChild (fNotifyAgent);
}

WindowList *
ClientWindowDock::pWindowList (void)
{
  return fWinListAgent->pWindowList();
}

NotifyList *
ClientWindowDock::pNotifyList (void)
{
  return fNotifyAgent->pNotifyList();
}

void
ClientWindowDock::AllAttached (void)
{
  if (vision_app->GetBool ("notifyExpanded"))
  {
    BMessenger dockMsgr (this);
    dockMsgr.SendMessage(M_NOTIFYLIST_RESIZE);
  }
}

void
ClientWindowDock::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_NOTIFYLIST_RESIZE:
    {
      if (fNotifyExpanded)
      {
        fNotifyAgent->ResizeTo (fNotifyAgent->Bounds().Width(), 15.0);
        fNotifyAgent->MoveTo (0.0, Bounds().bottom - 15.0);
        fWinListAgent->ResizeBy (0.0, (fNotifyAgent->Frame().top - fWinListAgent->Frame().bottom - 1.0));
        fNotifyExpanded = false;
      }
      else
      {
        fWinListAgent->ResizeBy (0.0, -1.0 * (fWinListAgent->Bounds().Height() / 3.0));
        fNotifyAgent->MoveTo (0.0, fWinListAgent->Frame().bottom + 1.0);
        fNotifyAgent->ResizeBy (0.0, Frame().bottom - fNotifyAgent->Frame().bottom - 1.0);
        fNotifyExpanded = true;
      }
      vision_app->SetBool ("notifyExpanded", fNotifyExpanded);
      break;
    }
    
    default:
      BView::MessageReceived (msg);
      break;
  }
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDock functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockWinList functions
//////////////////////////////////////////////////////////////////////////////

AgentDockWinList::AgentDockWinList (BRect frame_)
  : BView (
    frame_,
    "agentDockWinList",
    B_FOLLOW_ALL,
    B_WILL_DRAW)
{
  SetViewColor (vision_app->GetColor (C_WINLIST_BACKGROUND));
  BRect frame (frame_);
  
  BRect headerFrame (frame);
  headerFrame.top = 1;
  headerFrame.bottom = 14;
  headerFrame.right = headerFrame.right;
  fAHeader = new AgentDockHeader (headerFrame, S_CWD_WINLIST_HEADER, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
  AddChild (fAHeader);
   
  frame.top = frame.top + headerFrame.Height() + 4;  // make room for header
  frame.right = frame.right - B_V_SCROLL_BAR_WIDTH; // scrollbar
  frame.bottom = frame.bottom - 2; // room for "fancy" border

  fWinList = new WindowList (frame);

  fWinListScroll = new BScrollView (
    "fWinListScroll",
    fWinList,
    B_FOLLOW_ALL,
    0,
    false,
    true,
    B_PLAIN_BORDER);
  AddChild (fWinListScroll);
  
  
}

AgentDockWinList::~AgentDockWinList (void)
{
  //
}

WindowList *
AgentDockWinList::pWindowList (void)
{
  return fWinList;
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockWinList functions
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockNotifyList functions
//////////////////////////////////////////////////////////////////////////////

AgentDockNotifyList::AgentDockNotifyList (BRect frame_)
  : BView (
    frame_,
    "agentDockNotifyList",
    B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
    B_WILL_DRAW)
{

  SetViewColor (vision_app->GetColor (B_PANEL_BACKGROUND_COLOR));
  BRect frame (frame_);
  
  BRect headerFrame (frame);
  headerFrame.top = 1;
  headerFrame.bottom = 14;
  headerFrame.right = headerFrame.right;
  fAHeader = new AgentDockHeader (headerFrame, S_CWD_NOTIFY_HEADER, B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
  headerFrame.top = headerFrame.top + headerFrame.Height() + 4;
  // BScrollView in R5 has an odd bug where if you initialize it too small, it never draws its scrollbar arrows correctly
  headerFrame.bottom = headerFrame.top + 30.0;
  headerFrame.right -= B_V_SCROLL_BAR_WIDTH;
  fNotifyList = new NotifyList (headerFrame);
  fNotifyScroll = new BScrollView ("fNotifyListScroll",
                                   fNotifyList,
                                   B_FOLLOW_ALL,
                                   0,
                                   false,
                                   true,
                                   B_PLAIN_BORDER);
  AddChild (fAHeader);
  AddChild (fNotifyScroll);
}

AgentDockNotifyList::~AgentDockNotifyList (void)
{
  //
}

NotifyList *
AgentDockNotifyList::pNotifyList (void)
{
  return fNotifyList;
}

void
AgentDockNotifyList::AllAttached (void)
{
  // hack to deal with some R5 scrollbar drawing bugs
  fNotifyScroll->ResizeBy (0.0, Bounds().Height() - fNotifyScroll->Bounds().Height() - 15.0);
  BView::AllAttached ();
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockNotifyList functions
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////

/*
 * Class Purpose: Provides the visual header for AgentDock Agents
 */

AgentDockHeaderString::AgentDockHeaderString (BRect frame_, const char *name)
 : BStringView (
   frame_,
   "fHeaderView",
   name,
   B_FOLLOW_LEFT | B_FOLLOW_BOTTOM)
{

}

AgentDockHeaderString::~AgentDockHeaderString (void)
{
}

void
AgentDockHeaderString::MouseMoved (BPoint where, uint32 transitcode, const BMessage *mmMsg)
{
  switch (transitcode)
  {
    case B_ENTERED_VIEW:
      SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
      Parent()->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
      Invalidate();
      Parent()->Invalidate();
      break;
    
    case B_EXITED_VIEW:
      SetViewColor (ui_color (B_MENU_BACKGROUND_COLOR));
      Parent()->SetViewColor (ui_color (B_MENU_BACKGROUND_COLOR));
      Invalidate();
      Parent()->Invalidate();
      break;
  }
    
  BStringView::MouseMoved (where, transitcode, mmMsg);
}

void
AgentDockHeaderString::MouseDown (BPoint where)
{
  SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_2_TINT));
  Parent()->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_2_TINT));
  Invalidate();
  Parent()->Invalidate();
  BStringView::MouseDown (where);
}

void
AgentDockHeaderString::MouseUp (BPoint where)
{
  SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
  Parent()->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
  Invalidate();
  Parent()->Invalidate();
  
  // check if this header string belongs to notify list and send resize message if so
  BView *notifyList (Parent());
  if (notifyList && dynamic_cast<AgentDockHeader *>(notifyList) != NULL)
  {
    notifyList = notifyList->Parent();
    if (notifyList && dynamic_cast<AgentDockNotifyList *>(notifyList) != NULL)
    {
      BMessenger msgr (((AgentDockNotifyList *)notifyList)->pNotifyList());
      if (msgr.IsValid())
        msgr.SendMessage (M_NOTIFYLIST_RESIZE);
    }
  }

  BStringView::MouseUp (where);
}



AgentDockHeader::AgentDockHeader (BRect frame, const char *name, uint32 resize)
  : BView (
    frame,
    "AgentDockHeader",
    resize,
    B_WILL_DRAW)
{  
  SetViewColor (ui_color (B_MENU_BACKGROUND_COLOR));

  BRect stringRect (frame);
  stringRect.left = stringRect.left + 3;
  stringRect.right = stringRect.right - 24;
    
  fHeaderView = new AgentDockHeaderString (stringRect, name);
  AddChild (fHeaderView);
}

AgentDockHeader::~AgentDockHeader (void)
{
  // nothing
}


//////////////////////////////////////////////////////////////////////////////
/// End AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////
