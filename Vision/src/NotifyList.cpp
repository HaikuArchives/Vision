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
 * Contributor(s): Rene Gollent
 *								 Alan Ellis <alan@cgsoftware.org>
 *
 */

#include <Catalog.h>
#include <MenuItem.h>
#include <PopUpMenu.h>

#include "ClientWindow.h"
#include "ClientWindowDock.h"
#include "NotifyList.h"
#include "Theme.h"
#include "Utilities.h"
#include "Vision.h"
#include "WindowList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "NotifyList"

NotifyList::NotifyList (BRect _frame)
	: BListView (_frame,
			"NotifyList",
			B_SINGLE_SELECTION_LIST,
			B_FOLLOW_ALL ),
			fActiveTheme (vision_app->ActiveTheme()),
			fLastButton (0),
			fClickCount (0),
			fLastClick (0,0),
			fLastClickTime (0),
			fMyPopUp (NULL)
{
	fActiveTheme->ReadLock();
	SetFont (&fActiveTheme->FontAt (F_WINLIST));
	SetViewColor (fActiveTheme->ForegroundAt (C_NOTIFYLIST_BACKGROUND));
	fActiveTheme->ReadUnlock();
}

NotifyList::~NotifyList (void)
{
	while (CountItems() > 0)
		delete RemoveItem((int32)0);
	delete fMyPopUp;
}

void
NotifyList::UpdateList(BObjectList<NotifyListItem> *newList)
{
	while (CountItems() > 0)
		delete RemoveItem ((int32)0);
	BList updateList;
	// make private copy of list items otherwise things go bad
	for (int32 i = 0; i < newList->CountItems(); i++)
		updateList.AddItem (new NotifyListItem (*newList->ItemAt(i)));
	AddList(&updateList);
}

void
NotifyList::AttachedToWindow (void)
{
	fActiveTheme->AddView(this);
	BListView::AttachedToWindow ();
}

void
NotifyList::DetachedFromWindow (void)
{
	fActiveTheme->RemoveView(this);
	BListView::DetachedFromWindow ();
}

void
NotifyList::MouseDown (BPoint myPoint)
{
	BMessage *msg (Window()->CurrentMessage());
	int32 selected (IndexOf (myPoint));
	if (selected >= 0)
	{
		BMessage *inputMsg (Window()->CurrentMessage());
		int32 mousebuttons (0),
					keymodifiers (0);

		NotifyListItem *item ((NotifyListItem *)ItemAt(selected));
		if (!item)
			return;

		inputMsg->FindInt32 ("buttons", &mousebuttons);
		inputMsg->FindInt32 ("modifiers", &keymodifiers);

		bigtime_t sysTime;
		msg->FindInt64 ("when", &sysTime);
		uint16 clicks = CheckClickCount (myPoint, fLastClick, sysTime, fLastClickTime, fClickCount) % 3;

		// slight kludge to make sure the expand/collapse triangles behave how they should
		// -- needed since OutlineListView's Expand/Collapse-related functions are not virtual
		if (mousebuttons == B_PRIMARY_MOUSE_BUTTON)
		{
			if (((clicks % 2) == 0) && item->GetState())
			{
				// react to double click by creating a new messageagent or selecting
				// an existing one (use /query logic in parsecmd)
				BString data (item->Text());
				data.Prepend ("/QUERY ");
				BMessage submitMsg (M_SUBMIT);
				submitMsg.AddString ("input", data.String());
				submitMsg.AddBool ("history", false);
				// don't clear in case user has something typed in text control
				submitMsg.AddBool ("clear", false);
				WindowListItem *winItem ((WindowListItem *)vision_app->pClientWin()->pWindowList()->ItemAt(
					vision_app->pClientWin()->pWindowList()->CurrentSelection()));
				if (winItem)
				{
					BMessenger msgr (winItem->pAgent());
					if (msgr.IsValid())
						msgr.SendMessage(&submitMsg);
				}
			}
			else
				Select (selected);
		}

		if ((keymodifiers & B_SHIFT_KEY)	== 0
		&& (keymodifiers & B_OPTION_KEY)	== 0
		&& (keymodifiers & B_COMMAND_KEY) == 0
		&& (keymodifiers & B_CONTROL_KEY) == 0)
		{
			if (mousebuttons == B_SECONDARY_MOUSE_BUTTON)
			{
				if (item)
				{
					if(!item->IsSelected())
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

}

void
NotifyList::BuildPopUp(void)
{
	delete fMyPopUp;
	fMyPopUp = new BPopUpMenu("Notify Selection", false, false);

	int index (CurrentSelection());
	if (index < 0)
		return;

	NotifyListItem *item (dynamic_cast<NotifyListItem *>(ItemAt(index)));
	if (item)
	{
		BString name (item->Text());
		BMessage msg (M_SUBMIT);
		BString data ("/QUERY ");
		data.Append(name);
		msg.AddString("input", data.String());
		msg.AddBool ("history", false);
		msg.AddBool ("clear", false);
		fMyPopUp->AddItem (new BMenuItem (B_TRANSLATE("Query"), new BMessage (msg)));
		data = "/WHOIS ";
		data.Append(name);
		msg.ReplaceString("input", data.String());
		fMyPopUp->AddItem (new BMenuItem (B_TRANSLATE("Whois"), new BMessage (msg)));
		data = "/DCC CHAT ";
		data.Append(name);
		msg.ReplaceString("input", data.String());
		fMyPopUp->AddItem (new BMenuItem (B_TRANSLATE("DCC Chat"), new BMessage (msg)));
		fMyPopUp->AddSeparatorItem();
		data = "/UNNOTIFY ";
		data.Append(name);
		msg.ReplaceString("input", data.String());
		fMyPopUp->AddItem (new BMenuItem (B_TRANSLATE("Remove"), new BMessage (msg)));

		WindowListItem *winItem (dynamic_cast<WindowListItem *>(
			vision_app->pClientWin()->pWindowList()->ItemAt(
				vision_app->pClientWin()->pWindowList()->CurrentSelection())));
		if (winItem)
			fMyPopUp->SetTargetForItems(winItem->pAgent());

		if (!item->GetState())
		{
			// user is offline, do not allow whois, query or dcc chat
			fMyPopUp->ItemAt(0)->SetEnabled(false);
			fMyPopUp->ItemAt(1)->SetEnabled(false);
			fMyPopUp->ItemAt(2)->SetEnabled(false);
		}
		fMyPopUp->SetFont(be_plain_font);
	}
}

void
NotifyList::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_NOTIFYLIST_RESIZE:
			{
				ClientWindow *cWin (vision_app->pClientWin());
				cWin->DispatchMessage (msg, cWin->pCwDock());
				break;
			}

		case M_THEME_FOREGROUND_CHANGE:
			{
				int16 which (msg->FindInt16 ("which"));
				bool refresh (false);
				switch (which)
				{
					case C_NOTIFYLIST_BACKGROUND:
						fActiveTheme->ReadLock();
						SetViewColor (fActiveTheme->ForegroundAt (C_NOTIFYLIST_BACKGROUND));
						fActiveTheme->ReadUnlock();
						refresh = true;
						break;

					case C_NOTIFY_ON:
					case C_NOTIFY_OFF:
					case C_NOTIFYLIST_SELECTION:
						refresh = true;
						break;
				}
				if (refresh)
					Invalidate();
			}
			break;

		case M_THEME_FONT_CHANGE:
			{
				int16 which (msg->FindInt16 ("which"));
				if (which == F_WINLIST)
				{
					fActiveTheme->ReadLock();
					SetFont (&fActiveTheme->FontAt (F_WINLIST));
					fActiveTheme->ReadUnlock();
					Invalidate();
				}
			}
			break;

		default:
			BListView::MessageReceived (msg);
	}
}

NotifyListItem::NotifyListItem (const char *name, bool state)
	: BStringItem (name),
		fNotifyState (state)
{
	// empty c'tor
}

NotifyListItem::NotifyListItem (const NotifyListItem &copyItem)
	: BStringItem (copyItem.Text()),
		fNotifyState (copyItem.fNotifyState)
{
	// empty copy c'tor
}

NotifyListItem::~NotifyListItem (void)
{
	// empty d'tor
}

void
NotifyListItem::SetState (bool newState)
{
	fNotifyState = newState;
}

bool
NotifyListItem::GetState(void) const
{
	return fNotifyState;
}

void
NotifyListItem::DrawItem (BView *father, BRect frame, bool complete)
{
	Theme *fActiveTheme (vision_app->ActiveTheme());

	fActiveTheme->ReadLock();

	if (IsSelected())
	{
		father->SetHighColor (fActiveTheme->ForegroundAt (C_NOTIFYLIST_SELECTION));
		father->SetLowColor (fActiveTheme->ForegroundAt (C_NOTIFYLIST_BACKGROUND));
		father->FillRect (frame);
	}
	else if (complete)
	{
		father->SetLowColor (fActiveTheme->ForegroundAt (C_NOTIFYLIST_BACKGROUND));
		father->FillRect (frame, B_SOLID_LOW);
	}

	rgb_color color = fActiveTheme->ForegroundAt ((fNotifyState) ? C_NOTIFY_ON : C_NOTIFY_OFF);

	font_height fh;
	father->GetFontHeight (&fh);

	father->MovePenTo (
		frame.left + 4,
		frame.bottom - fh.descent);

	BString drawString (Text());

	fActiveTheme->ReadUnlock();

	father->SetHighColor (color);

	father->SetDrawingMode (B_OP_OVER);
	father->DrawString (drawString.String());
	father->SetDrawingMode (B_OP_COPY);
}
