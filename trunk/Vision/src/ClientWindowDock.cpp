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

#include "ClientWindowDock.h"
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
    B_WILL_DRAW | B_FRAME_EVENTS)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  
  workingFrame = Bounds();
  
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
  winListAgent = new AgentDockWinList (workingFrame);
  AddChild (winListAgent);
}

void
ClientWindowDock::AddNotifyList (void)
{
  BRect notifyFrame (workingFrame);
  notifyFrame.top = workingFrame.bottom - 15;
  
  workingFrame.bottom = workingFrame.bottom - (notifyFrame.Height() + 1);
  
  notifyAgent = new AgentDockNotifyList (notifyFrame);
  AddChild (notifyAgent);
}

WindowList *
ClientWindowDock::pWindowList (void)
{
  return winListAgent->pWindowList();
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
  aHeader = new AgentDockHeader (headerFrame, "Window List", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
  AddChild (aHeader);
   
  frame.top = frame.top + headerFrame.Height() + 4;  // make room for header
  frame.right = frame.right - B_V_SCROLL_BAR_WIDTH; // scrollbar
  frame.bottom = frame.bottom - 2; // room for "fancy" border
    
  winList = new WindowList (frame);
  
  winListScroll = new BScrollView (
    "winListScroll",
    winList,
    B_FOLLOW_ALL,
    0,
    false,
    true,
    B_PLAIN_BORDER);
  AddChild (winListScroll);
  
  
}

AgentDockWinList::~AgentDockWinList (void)
{
  //
}

WindowList *
AgentDockWinList::pWindowList (void)
{
  return winList;
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
  headerFrame.top = 0;
  headerFrame.bottom = 14;
  headerFrame.right = headerFrame.right;
  aHeader = new AgentDockHeader (headerFrame, "Notify List", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
  AddChild (aHeader);
}

AgentDockNotifyList::~AgentDockNotifyList (void)
{
  //
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
   "headerView",
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
    
  headerView = new AgentDockHeaderString (stringRect, name);
  AddChild (headerView);
}

AgentDockHeader::~AgentDockHeader (void)
{
  // nothing
}

void
AgentDockHeader::MouseMoved (BPoint where, uint32 transitcode, const BMessage *mmMsg)
{
  switch (transitcode)
  {
    case B_ENTERED_VIEW:
      SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
      headerView->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
      Invalidate();
      headerView->Invalidate();
      break;
    
    case B_EXITED_VIEW:
      SetViewColor (ui_color (B_MENU_BACKGROUND_COLOR));
      headerView->SetViewColor (ui_color (B_MENU_BACKGROUND_COLOR));
      Invalidate();
      headerView->Invalidate();
      break;
  }
    
  BView::MouseMoved (where, transitcode, mmMsg);
}

void
AgentDockHeader::MouseDown (BPoint where)
{
  SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_2_TINT));
  headerView->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_2_TINT));
  Invalidate();
  headerView->Invalidate();
  
  BView::MouseDown (where);
}

void
AgentDockHeader::MouseUp (BPoint where)
{
  SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
  headerView->SetViewColor (tint_color (ui_color (B_MENU_BACKGROUND_COLOR), B_DARKEN_1_TINT));
  Invalidate();
  headerView->Invalidate();
  
  BView::MouseUp (where);
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////
