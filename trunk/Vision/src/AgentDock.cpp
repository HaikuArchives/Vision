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
 */
 

#include <ScrollView.h>
#include <StringView.h>

#include "AgentDock.h"
#include "Vision.h"
#include "WindowList.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////////////
/// Begin AgentDock functions
//////////////////////////////////////////////////////////////////////////////

AgentDock::AgentDock (BRect frame)
  : BView (
    frame,
    "agentDock",
    B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM,
    B_WILL_DRAW)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  
  workingFrame = Bounds();
  
  // add collapsed agent first
  AddNotifyList();
  
  // add "focused" agent
  AddWinList();
}

AgentDock::~AgentDock (void)
{
  //
}

void
AgentDock::AddWinList (void)
{
  winListAgent = new AgentDockWinList (workingFrame);
  AddChild (winListAgent);
}

void
AgentDock::AddNotifyList (void)
{
  BRect notifyFrame (workingFrame);
  notifyFrame.top = workingFrame.bottom - 13;
  
  workingFrame.bottom = workingFrame.bottom - notifyFrame.Height();
  
  notifyAgent = new AgentDockNotifyList (notifyFrame);
  AddChild (notifyAgent);
}

WindowList *
AgentDock::pWindowList (void)
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
    B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP_BOTTOM,
    B_WILL_DRAW)
{

  SetViewColor (vision_app->GetColor (C_WINLIST_BACKGROUND));
  BRect frame (frame_);
  
  BRect headerFrame (frame);
  headerFrame.top = 0;
  headerFrame.bottom = 14;
  headerFrame.right = headerFrame.right;
  aHeader = new AgentDockHeader (headerFrame, "Window List", B_FOLLOW_NONE);
  AddChild (aHeader);
   
  frame.top = frame.top + 15;  // make room for header
  frame.right = frame.right - B_V_SCROLL_BAR_WIDTH; // scrollbar
  frame.bottom = frame.bottom - 2; // room for "fancy" border
    
  winList = new WindowList (frame);
  
  winListScroll = new BScrollView (
    "winListScroll",
    winList,
    B_FOLLOW_TOP_BOTTOM,
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
    B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM,
    B_WILL_DRAW)
{

  SetViewColor (vision_app->GetColor (B_PANEL_BACKGROUND_COLOR));
  BRect frame (frame_);
  
  BRect headerFrame (frame);
  headerFrame.top = 0;
  headerFrame.bottom = 14;
  headerFrame.right = headerFrame.right;
  aHeader = new AgentDockHeader (headerFrame, "Notify List", B_FOLLOW_LEFT | B_FOLLOW_BOTTOM);
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
    
  headerView = new BStringView (stringRect,
                                "headerView",
                                name,
                                B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
  AddChild (headerView);
}

AgentDockHeader::~AgentDockHeader (void)
{
  //
}

//////////////////////////////////////////////////////////////////////////////
/// End AgentDockHeader functions
//////////////////////////////////////////////////////////////////////////////
