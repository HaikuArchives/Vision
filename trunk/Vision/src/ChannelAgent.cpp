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
 *								 Alan Ellis <alan@cgsoftware.org>
 */

#include <Beep.h>
#include <FilePanel.h>
#include <Catalog.h>
#include <MenuItem.h>
#ifdef __HAIKU__
#include <Notification.h>
#endif
#include <PopUpMenu.h>
#include <Roster.h>
#include <ScrollView.h>

#include <stdio.h>

#include "Names.h"
#include "Theme.h"
#include "ChannelAgent.h"
#include "Vision.h"
#include "StatusView.h"
#include "ClientWindow.h"
#include "Utilities.h"
#include "VTextControl.h"
#include "ChannelOptions.h"
#include "ResizeView.h"

#ifdef USE_INFOPOPPER
#include <infopopper/InfoPopper.h>
#endif

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChannelWindow"

ChannelAgent::ChannelAgent (
	const char *id_,
	const char *serverName_,
	int ircdtype_,
	const char *nick,
	BMessenger &sMsgr_,
	BRect &frame_)

	: ClientAgent (
		id_,
		serverName_,
		nick,
		sMsgr_,
		frame_),

	fChanMode (""),
	fChanLimit (""),
	fChanLimitOld (""),
	fChanKey (""),
	fChanKeyOld (""),
	fLastExpansion (""),
	fUserCount (0),
	fOpsCount (0),
	fIrcdtype (ircdtype_),
	fChanOpt (0)
{
	/*
	 * Function purpose: Consctruct
	 */	
}

ChannelAgent::~ChannelAgent (void)
{
	/*
	 * Function purpose: Clean up
	 */
	 
	fNamesList->ClearList();
	
	// empty recent nick list
	while (fRecentNicks.CountItems() > 0)
		delete fRecentNicks.RemoveItemAt((int32)0);
	
	// empty nick completion list
	while (fCompletionNicks.CountItems() > 0)
		delete fCompletionNicks.RemoveItemAt((int32)0);
}

void
ChannelAgent::AttachedToWindow(void)
{
	/*
	 * Function purpose: Once the BView has been successfully attached,
											 call Init()
	 */
	 
	Init();
	ClientAgent::AttachedToWindow();
}

void
ChannelAgent::Init (void)
{
	/*
	 * Function purpose: Setup everything
	 */
	 
	const BRect namesRect (vision_app->GetRect ("nameListRect"));

	fTextScroll->ResizeTo (
		Frame().Width() - ((namesRect.Width() == 0.0) ? 100 : namesRect.Width()),
		fTextScroll->Frame().Height());
	
	fFrame = Bounds();
	fFrame.left	 = fTextScroll->Frame().right + 6;
	fFrame.right -= B_V_SCROLL_BAR_WIDTH + 1;
	fFrame.bottom = fTextScroll->Frame().bottom - 1;
	
	fNamesList = new NamesView (fFrame);
	
	fNamesScroll = new BScrollView(
		"scroll_names",
		fNamesList,
		B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
		0,
		false,
		true,
		B_PLAIN_BORDER);

	fResize = new ResizeView (fNamesList, BRect (fTextScroll->Frame().right + 1,
		Bounds().top + 1, fTextScroll->Frame().right + 5,
		fTextScroll->Frame().Height()), "resize",
		B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM);

	AddChild (fNamesScroll);

	AddChild (fResize);
	
	BString joinString = B_TRANSLATE("Now talking in %1");
	joinString.ReplaceFirst("%1", fId.String());
	joinString += "\n";

	Display (joinString.String(), C_JOIN);
}

void
ChannelAgent::Show (void)
{
	const BRect namesListRect (vision_app->GetRect ("namesListRect"));
	float difference (fNamesList->Bounds().Width() - namesListRect.Width());
	if (difference != 0.0)
	{
		fResize->MoveBy (difference, 0.0);
		fTextScroll->ResizeBy (difference, 0.0);
		fNamesScroll->ResizeBy (-difference, 0.0);
		fNamesScroll->MoveBy (difference, 0.0);
		Sync();
	}
	
	ClientAgent::Show();
}

int
ChannelAgent::FindPosition (const char *data) const
{
	ASSERT (data != NULL);
	/*
	 * Function purpose: Find the index of nickname {data} in the
	 *									 ChannelAgent's NamesView
	 */
	 
	if (fNamesList == NULL)
		return -1;
	 
	int32 count (fNamesList->CountItems());

	for (int32 i = 0; i < count; ++i)
	{
		NameItem *item (static_cast<NameItem *>(fNamesList->ItemAt (i)));
		
		if (item == NULL)
			continue;
			
		BString nick (item->Name());

		if ((nick[0] == '@' || nick[0] == '+' || nick[0] == '%')
		&&	*data != nick[0])
			nick.Remove (0, 1);

		if ((*data == '@' || *data == '+' || *data == '%')
		&&	 nick[0] != *data)
			++data;

		if (!nick.ICompare (data))
			return i;
	}

	return -1;
}

const NamesView *
ChannelAgent::pNamesList(void) const
{
	return fNamesList;
}

void
ChannelAgent::RemoveNickFromList (BObjectList<BString> &list, const char *data)
{
	int32 count (list.CountItems());
	for (int32 i = 0; i < count; i++)
	{
		if (list.ItemAt(i)->ICompare(data) == 0)
		{
			delete list.RemoveItemAt (i);
			break;
		}
	}
}

void
ChannelAgent::AddUser (const char *nick, const int32 status)
{
	NameItem *compareItem (NULL);
	
	// make sure this nick doesn't already exist in the list
	// can happen since some ircds allow stealth quits that don't
	// broadcast a quit message
	int32 i;
	for (i = 0; i < fNamesList->CountItems(); i++)
	{
		compareItem = static_cast<NameItem *>(fNamesList->ItemAt(i));
		if (compareItem && compareItem->Name().ICompare(nick) == 0)
			return;
	}
	
	fNamesList->AddItem (new NameItem (nick, status));
	fNamesList->SortItems (SortNames);

	++fUserCount;
	BString buffer;
	buffer << fUserCount;
	BString *comparator (NULL);
	
	// check if new nickname matches against tab completion sequence and update nick list
	// if so
	if (fLastExpansion.Length() > 0 && fLastExpansion.ICompare(nick, fLastExpansion.Length()) == 0)
	{
		int32 count (fCompletionNicks.CountItems());
		for (i = count - 1; i >= 0; i--)
		{
			comparator = fCompletionNicks.ItemAt (i);
			if (comparator && comparator->ICompare (nick) < 0)
			{	 
				BString *string (new BString (nick));
				fCompletionNicks.AddItem (string, i+1);
				break;
			}
		}
	}
	
	if (!IsHidden())
		vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
}

bool
ChannelAgent::RemoveUser (const char *data)
{
	ASSERT (data != NULL);
	/*
	 * Function purpose: Remove nickname {data} from the ChannelAgent's
	 *									 NamesView and update the status counts
	 */
	 
	
	if (fNamesList == NULL)
		return false;
	
	// if nickname is present in tab completion lists, remove
	RemoveNickFromList(fRecentNicks, data);
	RemoveNickFromList(fCompletionNicks, data);
	
	int32 myIndex (FindPosition (data));

	if (myIndex >= 0)
	{
		NameItem *item;

		fNamesList->Deselect (myIndex);
		if ((item = static_cast<NameItem *>(fNamesList->RemoveItem (myIndex))) != NULL)
		{
			BString buffer;

			if ((item->Status() & STATUS_OP_BIT) != 0)
			{
				--fOpsCount;
				buffer << fOpsCount;
				
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

				buffer = "";
			}

			--fUserCount;
			buffer << fUserCount;
			if (!IsHidden())
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
				
			delete item;
			return true;
		}
	}
	
	return false;
}

void
ChannelAgent::ChannelMessage (
	const char *msgz,
	const char *nick,
	const char *ident,
	const char *address)
{
	if (nick)
	{
		 int32 count (fRecentNicks.CountItems());
		 if (count > MAX_RECENT_NICKS)
		 {
			 delete fRecentNicks.RemoveItemAt ((int32)0);
		 }
		 // scan for presence of nick in list, and remove duplicate if found
		 RemoveNickFromList (fRecentNicks, nick);
		 fRecentNicks.AddItem (new BString (nick));
	}

	ClientAgent::ChannelMessage (msgz, nick, ident, address);
}

int
ChannelAgent::AlphaSortNames(const BString *name1, const BString *name2)
{
	/*
	 * Function purpose: Help Tab Completion sort nicknames
	 *
	 * uses strict lexicographic order, ignores the op/voice bits
	 */
	 
	 // clunky way to get around C++ warnings re casting from const void * to NameItem *
	 
	// Not sure if this can happen, and we
	// are assuming that if one is NULL
	// we return them as equal.	What if one
	// is NULL, and the other isn't?
	if (!name1
	||	!name2)
		return 0;

	return name1->ICompare (*name2);
}


int
ChannelAgent::SortNames(const void *name1, const void *name2)
{
	/*
	 * Function purpose: Help NamesView::SortItems() sort nicknames
	 *
	 * Order:
	 *	 Channel Ops
	 *	 Channel Helper/HalfOps
	 *	 Voiced
	 *	 Normal User
	 */
	 
	 // clunky way to get around C++ warnings re casting from const void * to NameItem *
	 
	const NameItem *firstPtr (*((NameItem * const *)name1));
	const NameItem *secondPtr (*((NameItem * const *)name2));

	// Not sure if this can happen, and we
	// are assuming that if one is NULL
	// we return them as equal.	What if one
	// is NULL, and the other isn't?
	if (!firstPtr
	||	!secondPtr)
		return 0;

	BString first, second;

	first << (((firstPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (firstPtr)->Status());
	second << (((secondPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (secondPtr)->Status());
	first.Prepend ('0', 10 - first.Length());
	second.Prepend ('0', 10 - second.Length());
	first	+= (firstPtr)->Name();
	second += (secondPtr)->Name();

	return first.ICompare(second);
}

void
ChannelAgent::TabExpansion (void)
{
	/*
	 * Function purpose: Get the characters before the caret's current position,
	 *									 and update the fInput VTextControl with a relevant match
	 *									 from the ChannelAgent's NamesView 
	 */
	 
	int32 start, finish;
	static int32 lastindex;
	static BString lastNick;
	fInput->TextView()->GetSelection (&start, &finish);

	if (fInput->TextView()->TextLength()
	&&	start == finish
	&&	start == fInput->TextView()->TextLength())
	{
		const char *fInputText (fInput->TextView()->Text() + fInput->TextView()->TextLength());
		const char *place (fInputText);

		while (place > fInput->TextView()->Text())
		{
			if (*(place - 1) == '\x20')
				break;

			--place;
		}

		if (fLastExpansion.Length() == 0
		|| fLastExpansion.ICompare(place, fLastExpansion.Length()) != 0
		|| lastNick != place)
		{
			lastindex = 0;
			fLastExpansion = place;

			while (!fCompletionNicks.IsEmpty())
				delete fCompletionNicks.RemoveItemAt((int32)0);
			
			int32 count (fNamesList->CountItems()),
						i (0);
			
			for (i = 0; i < count ; i++)
			{
				BString *name (new BString(static_cast<NameItem *>(fNamesList->ItemAt(i))->Name()));
				if (!(name->ICompare(fLastExpansion.String(), strlen(fLastExpansion.String()))))
					fCompletionNicks.AddItem(name);
				else
					delete name;
			}
			// sort items alphabetically
			fCompletionNicks.SortItems (AlphaSortNames);
			
			count = fRecentNicks.CountItems();
			// parse recent nicks in reverse to ensure that they're pushed onto the completion
			// list in the correct order
			for (i = 0; i < count; i++)
			{
				BString *name (new BString(*fRecentNicks.ItemAt(i)));
				if (!(name->ICompare(fLastExpansion.String(), strlen(fLastExpansion.String()))))
				{
					// parse through list and nuke duplicate if present
					for (int32 j = fCompletionNicks.CountItems() - 1; j >= 0; j--)
					{
						if (!(name->ICompare(*fCompletionNicks.ItemAt (j))))
						{
							delete fCompletionNicks.RemoveItemAt(j);
							break;
						}
					}
					fCompletionNicks.AddItem(name, 0);
				}
				else
					delete name;
			}
		}
		
		// We first check if what the user typed matches the channel
		// If that doesn't match, we check the names
		BString insertion;

		if (!fId.ICompare (place, strlen (place)))
			insertion = fId;
		else
		{
			int32 count = fCompletionNicks.CountItems();
			if (count > 0)
			{
				insertion = *(fCompletionNicks.ItemAt(lastindex++));
		
				if (lastindex == count) lastindex = 0;
					lastNick = insertion;
			}
		}

		if (insertion.Length())
		{
			// check if we are at the beginning of a line
			// (ignoring whitespace). if we are, prepend a colon to the nick being
			// inserted
#if 0
			const char *place2 = place;
			while (place2 > fInput->TextView()->Text())
			{
				--place2;
				if (*place2 != 0x20)
					break;
			}
			if (place2 == fInput->TextView()->Text())
	insertion += ": ";
#endif
			fInput->TextView()->Delete (
				place - fInput->TextView()->Text(),
				fInput->TextView()->TextLength());

			fInput->TextView()->Insert (insertion.String());
			fInput->TextView()->Select (
				fInput->TextView()->TextLength(),
				fInput->TextView()->TextLength());
		}
	}
}


void
ChannelAgent::AddMenuItems (BPopUpMenu *pMenu)
{
		BMenuItem *item (NULL);
		item = new BMenuItem("Channel Options", new BMessage (M_CHANNEL_OPTIONS_SHOW));
		item->SetTarget (this);
		pMenu->AddItem (item);
		pMenu->AddSeparatorItem();
}

void
ChannelAgent::MessageReceived (BMessage *msg)
{
	int32 i (0);
	
	switch (msg->what)
	{
		case M_USER_QUIT:
			{
				const char *nick (NULL);

				msg->FindString ("nick", &nick);
 
				if (RemoveUser (nick))
				{
					BMessage display;
 
					if (msg->FindMessage ("display", &display) == B_NO_ERROR)
						ClientAgent::MessageReceived (&display);
				}
			}
			break;

		case M_USER_ADD:
			{
				const char *nick (NULL);
				bool ignore (false);
				
				int32 iStatus (STATUS_NORMAL_BIT);

				msg->FindString ("nick", &nick);
				msg->FindBool ("ignore", &ignore);
				if (nick == NULL)
				{
					printf("ERROR: attempted to AddUser an empty nick in ChannelAgent, channel: %s\n", fId.String());
					break;
				}

				if (ignore) iStatus |= STATUS_IGNORE_BIT;
				
				AddUser (nick, iStatus);

				BMessage display;
				if (msg->FindMessage ("display", &display) == B_NO_ERROR)
					ClientAgent::MessageReceived (&display);
			}
			break;

		case M_CHANGE_NICK:
			{
				const char *oldNick (NULL), *newNick (NULL);
				NameItem *item (NULL);
				int32 thePos (-1);

				if ((msg->FindString ("oldnick", &oldNick) != B_OK) ||
					(msg->FindString ("newnick", &newNick) != B_OK))
					{
						printf("Error: invalid pointer, ChannelAgent::MessageReceived, M_CHANGE_NICK");
						break;
					}

				if (fMyNick.ICompare (oldNick) == 0 && !IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);

				if (((thePos = FindPosition (oldNick)) >= 0)
				&&	((item = (static_cast<NameItem *>(fNamesList->ItemAt (thePos)))) != 0))
				{
					item->SetName (newNick);
					fNamesList->SortItems (SortNames);
					if (fLastExpansion.ICompare (oldNick, fLastExpansion.Length()) == 0)
					{
						int32 count (fRecentNicks.CountItems());
						BString *name (NULL);
						
						for (i = 0; i < count ; i++)
							if ((name = fRecentNicks.ItemAt (i))->ICompare (oldNick) == 0)
							{
								if (fLastExpansion.ICompare (newNick, fLastExpansion.Length()) == 0)
									name->SetTo (newNick);
								else
									delete fRecentNicks.RemoveItemAt (i);
								break;
							}
						count = fCompletionNicks.CountItems();
						for (i = 0; i < count; i++)
							if ((name = fCompletionNicks.ItemAt (i))->ICompare (oldNick) == 0)
							{
								if (fLastExpansion.ICompare (newNick, fLastExpansion.Length()) == 0)
									name->SetTo (newNick);
								else
									delete fCompletionNicks.RemoveItemAt (i);
								break;
							}
					}
				}
				else
					break;
					
				ClientAgent::MessageReceived (msg);
			}
			break;

		case M_CHANNEL_NAMES:
			{
				for (i = 0; msg->HasString ("nick", i); ++i)
				{
					const char *nick (NULL);
					bool founder (false),
						protect (false),
						op (false),
						voice (false),
						helper (false),
						ignored (false);

					msg->FindString ("nick", i, &nick);
					msg->FindBool ("founder", i, &founder);
					msg->FindBool ("protect", i, &protect);
					msg->FindBool ("op", i, &op);
					msg->FindBool ("voice", i, &voice);
					msg->FindBool ("helper", i, &helper);
					msg->FindBool ("ignored", i, &ignored);
	
					if (FindPosition (nick) < 0)
					{
						int32 iStatus (ignored ? STATUS_IGNORE_BIT : 0);
						
						if (founder)
						{
							++nick;
							++fOpsCount;
							iStatus |= STATUS_FOUNDER_BIT;
						}
						else if (protect)
						{
							++nick;
							++fOpsCount;
							iStatus |= STATUS_PROTECTED_BIT;
						}
						else if (op)
						{
							++nick;
							++fOpsCount;
							iStatus |= STATUS_OP_BIT;
						}
						else if (voice)
						{
							++nick;
							iStatus |= STATUS_VOICE_BIT;
						}
						else if (helper)
						{
							++nick;
							iStatus |= STATUS_HELPER_BIT;
						}
						else
							iStatus |= STATUS_NORMAL_BIT;
 
						fUserCount++;
 
						fNamesList->AddItem (new NameItem (nick, iStatus));
					}
				}
			
				fNamesList->SortItems (SortNames);

				if (!IsHidden())
				{
					BString buffer;
					buffer << fOpsCount;
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

					buffer = "";
					buffer << fUserCount;
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
				}
			}
			break;
		
		case M_RESIZE_VIEW:
			{
					BPoint point;
					msg->FindPoint ("loc", &point);
					point.x -= Frame().left;
					float offset (point.x - fNamesScroll->Frame().left);
					fResize->MoveBy (offset, 0.0);
					fTextScroll->ResizeBy (offset, 0.0);
					fNamesScroll->ResizeBy (-offset, 0.0);
					fNamesScroll->MoveBy (offset, 0.0);
					BRect namesRect (0, 0, fNamesScroll->Bounds().Width(), 0);
					vision_app->SetRect ("namesListRect", namesRect);
			}
			break;

		case M_SERVER_DISCONNECT:
			{
				// clear names list on disconnect
				fNamesList->ClearList();
				fOpsCount = 0;
				fUserCount = 0;
				
				// clear heuristics completion list - this ensures that no stale nicks are left
				// over in it after reconnect -- list will quickly be rebuilt anyhow if there
				// is any conversation whatsoever going on
				while (fRecentNicks.CountItems() > 0)
					delete fRecentNicks.RemoveItemAt((int32)0);
				
			}
			break;
		
		case M_REJOIN:
			{
				const char *newNick (NULL);
				
				if (msg->FindString ("nickname", &newNick) != B_OK)
				{
					printf("Error: ChannelAgent::MessageReceived, M_REJOIN: invalid pointer\n");
					break;
				}

				fMyNick = newNick;	// update nickname (might have changed on reconnect)
				
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String());
				
				BString rejoinString = "[@] ";
				rejoinString += B_TRANSLATE("Attempting to rejoin " B_UTF8_ELLIPSIS);
				rejoinString += "\n";
													
				Display (rejoinString.String(), C_ERROR, C_BACKGROUND, F_SERVER);

				// send join cmd		
				BMessage send (M_SERVER_SEND);	
				AddSend (&send, "JOIN ");
				AddSend (&send, fId);	
				if (fChanKey != "")
				{
					AddSend (&send, " ");
					AddSend (&send, fChanKey);
				}
				AddSend (&send, endl);
			}
			break;
			
		case M_CHANNEL_TOPIC:
			{
				const char *theTopic (NULL);
				BString buffer;

				if (msg->FindString ("topic", &theTopic) != B_OK)
				{
					printf("ChannelAgent::MessageReceived, M_CHANNEL_TOPIC: invalid pointer\n");
					break;
				}
				fTopic = theTopic;
			
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_META, FilterCrap(theTopic, true).String());

				BMessage display;
			 
				if (msg->FindMessage ("display", &display) == B_NO_ERROR)
					ClientAgent::MessageReceived (&display);
			}
			break;

		case M_OPEN_MSGAGENT:
			{
				const char *theNick (NULL);
				msg->FindString("nick", &theNick);
			
				if (theNick == NULL)
				{
					NameItem *myUser;
					int32 pos = fNamesList->CurrentSelection();
					if (pos >= 0)
					{
						myUser = static_cast<NameItem *>(fNamesList->ItemAt (pos));
						BString targetNick = myUser->Name();
						msg->AddString ("nick", targetNick.String());
					}
				}
			
				fSMsgr.SendMessage (msg);
			}
			break;

		case M_CHANNEL_GOT_KICKED:
			{
				const char *theChannel (NULL),
					*kicker (NULL), *rest (NULL);
					
				if ((msg->FindString ("channel", &theChannel) != B_OK) ||
					(msg->FindString ("kicker", &kicker) != B_OK) || 
					(msg->FindString ("rest", &rest) != B_OK))
					{
						printf("Error: ClientAgent::MessageReceived, M_CHANNEL_GOT_KICKED, invalid pointer\n");
						break;
					}		

				BMessage wegotkicked (M_DISPLAY); // "you were kicked"
				BString buffer = "*** ";
				buffer += B_TRANSLATE("You have been kicked from %1 by %2 (%3)");
				buffer.ReplaceFirst("%1", theChannel);
				buffer.ReplaceFirst("%2", kicker);
				buffer.ReplaceFirst("%3", rest);
				buffer += "\n";
				
				PackDisplay (&wegotkicked, buffer.String(), C_QUIT, C_BACKGROUND, F_TEXT);

				// clean up
				fNamesList->ClearList();
				fOpsCount = 0;
				fUserCount = 0;
				
				fMsgr.SendMessage (&wegotkicked);

				BMessage attemptrejoin (M_DISPLAY); // "you were kicked"
				buffer = "*** ";
				buffer += B_TRANSLATE("Attempting to rejoin %1" B_UTF8_ELLIPSIS);
				buffer.ReplaceFirst("%1", theChannel);
				buffer += "\n";
				PackDisplay (&attemptrejoin, buffer.String(), C_QUIT, C_BACKGROUND, F_TEXT);
				fMsgr.SendMessage (&attemptrejoin);

				BMessage send (M_SERVER_SEND);
				AddSend (&send, "JOIN ");
				AddSend (&send, theChannel);
				if (fChanKey != "")
				{
					AddSend (&send, " ");
					AddSend (&send, fChanKey);
				}		 
				AddSend (&send, endl);
			}
			break;

		case M_CHANNEL_MODE:
			{
				ModeEvent (msg);
			}
			break;
		
		case M_CHANNEL_MSG:
			{
				bool hasNick (false);
				BString tempString,
									theNick,
									knownAs;
				msg->FindString ("msgz", &tempString);
				msg->FindString ("nick", &theNick);
				if (theNick != fMyNick)
					FirstKnownAs (tempString, knownAs, &hasNick);
				
				if (IsHidden())
				{
					UpdateStatus((hasNick) ? WIN_NICK_BIT : WIN_NEWS_BIT);
#ifdef __HAIKU__
					if (hasNick)
					{
						if (tempString[0] == '\1')
						{
							tempString.RemoveFirst("\1ACTION ");
							tempString.RemoveLast ("\1");
						}

						BNotification notification(B_INFORMATION_NOTIFICATION);
						notification.SetGroup(BString("Vision"));
						entry_ref ref = vision_app->AppRef();
						notification.SetOnClickFile(&ref);
						notification.SetTitle(fServerName.String());
						BString content;
						content << fId << " - " << theNick << " said: " << tempString;
						notification.SetContent(content);
						notification.Send();
					}
#endif
#ifdef USE_INFOPOPPER
					if (hasNick)
					{
						if (tempString[0] == '\1')
						{
							tempString.RemoveFirst("\1ACTION ");
							tempString.RemoveLast ("\1");
						}

						if (be_roster->IsRunning(InfoPopperAppSig) == true) {
							entry_ref ref = vision_app->AppRef();
							BMessage infoMsg(InfoPopper::AddMessage);
							infoMsg.AddString("appTitle", S_INFOPOPPER_TITLE);
							infoMsg.AddString("title", fServerName.String());
							infoMsg.AddInt8("type", (int8)InfoPopper::Important);
					 
							infoMsg.AddInt32("iconType", InfoPopper::Attribute);
							infoMsg.AddRef("iconRef", &ref);							
					
							BString content;
							content << fId << " - " << theNick << " said: " << tempString;
							infoMsg.AddString("content", content);
					
							BMessenger(InfoPopperAppSig).SendMessage(&infoMsg);
						};
					}
#endif

				}
				else if (hasNick)
					system_beep(kSoundEventNames[(uint32)seNickMentioned]);

				ClientAgent::MessageReceived (msg);
			}
			break;

		case M_CHANNEL_MODES:
			{
				const char *mode (NULL),
					*chan (NULL),
					*msgz (NULL);
					
				if ((msg->FindString ("mode", &mode) != B_OK) ||
					(msg->FindString ("chan", &chan) != B_OK) ||
					(msg->FindString ("msgz", &msgz) != B_OK))
					{
						printf("Error: ChannelAgent::MessageReceived, M_CHANNEL_MODES: invalid pointer\n");
						break;
					}

				if (fId.ICompare (chan) == 0)
				{
					BString realMode (GetWord (mode, 1));
					int32 place (2);
 
					if (realMode.FindFirst ("l") >= 0)
						fChanLimit = GetWord (mode, place++);

					if (realMode.FindFirst ("k") >= 0)
					{
						fChanKey = GetWord (mode, place++);
						
						// u2 may not send the channel key, thats why we stored the /join cmd
						// in a string in ParseCmd
						if (fChanKey == "*" && fIrcdtype == IRCD_UNDERNET)
						{
							BString tempId (fId);
							tempId.Remove (0, 1); // remove any #, &, !, blah.
							
							if (vision_app->pClientWin()->joinStrings.FindFirst (tempId) < 1)
							{
								// can't find the join cmd for this channel in joinStrings!
							}
							else
							{
								BString joinStringsL (vision_app->pClientWin()->joinStrings);
								
								// FindLast to make sure we get the last attempt (user might have
								// tried several keys)
								int32 idPos (joinStringsL.FindLast (tempId));								
								BString tempKeyString;
								joinStringsL.MoveInto (tempKeyString, idPos, joinStringsL.Length());
								
								fChanKey = GetWord (tempKeyString.String(), 2);
							}
						} // end u2-kludge stuff
							
					}
					fChanMode = mode;
					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, fChanMode.String());	
				}
			
				BMessage dispMsg (M_DISPLAY);
				PackDisplay (&dispMsg, msgz, C_OP, C_BACKGROUND, F_TEXT);
				BMessenger display (this);
				display.SendMessage (&dispMsg);
			}
			break;

		case M_NAMES_POPUP_MODE:
			{
				const char *inaction (NULL);
				
				msg->FindString ("action", &inaction);

				int32 count (0), index (0);
			
				BString victims,
								targetNick,
								action (inaction),
								modechar,
								polarity;
			
				NameItem *myUser (NULL);

				/// action ///
				if (action.FindFirst ("voice") >= 0)
					modechar = "v";
				else if (action.FindFirst ("op") >= 0)
					modechar = "o";
				else
					break;

				/// polarity ///
				if (action.FindFirst ("de") >= 0)
					polarity += " -";
				else
					polarity += " +";

				/// iterate ///
				while ((i = fNamesList->CurrentSelection (index++)) >= 0)
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt (i));
					targetNick = myUser->Name();

					victims += " ";
					victims += targetNick;
					count++;
				}


				BString command ("/mode ");
				command += fId;
				command += polarity;

				for (i = 0; i < count; i++)
					command += modechar;

				command += victims;
 
				ParseCmd (command.String());
			}
			break;

		case M_NAMES_POPUP_CTCP:
			{
				const char *inaction (NULL);
				
				msg->FindString ("action", &inaction);

				int32 index (0);
				BString victims,
								targetNick,
								action (inaction);
				NameItem *myUser (NULL);
				action.ToUpper();

				/// iterate ///
				while ((i = fNamesList->CurrentSelection (index++)) >= 0)
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt (i));
					targetNick = myUser->Name();

					victims += targetNick;
					victims += ",";
				}
 
				victims.RemoveLast (",");

				BString command ("/ctcp ");
				command += victims;
				command += " ";
				command += action;

				ParseCmd (command.String());
			}
			break;

		case M_NAMES_POPUP_WHOIS:
			{
				int32 index (0);
				BString victims,
								targetNick;
				NameItem *myUser (NULL);

				/// iterate ///
				while ((i = fNamesList->CurrentSelection (index++)) >= 0)
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt (i));
					targetNick = myUser->Name();

					victims += targetNick;
					victims += ",";
				}

				victims.RemoveLast (",");
	
				BString command ("/whois ");
				command += victims;

				ParseCmd (command.String());
			}
			break;
		
		case M_NAMES_POPUP_NOTIFY:
			{
				int32 index (0);
				BString victims,
								targetNick;
				NameItem *myUser (NULL);

				/// iterate ///
				while ((i = fNamesList->CurrentSelection (index++)) >= 0)
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt (i));
					targetNick = myUser->Name();

					victims += targetNick;
					victims += " ";
				}

				victims.RemoveLast (",");
	
				BString command ("/notify ");
				command += victims;

				ParseCmd (command.String());

			}
			break;

		case M_NAMES_POPUP_DCCCHAT: 
			{ 
				int32 index (0); 
				BString targetNick; 
				NameItem *myUser (NULL); 

				/// iterate /// 
				while ((i = fNamesList->CurrentSelection(index++)) >= 0) 
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt(i)); 
					targetNick = myUser->Name(); 
 
					 BString command ("/dcc chat "); 
					 command += targetNick; 

					 ParseCmd (command.String()); 
				 } 
			 } 
			 break;
		 
		case M_NAMES_POPUP_DCCSEND:
			{
				int32 index (0); 
				BString targetNick; 
				NameItem *myUser (NULL); 

				/// iterate /// 
				while ((i = fNamesList->CurrentSelection(index++)) >= 0) 
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt(i)); 
					targetNick = myUser->Name(); 
 
					 BString command ("/dcc send "); 
					 command += targetNick; 

					 ParseCmd (command.String()); 
				 } 
			}
			break;

		case M_NAMES_POPUP_KICK:
			{
				int32 index (0);
				BString targetNick,
								kickMsg (vision_app->GetCommand (CMD_KICK));
				NameItem *myUser (NULL); 

				/// iterate ///
				while ((i = fNamesList->CurrentSelection(index++)) >= 0)
				{ 
					myUser = static_cast<NameItem *>(fNamesList->ItemAt(i));
					targetNick = myUser->Name();
 
					 BString command ("/kick ");
					 command += targetNick;
					 command += " ";
					 command += kickMsg;

					 ParseCmd (command.String());
				 }
			 }
			 break;

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "StatusBar"

		 case M_STATUS_ADDITEMS:
			 {
			 	BString statusString;
				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 0, ""), true);
		
				 statusString = B_TRANSLATE("Lag");
				 statusString += ": ";
				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 statusString.String(), "", STATUS_ALIGN_LEFT), true);

				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 0, "", STATUS_ALIGN_LEFT), true);

				 statusString = B_TRANSLATE("Users");
				 statusString += ": ";
				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 statusString.String(), ""), true);

				 statusString = B_TRANSLATE("Ops");
				 statusString += ": ";
				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 statusString.String(), ""), true);

				 statusString = B_TRANSLATE("Modes");
				 statusString += ": ";
				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 statusString.String(), ""), true);

				 vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
					 "", "", STATUS_ALIGN_LEFT), true);

				 // The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
				 // We send true on the last SetItemValue().
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, fServerName.String(), false);
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), false);
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String(), false);
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, fChanMode.String(), false);
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_META, FilterCrap(fTopic.String(), true).String());

				 BString buffer;
				 buffer << fUserCount;
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String(), false);
				 buffer = "";
				 buffer << fOpsCount;
				 vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String(), true);
			 }
			 break;

		 case M_CHANNEL_OPTIONS_SHOW:
			 {
				 if (fChanOpt)
					 fChanOpt->Activate();
				 else
				 {
					 fChanOpt = new ChannelOptions (fId.String(), this);
					 fChanOpt->Show();
				 }
			 }
			 break;

		 case M_CHANNEL_OPTIONS_CLOSE:
			 {
				 fChanOpt = NULL;
			 }
			 break;


		 case M_CLIENT_QUIT:
			 {
				 if ((msg->HasBool ("vision:part") && msg->FindBool ("vision:part"))
				 ||	(msg->HasBool ("vision:winlist") && msg->FindBool ("vision:winlist")))
				 {
					 BMessage send (M_SERVER_SEND);
					 AddSend (&send, "PART ");
					 AddSend (&send, fId);
					 if (msg->HasString ("vision:partmsg"))
					 {
						 AddSend (&send, " :");
						 AddSend (&send, msg->FindString("vision:partmsg"));
					 }
					 AddSend (&send, endl);
				 }	
				 ClientAgent::MessageReceived(msg);
			 }
			 break;

		 default:
			ClientAgent::MessageReceived (msg);
	}
}

void
ChannelAgent::Parser (const char *buffer)
{
	/*
	 * Function purpose: Send the text in {buffer} to the server
	 */
	 
	fLastExpansion = "";	// used by ChannelAgent::TabExpansion()

	BMessage send (M_SERVER_SEND);

	AddSend (&send, "PRIVMSG ");
	AddSend (&send, fId);
	AddSend (&send, " :");

	Display ("<", C_MYNICK);
	Display (fMyNick.String(), C_NICKDISPLAY);
	Display ("> ", C_MYNICK);	
	
	BString sBuffer (buffer);
	
	AddSend (&send, sBuffer.String());
	AddSend (&send, endl);
	
	Display (sBuffer.String());
	Display ("\n");
}

void
ChannelAgent::UpdateMode(char theSign, char theMode)
{
	char modeString[2]; // necessary C-style string
	memset(modeString, 0, sizeof(modeString));
	sprintf (modeString, "%c", theMode);
	BString myTemp;
	
	if (theSign == '-')
	{
		switch (theMode)
		{
			case 'l':
				{
					myTemp = fChanLimit;
					myTemp.Append (" ");
					fChanMode.RemoveLast (myTemp);
					myTemp = fChanLimit;
					myTemp.Prepend (" ");
					fChanMode.RemoveLast (myTemp);
					fChanMode.RemoveLast (fChanLimit);
				}
				break;

			case 'k':
				{
					myTemp = fChanKey;
					myTemp.Append(" ");
					fChanMode.RemoveLast (myTemp);
					myTemp = fChanKey;
					myTemp.Prepend (" ");
					fChanMode.RemoveLast (myTemp);
					fChanMode.RemoveLast (fChanKey);
				}
				break;
		}
		
		fChanMode.RemoveFirst (modeString);
	}
	else
	{
		BString theReal (GetWord(fChanMode.String(), 1)),
						theRest (RestOfString(fChanMode.String(), 2));
		theReal.RemoveFirst(modeString);
		theReal.Append(modeString);
		BString tempString(theReal);
		if (theRest != "-9z99")
		{
			tempString += " ";
			tempString += theRest;
		}

		if (theMode == 'l')
		{
			if (fChanLimitOld != "")
			{
				BString theOld (" ");
				theOld += fChanLimitOld;
				tempString.RemoveFirst (theOld);
			}
			
			tempString.Append (" ");
			tempString.Append (fChanLimit);
			fChanLimitOld = fChanLimit;
		}
		else if (theMode == 'k')
		{
			if (fChanKeyOld != "")
			{
				BString theOld (" ");
				theOld += fChanKeyOld;
				tempString.RemoveFirst (theOld);
			}
			
			tempString.Append (" ");
			tempString.Append (fChanKey);
			fChanKeyOld = fChanKey;
		}
		
		fChanMode = tempString;
	}

	if (!IsHidden())
		vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, fChanMode.String());
}

void
ChannelAgent::ModeEvent (BMessage *msg)
{
	int32 modPos (0), targetPos (1);
	const char *mode (0), *target (0), *theNick (0);
	char theOperator (0);
	bool hit (false);

	// TODO Change Status to bitmask -- Too hard this way
	msg->FindString ("mode", &mode);
	msg->FindString ("target", &target);
	msg->FindString ("nick", &theNick);

	BString buffer,
					targetS (target);

	buffer = "*** ";
	if (targetS != "-9z99")
	{
		buffer += B_TRANSLATE("%1 set mode %2 %3");
	}
	else
	{
		buffer += B_TRANSLATE("%1 set mode %2");	
	}
	buffer.ReplaceFirst("%1", theNick);
	buffer.ReplaceFirst("%2", mode);
	buffer.ReplaceFirst("%3", targetS);
	buffer += "\n";

	BMessenger display (this);

	BMessage modeMsg (M_DISPLAY);
	PackDisplay (&modeMsg, buffer.String(), C_OP, C_BACKGROUND, F_TEXT);
	display.SendMessage (&modeMsg);


	// at least one
	if (mode && *mode && *(mode + 1))
		theOperator = mode[modPos++];

	while (theOperator && mode[modPos])
	{
		char theModifier (mode[modPos]);

		if (theModifier == 'o'
		||	theModifier == 'v'
		||	theModifier == 'h')
		{
			BString myTarget (GetWord (target, targetPos++));
			NameItem *item;
			int32 pos;

			if ((pos = FindPosition (myTarget.String())) < 0
			||	(item = static_cast<NameItem *>(fNamesList->ItemAt (pos))) == 0)
			{
				printf("[ERROR] Couldn't find %s in NamesView\n", myTarget.String());
				return;
			}

			int32 iStatus (item->Status());

			if (theOperator == '+' && theModifier == 'o')
			{
				hit = true;

				if ((iStatus & STATUS_OP_BIT) == 0)
				{
					item->SetStatus ((iStatus & ~STATUS_NORMAL_BIT) | STATUS_OP_BIT);
					++fOpsCount;

					buffer = "";
					buffer << fOpsCount;
					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());
				}
			}

			else if (theModifier == 'o')
			{
				hit = true;

				if ((iStatus & STATUS_OP_BIT) != 0)
				{
					iStatus &= ~STATUS_OP_BIT;
					if ((iStatus & STATUS_VOICE_BIT) == 0)
						iStatus |= STATUS_NORMAL_BIT;
					item->SetStatus (iStatus);
					--fOpsCount;

					buffer = "";
					buffer << fOpsCount;
					
					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());
				}
			}

			if (theOperator == '+' && theModifier == 'v')
			{
				hit = true;

				item->SetStatus ((iStatus & ~STATUS_NORMAL_BIT) | STATUS_VOICE_BIT);
			}
			else if (theModifier == 'v')
			{
				hit = true;

				iStatus &= ~STATUS_VOICE_BIT;
				if ((iStatus & STATUS_OP_BIT) == 0)
					iStatus |= STATUS_NORMAL_BIT;
				item->SetStatus (iStatus);
			}

			if (theOperator == '+' && theModifier == 'h')
			{
				hit = true;
	 
				item->SetStatus ((iStatus & ~STATUS_NORMAL_BIT) | STATUS_HELPER_BIT);
			}
			else if (theModifier == 'h')
			{
				hit = true;
	 
				iStatus &= ~STATUS_HELPER_BIT;
				if ((iStatus & STATUS_HELPER_BIT) == 0)
					iStatus |= STATUS_NORMAL_BIT;
				item->SetStatus (iStatus);
			}
		}
		else if (theModifier == 'l' && theOperator == '-')
		{
			BString myTarget (GetWord (target, targetPos++));
			UpdateMode ('-', 'l');
			fChanLimit = "";
		}
		else if (theModifier == 'l')
		{
			BString myTarget (GetWord (target, targetPos++));
			fChanLimitOld = fChanLimit;
			fChanLimit = myTarget;
			UpdateMode ('+', 'l');
		}
		else if (theModifier == 'k' && theOperator == '-')
		{
			UpdateMode('-', 'k');
			fChanKey = "";
		}
		else if (theModifier == 'k')
		{
			BString myTarget (GetWord (target, targetPos++));
			fChanKeyOld = fChanKey;
			fChanKey = myTarget;
			UpdateMode ('+', 'k');
		}
		else if (theModifier == 'b' || theModifier == 'a' || theModifier == 'q') 
		{
			// dont do anything else
		}
		else
		{
			UpdateMode (theOperator, theModifier);
		}

		++modPos;
		if (mode[modPos] == '+'
		||	mode[modPos] == '-')
			theOperator = mode[modPos++];
	}
 
	if (hit)
	{
		fNamesList->SortItems (SortNames);
		fNamesList->Invalidate();
	}
}
