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
 * Contributor(s): Rene Gollent
 */

#include "PrefsWindow.h"
#include "PrefGeneral.h"
#include "Vision.h"

#include <stdio.h>

#include <TabView.h>
#include <View.h>
#include <OutlineListView.h>
#include <Box.h>
#include <ScrollView.h>

const uint32 M_NETPREFS_SELECTION_CHANGED = 'npsc';

class NetManagementPrefsView : public BView
{
  public:
    NetManagementPrefsView (BRect, const char *, uint32, uint32);
    virtual ~NetManagementPrefsView (void);
    virtual void MessageReceived (BMessage *);
    virtual void AttachedToWindow (void);
    virtual void AllAttached (void);
 private:
    BOutlineListView *netList;
};

PrefsWindow::PrefsWindow(void)
  : BWindow (BRect (0.0, 0.0, 0.0, 0.0),
      "Preferences",
      B_TITLED_WINDOW,
      B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{
  GeneralPrefsView *generalView = new GeneralPrefsView(BRect(0.0, 0.0, 0.0, 0.0),
     "view", B_FOLLOW_NONE, B_WILL_DRAW);

  ResizeTo(generalView->Bounds().Width() + 14, generalView->Bounds().Height() + 2 * be_plain_font->Size() + 14);

  BTabView *tabView = new BTabView (Bounds().InsetByCopy(7,7), "Preferences", B_WIDTH_FROM_LABEL);

//  NetManagementPrefsView *netView = new NetManagementPrefsView(tabView->Bounds().InsetByCopy(5,5),
//     "net management", B_FOLLOW_NONE, B_WILL_DRAW);
  
  BTab *generalTab = new BTab();
//  BTab *netTab = new BTab();
 
  BBox *box = new BBox (Bounds().InsetByCopy(-1,-1));
  
  AddChild(box);

  box->AddChild(tabView);  

  tabView->AddTab(generalView, generalTab);
//  tabView->AddTab(netView, netTab);
  
  generalTab->SetLabel("General");
//  netTab->SetLabel("Network");
  tabView->Select (0L);
}

PrefsWindow::~PrefsWindow(void)
{
}

bool
PrefsWindow::QuitRequested(void)
{
  be_app_messenger.SendMessage (M_SETUP_CLOSE);
  return true;  
}
void
PrefsWindow::MessageReceived(BMessage *msg)
{
  switch (msg->what)
  {
     default:
        BWindow::MessageReceived(msg);
        break;
  }
}

NetManagementPrefsView::NetManagementPrefsView (BRect frame, const char *title, uint32 redraw, uint32 flags)
  : BView (frame, title, redraw, flags)
{
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  BRect boundsRect (Bounds());
  netList = new BOutlineListView (BRect (boundsRect.left+5, boundsRect.top+5, boundsRect.left+101, boundsRect.bottom-25), "PrefsList");
  netList->AddItem (new BStringItem ("Defaults"));
  netList->SetSelectionMessage (new BMessage (M_NETPREFS_SELECTION_CHANGED));
  BScrollView *scroller (new BScrollView("list scroller", netList, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));
  AddChild (scroller);
  BBox *netBox (new BBox(BRect (scroller->Frame().right + 5, scroller->Frame().top, boundsRect.right-1, scroller->Frame().bottom), "Network Items"));
  AddChild (netBox);
}

NetManagementPrefsView::~NetManagementPrefsView (void)
{
}

void
NetManagementPrefsView::AttachedToWindow (void)
{
  BView::AttachedToWindow();
}

void
NetManagementPrefsView::AllAttached (void)
{
  BView::AllAttached();
  netList->SetTarget(this);
  netList->Select(0L);
}

void
NetManagementPrefsView::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_NETPREFS_SELECTION_CHANGED:
    {
      int32 index (msg->FindInt32 ("index"));
      if (index < 0) return;
    }
    break;
      
    default:
      msg->PrintToStream();
      BView::MessageReceived(msg);
  }
}
