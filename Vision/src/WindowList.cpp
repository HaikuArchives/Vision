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
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jean-Baptiste M. Queru <jbq@be.com>
 *								 Seth Flaxman
 *								 Alan Ellis <alan@cgsoftware.org>
 */
 
#include <Catalog.h>
#include <List.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "Theme.h"
#include "Vision.h"
#include "WindowList.h"
#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ServerAgent.h"
#include "ListAgent.h"
#include "Utilities.h"
#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "WindowList"

//////////////////////////////////////////////////////////////////////////////
/// Begin WindowList functions
//////////////////////////////////////////////////////////////////////////////

WindowList::WindowList (BRect frame)
	: BOutlineListView (
		frame,
		"windowList",
		B_SINGLE_SELECTION_LIST,
		B_FOLLOW_ALL),
			fMyPopUp (NULL),
			fLastSelected (NULL),
			fActiveTheme (vision_app->ActiveTheme()),
			fLastButton (0),
			fClickCount (0),
			fLastClick (0, 0),
			fLastClickTime (0)
{
	fActiveTheme->ReadLock();
	
	SetFont (&fActiveTheme->FontAt (F_WINLIST));

	SetViewColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));
	
	fActiveTheme->ReadUnlock();
	
	SetTarget (this);
}

WindowList::~WindowList (void)
{
	delete fMyPopUp;
	//
}

void
WindowList::AttachedToWindow (void)
{
	BView::AttachedToWindow ();
	fActiveTheme->WriteLock();
	fActiveTheme->AddView (this);
	fActiveTheme->WriteUnlock();
}

void
WindowList::DetachedFromWindow (void)
{
	BView::DetachedFromWindow ();
	fActiveTheme->WriteLock();
	fActiveTheme->RemoveView (this);
	fActiveTheme->WriteUnlock();
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
			
		case M_WINLIST_NOTIFY_BLINKER:
				{
					int32 state (msg->FindInt32 ("state"));
					WindowListItem *item (NULL);
					msg->FindPointer ("source", reinterpret_cast<void **>(&item));
					// this is insufficient since it seems events can be timed such that the runner
					// lets off one last message before being destroyed in a destructor
					// as such, verify that the item does indeed still exist in the list before doing this
					if ((item != NULL) && HasItem(item))
					{
						int32 oldState (item->BlinkState());
						item->SetBlinkState ((oldState == 0) ? state : 0);
					}
				}
				break;
		
		case M_THEME_FONT_CHANGE:
			{
				int16 which (msg->FindInt16 ("which"));
				if (which == F_WINLIST)
				{
					fActiveTheme->ReadLock();
					BFont newFont (fActiveTheme->FontAt (F_WINLIST));
					fActiveTheme->ReadUnlock();
					SetFont (&newFont);
					for (int32 i = 0; i < FullListCountItems(); i++)
						FullListItemAt (i)->Update (this, &newFont);
					Invalidate();
				}
			}
			break;
			
		case M_THEME_FOREGROUND_CHANGE:
			{
				int16 which (msg->FindInt16 ("which"));
				bool refresh (false);
				switch (which)
				{
					case C_WINLIST_BACKGROUND:
						fActiveTheme->ReadLock();
						SetViewColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));
						fActiveTheme->ReadUnlock();
						refresh = true;
						break;
					
					case C_WINLIST_SELECTION:
					case C_WINLIST_NORMAL:
					case C_WINLIST_NEWS:
					case C_WINLIST_NICK:
					case C_WINLIST_PAGESIX:
						refresh = true;
						break;
				}
				if (refresh)
					Invalidate();
			}
			break;
			
		case B_SIMPLE_DATA:
		{
			BPoint dropPoint;
			msg->FindPoint("_drop_point_", &dropPoint);
			dropPoint = ConvertFromScreen(dropPoint);
			
			msg->PrintToStream();
			
			int32 idx (IndexOf(dropPoint));
			
			if (idx >= 0)
			{
				WindowListItem *item (reinterpret_cast<WindowListItem *>(ItemAt(idx)));
				if (item != NULL)
				{
					BMessenger msgr (item->pAgent());
					if (msgr.IsValid())
					{
						msgr.SendMessage(msg);
					}
				}
			}
		}
		break;	 
		
		default:
			BOutlineListView::MessageReceived (msg);
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
		
		BMessenger killmsgr (killTarget);
		killmsgr.SendMessage (&killMsg);
	}
}

void
WindowList::MouseDown (BPoint myPoint)
{
	BMessage *msg (Window()->CurrentMessage());
	int32 selected (IndexOf (myPoint));
	if (selected >= 0)
	{
		BMessage *inputMsg (Window()->CurrentMessage());
		int32 mousebuttons (0),
					keymodifiers (0);

		inputMsg->FindInt32 ("buttons", &mousebuttons);
		inputMsg->FindInt32 ("modifiers", &keymodifiers);
		
		bigtime_t sysTime;
		msg->FindInt64 ("when", &sysTime);
		uint16 clicks (0);
		if (mousebuttons == B_PRIMARY_MOUSE_BUTTON)
			clicks = CheckClickCount (myPoint, fLastClick, sysTime, fLastClickTime, fClickCount) % 3;
		
		// slight kludge to make sure the expand/collapse triangles behave how they should
		// -- needed since OutlineListView's Expand/Collapse-related functions are not virtual
		if ((myPoint.x < 10.0) || ((clicks % 2) == 0))
		{
			// since Expand/Collapse are not virtual, override them by taking over processing
			// the collapse triangle logic manually
			WindowListItem *item ((WindowListItem *)ItemAt (selected));
			if (item)
			{
				if (item->Type() == WIN_SERVER_TYPE)
				{
					if (item->IsExpanded())
						Collapse (item);
					else
						Expand (item);
				}
				// if a non-server item was double-clicked, treat it as a regular click
				else
					Select (selected);
			}
		}
		else if (mousebuttons == B_PRIMARY_MOUSE_BUTTON)
			Select (selected);

		if ((keymodifiers & B_SHIFT_KEY)	== 0
		&& (keymodifiers & B_OPTION_KEY)	== 0
		&& (keymodifiers & B_COMMAND_KEY) == 0
		&& (keymodifiers & B_CONTROL_KEY) == 0)
		{
			if (mousebuttons == B_SECONDARY_MOUSE_BUTTON)
			{
				BListItem *item = ItemAt(IndexOf(myPoint));
				if (item && !item->IsSelected())
					Select (IndexOf (myPoint));

				BuildPopUp();

				fMyPopUp->Go (
					ConvertToScreen (myPoint),
					true,
					true,
					ConvertToScreen (ItemFrame (selected)),
					true);
			}
		}
	}
}

void 
WindowList::KeyDown (const char *, int32) 
{
	// TODO: WindowList never gets keyboard focus (?)
	// if you have to uncomment this, either fix WindowList so it
	// doesn't retain keyboard focus, or update this code to work
	// like IRCView::KeyDown()	--wade 20010506
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
		activeagent->fMsgr.SendMessage (&inputMsg);
	#endif
}

void
WindowList::SelectionChanged (void)
{
	int32 currentIndex (CurrentSelection());
	if (currentIndex >= 0) // dont bother casting unless somethings selected
	{
		WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (currentIndex)));
		if (myItem != NULL)
		{
			if (myItem->OutlineLevel() > 0)
				myItem = dynamic_cast<WindowListItem *>(Superitem(myItem));
			Activate (currentIndex);
			
			// reset blink state box
			myItem->SetBlinkState(-1);

			if (fLastSelected != NULL)
			{
				WindowListItem *lastItem (fLastSelected);
				if (lastItem->OutlineLevel() > 0)
					lastItem = dynamic_cast<WindowListItem *>(Superitem(lastItem));
				if (lastItem->pAgent() != myItem->pAgent())
					BMessenger(myItem->pAgent()).SendMessage(M_NOTIFYLIST_UPDATE);
			}
			else
				BMessenger(myItem->pAgent()).SendMessage(M_NOTIFYLIST_UPDATE);

		}
	}
	BOutlineListView::SelectionChanged();
}

void
WindowList::ClearList (void)
{
	// never ever call this function unless you understand
	// the consequences!
	int32 i,
				all (CountItems());

	for (i = 0; i < all; i++)
		delete static_cast<WindowListItem *>(RemoveItem ((int32)0));
}

void
WindowList::SelectLast (void)
{
	/*
	 * Function purpose: Select the last active agent
	 */
	int32 lastInt (IndexOf (fLastSelected));
	if (lastInt >= 0)
		Select (lastInt);
	else
		Select ((int32)0);
	ScrollToSelection();
}

void
WindowList::BlinkNotifyChange(int32 changeState, ServerAgent *victim)
{
	if (victim != NULL)
	{
		WindowListItem *item (victim->fAgentWinItem);
		if (HasItem(item))
			item->SetNotifyBlinker(changeState);
	}
}

void
WindowList::Collapse (BListItem *collapseItem)
{
	WindowListItem *citem ((WindowListItem *)collapseItem),
											 *item (NULL);
	int32 fSubstatus (-1);
	int32 itemcount (CountItemsUnder (citem, true));

	for (int32 i = 0; i < itemcount; i++)
		if (fSubstatus < (item = (WindowListItem *)ItemUnderAt(citem, true, i))->Status())
			fSubstatus = item->Status();

	citem->SetSubStatus (fSubstatus);
	BOutlineListView::Collapse (collapseItem);
}

void
WindowList::CollapseCurrentServer (void)
{
	int32 currentsel (CurrentSelection());
	if (currentsel < 0)
		return;
	
	int32 serversel (GetServer (currentsel));
	
	if (serversel < 0)
		return;
		
	WindowListItem *citem ((WindowListItem *)ItemAt (serversel));
	if (citem && (citem->Type() == WIN_SERVER_TYPE))
	{
		if (citem->IsExpanded())
			Collapse (citem);
	}
}

void
WindowList::Expand (BListItem *expandItem)
{
	((WindowListItem *)expandItem)->SetSubStatus (-1);
	BOutlineListView::Expand (expandItem);	
}

void
WindowList::ExpandCurrentServer(void)
{
	int32 currentsel (CurrentSelection());
	if (currentsel < 0)
		return;

	WindowListItem *citem ((WindowListItem *)ItemAt (currentsel));
	if (citem && (citem->Type() == WIN_SERVER_TYPE))
	{
		if (!citem->IsExpanded())
			Expand (citem);
	}
}

int32
WindowList::GetServer (int32 index)
{
	if (index < 0)
		return -1;

	WindowListItem *citem ((WindowListItem *)ItemAt (index));
	
	if (citem == NULL)
		return -1;		
		
	if (citem->Type() == WIN_SERVER_TYPE)
		return index;
	
	return IndexOf (Superitem(citem));
}

void
WindowList::SelectServer (void)
{
	int32 currentsel (CurrentSelection());
	if (currentsel < 0)
		return;
	
	int32 serversel = GetServer (currentsel);
	if (serversel < 0)
		return;
		
	Select (serversel);
}

void
WindowList::ContextSelectUp (void)
{
	int32 currentsel (CurrentSelection());
	if (currentsel < 0)
		return;
					
	WindowListItem *selItem (NULL),
								 *aItem (NULL);

	for (int32 iloop = currentsel - 1; iloop > -1; --iloop)
	{
		aItem = dynamic_cast<WindowListItem *>(ItemAt (iloop));
		if (aItem)
		{
			if (selItem)
			{
				if (aItem->Status() > selItem->Status())
					selItem = aItem;
			}
			else
			{
				selItem = aItem;
			}
			// no point in searching any further
			if (selItem->Status() == WIN_NICK_BIT)
				break;
		}	
	}

	// just select the previous item then.
	if (selItem)
		Select (IndexOf(selItem));
	ScrollToSelection();				
}

void
WindowList::ContextSelectDown (void)
{
	int32 currentsel (CurrentSelection());
	if (currentsel < 0)
		return;
					
	WindowListItem *selItem (NULL),
								 *aItem (NULL);

	for (int32 iloop = currentsel + 1; iloop < CountItems(); ++iloop)
	{
		aItem = dynamic_cast<WindowListItem *>(ItemAt (iloop));
		if (aItem)
		{
			if (selItem)
			{
				if (aItem->Status() > selItem->Status())
					selItem = aItem;
			}
			else
			{
				selItem = aItem;
			}
			// no point in searching any further
			if (selItem->Status() == WIN_NICK_BIT)
				break;
		}	
	}

	// just select the previous item then.
	if (selItem)
		Select (IndexOf(selItem));
	ScrollToSelection();				
}

void
WindowList::MoveCurrentUp (void)
{
	int32 currentsel (FullListCurrentSelection());
	if (currentsel < 0)
		return;
	
	WindowListItem *item (dynamic_cast<WindowListItem *>(FullListItemAt(currentsel)));
	if (item == NULL)
		return;
		
	WindowListItem *selItem(item);
	
	if (item->Type() != WIN_SERVER_TYPE)
	{
		item = dynamic_cast<WindowListItem *>(Superitem(item));
		if (item == NULL)
			return;
		currentsel = IndexOf(item);
	}
	
	if (currentsel > 0)
	{
		for (int32 i = currentsel - 1; i >= 0; i--)
			if (dynamic_cast<WindowListItem *>(FullListItemAt(i))->Type() == WIN_SERVER_TYPE)
			{
				SwapItems(currentsel, i);
				Select(IndexOf(selItem));
				ScrollToSelection();
				break;
			}
	}

}

void
WindowList::MoveCurrentDown (void)
{
	int32 currentsel (FullListCurrentSelection());
	if (currentsel < 0)
		return;
		
	WindowListItem *item (dynamic_cast<WindowListItem *>(FullListItemAt(currentsel)));
	if (item == NULL)
		return;

	WindowListItem *selItem(item);

	if (item->Type() != WIN_SERVER_TYPE)
	{
		item = dynamic_cast<WindowListItem *>(Superitem(item));
		if (item == NULL)
			return;
		currentsel = IndexOf(item);
	}
	
	int32 itemCount (FullListCountItems());
	
	if (currentsel < itemCount)
	{
		currentsel = FullListIndexOf (item);
		int32 i (currentsel + CountItemsUnder(item, true) + 1);
		if (i < itemCount && dynamic_cast<WindowListItem *>(FullListItemAt(i))->Type() == WIN_SERVER_TYPE)
		{
			SwapItems(currentsel, i);
			Select(IndexOf(selItem));
			ScrollToSelection();
		}
	}
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
WindowList::AddAgent (BView *agent, const char *name, int32 winType, bool activate)
{
	if (agent == NULL)
		return;

	int32 itemindex (0);

	WindowListItem *currentitem ((WindowListItem *)ItemAt (CurrentSelection()));
	
	WindowListItem *newagentitem (new WindowListItem (name, winType, WIN_NORMAL_BIT, agent));
	if (dynamic_cast<ServerAgent *>(agent) != NULL) 
		AddItem (newagentitem); 
	else 
	{ 
		BLooper *looper (NULL); 
		ServerAgent *agentParent (NULL); 
		if (dynamic_cast<ClientAgent *>(agent) != NULL) 
			agentParent = dynamic_cast<ServerAgent *>(dynamic_cast<ClientAgent *>(agent)->fSMsgr.Target(&looper)); 
		else 
			agentParent = dynamic_cast<ServerAgent *>(dynamic_cast<ListAgent *>(agent)->fSMsgr->Target(&looper)); 
		AddUnder (newagentitem, agentParent->fAgentWinItem); 
		SortItemsUnder (agentParent->fAgentWinItem, false, SortListItems); 
	}
	
	itemindex = IndexOf (newagentitem);

	// give the agent its own pointer to its WinListItem,
	// so it can quickly update it's status entry
	ClientAgent *clicast (NULL);
	ListAgent *listcast (NULL);
	if ((clicast = dynamic_cast<ClientAgent *>(agent)) != NULL)
		clicast->fAgentWinItem = newagentitem;
	else if ((listcast = dynamic_cast<ListAgent *>(agent)) != NULL)
		listcast->fAgentWinItem = newagentitem;

	vision_app->pClientWin()->DisableUpdates();
	agent->Hide(); // get it out of the way
	vision_app->pClientWin()->bgView->AddChild (agent);
	agent->Sync(); // clear artifacts
	vision_app->pClientWin()->EnableUpdates();

	if (activate && itemindex >= 0)	// if activate is true, show the new view now.
	{
		if (CurrentSelection() == -1)
			Select (itemindex); // first item, let SelectionChanged() activate it
		else
			Activate (itemindex);
	}
	else
		Select (IndexOf (currentitem));
}

void
WindowList::Activate (int32 index)
{
	if (index < 0)
		return;
	WindowListItem *newagentitem ((WindowListItem *)ItemAt (index));
	BView *newagent (newagentitem->pAgent());

	// find the currently active agent (if there is one)
	BView *activeagent (NULL);
	for (int32 i (0); i < FullListCountItems(); ++i)
	{ 
		WindowListItem *aitem ((WindowListItem *)FullListItemAt (i));
		if (!aitem->pAgent()->IsHidden())
		{
			activeagent = aitem->pAgent();
			fLastSelected = aitem;
			break;
		}
	}
	
	if (!(newagent = dynamic_cast<BView *>(newagent)))
	{
		// stop crash
		printf ("no newagent!?\n");
		return;
	}
	
	 
	if ((activeagent != newagent) && (activeagent != NULL))
	{
		newagent->Show();
		
		if (activeagent)
		{
			activeagent->Hide(); // you arent wanted anymore!
			activeagent->Sync(); // and take your damned pixels with you!
		}
	}
	if (activeagent == 0)
		newagent->Show();
 
	// activate the input box (if it has one)
	if ( (newagent = dynamic_cast<ClientAgent *>(newagent)) )
		reinterpret_cast<ClientAgent *>(newagent)->fMsgr.SendMessage (M_INPUT_FOCUS);

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
	Window()->DisableUpdates();
	agent->Hide();
	agent->Sync();
	agent->RemoveSelf();
	RemoveItem (agentitem);
	// not quite sure why this would happen but better safe than sorry
	if (fLastSelected == agentitem)
		fLastSelected = NULL;
	// agent owns the window list item and destroys it on destruct
	delete agent;
	// if there isn't anything left in the list, don't try to do any ptr comparisons
	if (CountItems() > 0)
		SelectLast();
	fLastSelected = NULL;
	Window()->EnableUpdates();
}


int
WindowList::SortListItems (const BListItem *name1, const BListItem *name2)
{
	const WindowListItem *firstPtr ((const WindowListItem *)name1);
	const WindowListItem *secondPtr ((const WindowListItem *)name2);

	/* Not sure if this can happen, and we
	 * are assuming that if one is NULL
	 * we return them as equal.	What if one
	 * is NULL, and the other isn't?
	 */
	if (!firstPtr
	||	!secondPtr)
	{
		return 0;
	}
	
	BString firstName, secondName;
	
	firstName = (firstPtr)->Name();
	secondName = (secondPtr)->Name();
 
	if ((firstPtr)->Type() == WIN_SERVER_TYPE)
		return -1;
	return firstName.ICompare (secondName);
}

void
WindowList::BuildPopUp (void)
{
	delete fMyPopUp;
	fMyPopUp = new BPopUpMenu("Window Selection", false, false);
	BMenuItem *item;
	
	WindowListItem *myItem (dynamic_cast<WindowListItem *>(ItemAt (CurrentSelection())));
	if (myItem)
	{
		ClientAgent *activeagent (dynamic_cast<ClientAgent *>(myItem->pAgent()));
		if (activeagent)
			activeagent->AddMenuItems (fMyPopUp);
	}
	
	item = new BMenuItem(B_TRANSLATE("Close"), new BMessage (M_MENU_NUKE));
	item->SetTarget (this);
	fMyPopUp->AddItem (item);
	
	fMyPopUp->SetFont (be_plain_font);
}


//////////////////////////////////////////////////////////////////////////////
/// End WindowList functions
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// Begin WindowListItem functions
//////////////////////////////////////////////////////////////////////////////

WindowListItem::WindowListItem (
	const char *name,
	int32 winType,
	int32 winStatus,
	BView *agent)

	: BListItem (),
		fMyName (name),
		fMyStatus (winStatus),
		fMyType (winType),
		fSubStatus (-1),
		fMyAgent (agent),
		fBlinkState (0),
		fBlinkStateCount (0),
		fBlinker (NULL)
{

}

WindowListItem::~WindowListItem (void)
{
	if (fBlinker)
		delete fBlinker;
}

BString
WindowListItem::Name (void) const
{
	return fMyName;
}

int32
WindowListItem::Status() const
{
	return fMyStatus;
}

int32
WindowListItem::SubStatus() const
{
	return fSubStatus;
}

int32
WindowListItem::BlinkState() const
{
	return fBlinkState;
}

int32
WindowListItem::Type() const
{
	return fMyType;
}

BView *
WindowListItem::pAgent() const
{
	return fMyAgent;
}

// TODO: verify that these Lock/Unlocks are not needed -- in theory they shouldn't be
// since these functions only get called in response to window messages, and as such
// the looper *has* to be locked -- needs testing to verify

void
WindowListItem::SetName (const char *name)
{
//	vision_app->pClientWin()->Lock();
	fMyName = name;
	int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
	vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
	
	// if the agent is active, update ClientWindow's titlebar
	if (IsSelected())
	{
		BString agentid (name);
		agentid.Append (" - Vision");
		vision_app->pClientWin()->SetTitle (agentid.String());
	}
		
//	vision_app->pClientWin()->Unlock();
}

void
WindowListItem::SetSubStatus (int32 winStatus)
{
//	vision_app->pClientWin()->Lock();
	fSubStatus = winStatus;
	int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
	vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
//	vision_app->pClientWin()->Unlock();
}

void
WindowListItem::SetStatus (int32 winStatus)
{
//	vision_app->pClientWin()->Lock();
	fMyStatus = winStatus;
	int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
	vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
//	vision_app->pClientWin()->Unlock();
}

void
WindowListItem::ActivateItem (void)
{
	int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
	vision_app->pClientWin()->pWindowList()->Activate (myIndex);
}

void
WindowListItem::DrawItem (BView *father, BRect frame, bool complete)
{
	Theme *fActiveTheme (vision_app->ActiveTheme());
	
	fActiveTheme->ReadLock();

	if (fSubStatus > WIN_NORMAL_BIT)
	{
		rgb_color color = rgb_color();
		if ((fSubStatus & WIN_NEWS_BIT) != 0)
			color = fActiveTheme->ForegroundAt (C_WINLIST_NEWS);
		else if ((fSubStatus & WIN_PAGESIX_BIT) != 0)
			color = fActiveTheme->ForegroundAt (C_WINLIST_PAGESIX);
		else if ((fSubStatus & WIN_NICK_BIT) != 0)
			color = fActiveTheme->ForegroundAt (C_WINLIST_NICK);
		
		father->SetHighColor (color);
		father->StrokeRect (BRect (0.0, frame.top, 10.0, frame.top + 10.0));
	}
	if (IsSelected())
	{
		father->SetHighColor (fActiveTheme->ForegroundAt (C_WINLIST_SELECTION));
		father->SetLowColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));		
		father->FillRect (frame);
	}
	else if (complete)
	{
		father->SetLowColor (fActiveTheme->ForegroundAt (C_WINLIST_BACKGROUND));
		father->FillRect (frame, B_SOLID_LOW);
	}

	font_height fh;
	father->GetFontHeight (&fh);

	father->MovePenTo (
		frame.left + 4,
		frame.bottom - fh.descent);

	BString drawString (fMyName);
	rgb_color color = fActiveTheme->ForegroundAt (C_WINLIST_NORMAL);

	if ((fMyStatus & WIN_NEWS_BIT) != 0)
		color = fActiveTheme->ForegroundAt (C_WINLIST_NEWS);

	else if ((fMyStatus & WIN_PAGESIX_BIT) != 0)
		color = fActiveTheme->ForegroundAt (C_WINLIST_PAGESIX);

	else if ((fMyStatus & WIN_NICK_BIT) != 0)
		color = fActiveTheme->ForegroundAt (C_WINLIST_NICK);

	if (IsSelected())
		color = fActiveTheme->ForegroundAt (C_WINLIST_NORMAL);
	
	father->SetHighColor (color);

	father->SetDrawingMode (B_OP_OVER);
	father->DrawString (drawString.String());
	father->SetDrawingMode (B_OP_COPY);

	if (fBlinkState > 0)
	{
		color = fActiveTheme->ForegroundAt((fBlinkState == 1) ? C_NOTIFY_ON : C_NOTIFY_OFF);
		father->SetHighColor (color);
		father->StrokeRect(frame);
	}

	fActiveTheme->ReadUnlock();
}

void
WindowListItem::SetNotifyBlinker(int32 state)
{
	BMessage *msg (new BMessage (M_WINLIST_NOTIFY_BLINKER));
	msg->AddPointer ("source", this);
	msg->AddInt32 ("state", state);
	if (fBlinker)
		delete fBlinker;
	fBlinkStateCount = 0;
	fBlinkState = 0;
	fBlinker = new BMessageRunner (BMessenger(vision_app->pClientWin()->pWindowList()),
																				 msg, 300000, 5);
}

void
WindowListItem::SetBlinkState(int32 state)
{
	if (state < 0)
	{
		// reset item
		fBlinkStateCount = 0;
		fBlinkState = 0;
	}
	else
	{
		fBlinkStateCount++;
		fBlinkState = state;
	}
	vision_app->pClientWin()->Lock();
	int32 myIndex (vision_app->pClientWin()->pWindowList()->IndexOf (this));
	vision_app->pClientWin()->pWindowList()->InvalidateItem (myIndex);
	vision_app->pClientWin()->Unlock();
	if (fBlinkStateCount == 5)
	{
		fBlinkStateCount = 0;
		delete fBlinker;
		fBlinker = NULL;
	}
	
}

//////////////////////////////////////////////////////////////////////////////
/// End WindowListItem functions
//////////////////////////////////////////////////////////////////////////////

