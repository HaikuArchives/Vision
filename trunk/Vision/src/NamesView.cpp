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
 *								 Jamie Wilkinson
 *								 Seth Flaxman
 *								 Alan Ellis <alan@cgsoftware.org>
 */

#include <Directory.h>
#include <Entry.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "ChannelAgent.h"
#include "Vision.h"
#include "Names.h"
#include "Theme.h"

NamesView::NamesView(BRect frame)
	: BListView(
		frame,
		"namesList",
		B_MULTIPLE_SELECTION_LIST,
		B_FOLLOW_ALL),
			fActiveTheme (vision_app->ActiveTheme())
{
	fActiveTheme->ReadLock();
	SetFont (&fActiveTheme->FontAt (F_NAMES));

	SetViewColor (fActiveTheme->ForegroundAt (C_NAMES_BACKGROUND));
	fActiveTheme->ReadUnlock();

	fTracking = false;
}

NamesView::~NamesView (void)
{
	if (fMyPopUp)
		delete fMyPopUp;
}

void
NamesView::KeyDown (const char * bytes, int32 numBytes)
{
	BMessage inputMsg (M_INPUT_FOCUS);
	BString buffer;

	buffer.Append (bytes, numBytes);
	inputMsg.AddString ("text", buffer.String());

	reinterpret_cast<ChannelAgent *>(Parent()->Parent())->fMsgr.SendMessage (&inputMsg);
}

void NamesView::AttachedToWindow (void)
{
	fMyPopUp = new BPopUpMenu("User selection", false, false);


	BMessage *myMessage = new BMessage (M_NAMES_POPUP_WHOIS);
	fMyPopUp->AddItem(new BMenuItem("Whois", myMessage));

	myMessage = new BMessage (M_OPEN_MSGAGENT);
	fMyPopUp->AddItem(new BMenuItem("Query", myMessage));

	myMessage = new BMessage (M_NAMES_POPUP_NOTIFY);
	fMyPopUp->AddItem(new BMenuItem("Add To Notify", myMessage));

	fMyPopUp->AddSeparatorItem();

	myMessage = new BMessage(M_NAMES_POPUP_DCCSEND);
	fMyPopUp->AddItem(new BMenuItem("DCC Send", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_DCCCHAT);
	fMyPopUp->AddItem(new BMenuItem("DCC Chat", myMessage));

	fCTCPPopUp = new BMenu("CTCP");
	fMyPopUp->AddItem( fCTCPPopUp );

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "ping");
	fCTCPPopUp->AddItem(new BMenuItem("PING", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "version");
	fCTCPPopUp->AddItem(new BMenuItem("VERSION", myMessage));

	fCTCPPopUp->AddSeparatorItem();

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "finger");
	fCTCPPopUp->AddItem(new BMenuItem("FINGER", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "time");
	fCTCPPopUp->AddItem(new BMenuItem("TIME", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "clientinfo");
	fCTCPPopUp->AddItem(new BMenuItem("CLIENTINFO", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_CTCP);
	myMessage->AddString("action", "userinfo");
	fCTCPPopUp->AddItem(new BMenuItem("USERINFO", myMessage));

	fMyPopUp->AddSeparatorItem();

	myMessage = new BMessage(M_NAMES_POPUP_MODE);
	myMessage->AddString("action", "op");
	fMyPopUp->AddItem(new BMenuItem("Op", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_MODE);
	myMessage->AddString("action", "deop");
	fMyPopUp->AddItem(new BMenuItem("Deop", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_MODE);
	myMessage->AddString("action", "voice");
	fMyPopUp->AddItem(new BMenuItem("Voice", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_MODE);
	myMessage->AddString("action", "devoice");
	fMyPopUp->AddItem(new BMenuItem("Devoice", myMessage));

	myMessage = new BMessage(M_NAMES_POPUP_KICK);
	fMyPopUp->AddItem(new BMenuItem("Kick", myMessage));


	// PopUp Menus tend to have be_plain_font
	fMyPopUp->SetFont (be_plain_font);
	fCTCPPopUp->SetFont (be_plain_font);

	fMyPopUp->SetTargetForItems (this);
	fCTCPPopUp->SetTargetForItems (this);

	fActiveTheme->WriteLock();
	fActiveTheme->AddView (this);
	fActiveTheme->WriteUnlock();
}

void
NamesView::DetachedFromWindow (void)
{
	BView::DetachedFromWindow();
	fActiveTheme->WriteLock();
	fActiveTheme->RemoveView (this);
	fActiveTheme->WriteUnlock();
}

void
NamesView::MouseDown (BPoint myPoint)
{
	int32 selected (IndexOf (myPoint));
	bool handled (false);

	if (selected < 0)
	{
		DeselectAll();
		return;
	}

	BMessage *inputMsg (Window()->CurrentMessage());
	int32 mousebuttons (0),
				keymodifiers (0),
				mouseclicks (0);

	inputMsg->FindInt32 ("buttons", &mousebuttons);
	inputMsg->FindInt32 ("modifiers", &keymodifiers);
	inputMsg->FindInt32 ("clicks",	&mouseclicks);

	if (mouseclicks > 1
	&& CurrentSelection(1) <= 0
	&&	mousebuttons == B_PRIMARY_MOUSE_BUTTON
	&& (keymodifiers & B_SHIFT_KEY)	 == 0
	&& (keymodifiers & B_OPTION_KEY)	== 0
	&& (keymodifiers & B_COMMAND_KEY) == 0
	&& (keymodifiers & B_CONTROL_KEY) == 0)
	{
		// user double clicked

		BListItem *item (ItemAt (IndexOf(myPoint)));
		if (item && !item->IsSelected())
		{
			// "double" clicked away from another selection
			Select (IndexOf (myPoint), false);
			fCurrentindex = IndexOf (myPoint);
			fTracking = true;
		}
		else if (item && item->IsSelected())
		{
			// double clicking on a single item
			NameItem *myItem (reinterpret_cast<NameItem *>(item));
			BString theNick (myItem->Name());
			BMessage msg (M_OPEN_MSGAGENT);

			msg.AddString ("nick", theNick.String());
			reinterpret_cast<ChannelAgent *>(Parent()->Parent())->fMsgr.SendMessage (&msg);
		}

		handled = true;
	}

	if (mouseclicks == 1
	&&	CurrentSelection(1) <= 0
	&&	mousebuttons == B_PRIMARY_MOUSE_BUTTON
	&& (keymodifiers & B_SHIFT_KEY)	 == 0
	&& (keymodifiers & B_OPTION_KEY)	== 0
	&& (keymodifiers & B_COMMAND_KEY) == 0
	&& (keymodifiers & B_CONTROL_KEY) == 0)
	{
		// user single clicks
		BListItem *item (ItemAt (IndexOf(myPoint)));
		if (item && !item->IsSelected())
			Select (IndexOf (myPoint), false);

		fTracking = true;
		fCurrentindex = IndexOf (myPoint);
		handled = true;
	}

	if (mouseclicks >= 1
	&&	CurrentSelection(1) >= 0
	&&	mousebuttons == B_PRIMARY_MOUSE_BUTTON
	&& (keymodifiers & B_SHIFT_KEY)	 == 0
	&& (keymodifiers & B_OPTION_KEY)	== 0
	&& (keymodifiers & B_COMMAND_KEY) == 0
	&& (keymodifiers & B_CONTROL_KEY) == 0)
	{
		// user clicks on something in the middle of a sweep selection
		BListItem *item (ItemAt (IndexOf(myPoint)));
		if (item)
			Select (IndexOf (myPoint), false);

		fTracking = true;
		fCurrentindex = IndexOf (myPoint);
		handled = true;
	}

	if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
	&& (keymodifiers & B_SHIFT_KEY)	 == 0
	&& (keymodifiers & B_OPTION_KEY)	== 0
	&& (keymodifiers & B_COMMAND_KEY) == 0
	&& (keymodifiers & B_CONTROL_KEY) == 0)
	{
		// user right clicks - display popup menu
		BListItem *item (ItemAt (IndexOf(myPoint)));
		if (item && !item->IsSelected())
			Select (IndexOf (myPoint), false);

		fMyPopUp->Go (
			ConvertToScreen (myPoint),
			true,
			false,
			ConvertToScreen (ItemFrame (selected)));
		handled = true;
	}
	if (mousebuttons == B_TERTIARY_MOUSE_BUTTON)
		BListView::MouseDown (myPoint);

	fLastSelected = selected;
	if (!handled)
		BListView::MouseDown (myPoint);
}

void
NamesView::MouseUp (BPoint myPoint)
{
 if (fTracking)
	 fTracking = false;

 BListView::MouseUp (myPoint);
}

void
NamesView::MouseMoved (BPoint myPoint, uint32 transitcode, const BMessage *dragMessage)
{
 if ((dragMessage != NULL) && (dragMessage->HasRef("refs")))
 {
	 // make sure the ref isn't a dir
	 entry_ref ref;
	 dragMessage->FindRef("refs", &ref);
	 BDirectory dir (&ref);
	 if (dir.InitCheck() != B_OK)
	 {
		 int32 nameIndex (IndexOf(myPoint));
		 if (nameIndex >= 0)
			 Select(nameIndex);
	 }
 }
 else if (fTracking)
 {
	 if (transitcode == B_INSIDE_VIEW)
	 {
		 BListItem *item = ItemAt (IndexOf(myPoint));
		 if (item)
		 {
			 BListItem *lastitem (NULL);
			 int32 first (CurrentSelection (0));
			 int32 last (first);
			 int32 i (1);
			 while ((last = CurrentSelection(i++)) != -1)
				 lastitem = ItemAt(last);
			 if (lastitem)
				 last = IndexOf(lastitem);
			 else
				 last = first;
			 int32 current (IndexOf (myPoint));

			 if (current >= 0)
			 {
				 if (current < first)
				 {
					 // sweep up
					 Select (current, last, true);
				 }
				 else if (current > last)
				 {
					 // sweep down
					 Select (first, current, true);
				 }
				 else if (fCurrentindex > current)
				 {
					 // backtrack up
					 DeselectExcept (first, current);
				 }
				 else if (fCurrentindex < current)
				 {
					 // backtrack down
					 DeselectExcept (current, last);
				 }
				 fCurrentindex = current;
			 }
		 }
	 }
 }
 else
	 BListView::MouseMoved (myPoint, transitcode, dragMessage);
}

void
NamesView::ClearList (void)
{
	while (CountItems() > 0)
		delete RemoveItem ((int32)0);
}

void
NamesView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_THEME_FOREGROUND_CHANGE:
		{
			int16 which (msg->FindInt16 ("which"));
			bool refresh (false);
			switch (which)
			{
				case C_NAMES_BACKGROUND:
					fActiveTheme->ReadLock();
					SetViewColor (fActiveTheme->ForegroundAt (C_NAMES_BACKGROUND));
					refresh = true;
					fActiveTheme->ReadUnlock();
					break;

				case C_OP:
				case C_VOICE:
				case C_HELPER:
				case C_NAMES_SELECTION:
					refresh = true;
					break;

				default:
					break;
			}
			if (refresh)
				Invalidate();
		}
		break;

		case M_THEME_FONT_CHANGE:
		{
			int16 which (msg->FindInt16 ("which"));
			if (which == F_NAMES)
			{
				fActiveTheme->ReadLock();
				BFont newFont (fActiveTheme->FontAt (F_NAMES));
				fActiveTheme->ReadUnlock();
				SetFont (&newFont);
				for (int32 i = 0; i < CountItems(); i++)
					ItemAt(i)->Update(this, &newFont);
				Invalidate();
			}
		}
		break;

		case B_SIMPLE_DATA:
		{
			if (msg->HasRef("refs"))
			{
				// this only grabs the first ref for now
				// TODO: maybe implement queueing of multiple sends next time
				entry_ref ref;
				msg->FindRef("refs", &ref);
				BDirectory dir (&ref);
				// don't try to dcc send a dir
				if (dir.InitCheck() == B_OK)
					break;
				int32 idx (CurrentSelection());
				if (idx >= 0)
				{
					NameItem *item (dynamic_cast<NameItem *>(ItemAt(idx)));
					BMessage message (M_CHOSE_FILE);
					message.AddString ("nick", item->Name());
					message.AddRef ("refs", &ref);
					ClientAgent *myParent (dynamic_cast<ClientAgent *>(Parent()->Parent()));
					if (myParent)
						myParent->fSMsgr.SendMessage (&message);
				}
			}
		}
		break;

		default:
		{
			BListView::MessageReceived (msg);
		}
		break;
	}
}
