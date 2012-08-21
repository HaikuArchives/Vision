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
 */

#include <Catalog.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <stdio.h>
#include <stdlib.h>

#include "ColumnTypes.h"
#include "ColumnListView.h"
#include "Prompt.h"
#include "StatusView.h"
#include "Vision.h"
#include "Utilities.h"
#include "ListAgent.h"
#include "WindowList.h"
#include "ClientWindow.h"
#include "Theme.h"

/*
	-- #beos was here --
	<kurros> regurg, fondle my buttocks
	<regurg> Your?
	<kurros> regurg, yes. mine
	<regurg> I understand.
	<kurros> regurg, no. i don't think you do
	<regurg> Does this make you feel unhappy? 
	<Brazilian> hmm, eliza is just broke
	<kurros> regurg, i just want my buttocks fondled
	<regurg> Why do you say your?
	<Brazilian> the whole string ("fondle by buttocks") is getting passed
	<kurros> regurg, repair thyself
	<regurg> Earlier you said your .
	<kurros> regurg, get off it. don't worry about it anymore
	<regurg> But your .
*/


const int32 LIST_BATCH_SIZE = 75;

ListAgent::ListAgent (
	BRect frame,
	const char *title,
	BMessenger *sMsgr_)
	: BView (
			frame,
			title,
			B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW | B_FRAME_EVENTS),
	 activeTheme (vision_app->ActiveTheme()),
	 fSMsgr (sMsgr_),
	 listUpdateTrigger (NULL),
	filter (""),
	find (""),
	processing (false)
{
	frame = Bounds();

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChannelListMenu"
	
	listMenu = new BMenu (B_TRANSLATE("Channels"));

	BString itemText = B_TRANSLATE("Find" B_UTF8_ELLIPSIS);
	listMenu->AddItem (mFind = new BMenuItem (
		itemText.String(), 
		new BMessage (M_LIST_FIND)));
	listMenu->AddItem (mFindAgain = new BMenuItem (
		B_TRANSLATE("Find Next"), 
		new BMessage (M_LIST_FAGAIN)));
	itemText = B_TRANSLATE("Filter" B_UTF8_ELLIPSIS);
	listMenu->AddItem (mFilter = new BMenuItem (
		itemText.String(),
		new BMessage (M_LIST_FILTER)));
	
	mFind->SetEnabled (false);
	mFindAgain->SetEnabled (false);
	mFilter->SetEnabled (false);

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChannelListWindow"	

	BView *bgView (new BView (
		frame,
		"background",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW));

	bgView->SetViewColor (activeTheme->ForegroundAt (C_BACKGROUND));
	AddChild (bgView);

	frame = bgView->Bounds().InsetByCopy (1, 1);
	
	listView = new BColumnListView (
		frame,
		"list",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE);

	listView->SetInvocationMessage (new BMessage (M_LIST_INVOKE));
	bgView->AddChild (listView);
	listView->MakeFocus (true);
	listView->SetTarget(this);
	BString columnLabel = B_TRANSLATE("Channel");
	channelColumn = new BStringColumn (columnLabel.String(), be_plain_font->StringWidth (columnLabel.String()) * 2,
		0, frame.Width(), 0);
	listView->AddColumn (channelColumn, 0);
	columnLabel = B_TRANSLATE("Users");
	usersColumn = new BIntegerColumn (columnLabel.String(), be_plain_font->StringWidth (columnLabel.String()) * 2, 0, frame.Width(), B_ALIGN_CENTER);
	listView->AddColumn (usersColumn, 1);
	columnLabel = B_TRANSLATE("Topic");
	topicColumn = new BStringColumn (columnLabel.String(), frame.Width() / 2,
		0, frame.Width(), 0);
	listView->AddColumn (topicColumn, 2);
	listView->SetSelectionMode (B_SINGLE_SELECTION_LIST);
	activeTheme->ReadLock();
	listView->SetColor (B_COLOR_BACKGROUND, activeTheme->ForegroundAt (C_BACKGROUND));
	listView->SetColor (B_COLOR_TEXT, activeTheme->ForegroundAt (C_TEXT));
	listView->SetColor (B_COLOR_SELECTION, activeTheme->ForegroundAt (C_SELECTION));
	listView->SetFont (B_FONT_ROW, &activeTheme->FontAt (F_LISTAGENT));
	activeTheme->ReadUnlock();
	memset (&re, 0, sizeof (re));
	memset (&fre, 0, sizeof (fre));
}

ListAgent::~ListAgent (void)
{
	BRow *row (NULL);
	if (listUpdateTrigger)
		delete listUpdateTrigger;

	while (listView->CountRows() > 0)
	{
		row = listView->RowAt (0);
		listView->RemoveRow (row);
		delete row;
	}
	
	while (hiddenItems.CountItems() > 0)
		delete hiddenItems.RemoveItemAt ((int32)0);
	
	delete fSMsgr;
	delete fAgentWinItem;

	regfree (&re);
	regfree (&fre);
}

void
ListAgent::AttachedToWindow (void)
{
	fMsgr = BMessenger (this);
	listView->SetTarget(this);
}


void
ListAgent::Show (void)
{
	Window()->PostMessage (M_STATUS_CLEAR);
	this->fMsgr.SendMessage (M_STATUS_ADDITEMS);
	vision_app->pClientWin()->AddMenu (listMenu);
	listMenu->SetTargetForItems (this);
	const BRect *agentRect (dynamic_cast<ClientWindow *>(Window())->AgentRect());
	
	if (*agentRect != Frame())
	{
		ResizeTo (agentRect->Width(), agentRect->Height());
		MoveTo (agentRect->left, agentRect->top);
	}

	BView::Show();
}

void
ListAgent::Hide (void)
{
	vision_app->pClientWin()->RemoveMenu (listMenu);
	BView::Hide();
}

void
ListAgent::AddBatch (void)
{
	// make sure you call this from a locked looper
	BRow *row (NULL);
	Window()->DisableUpdates();
	while ((row = fBuildList.RemoveItemAt ((int32)0)) != NULL)
		listView->AddRow (row);
	Window()->EnableUpdates();
				
	BString cString;
	cString << listView->CountRows();
	if (!IsHidden())
		vision_app->pClientWin()->pStatusView()->SetItemValue (0, cString.String(), true);
}

void
ListAgent::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_THEME_FONT_CHANGE:
			{
				int32 which (msg->FindInt16 ("which"));
				if (which == F_LISTAGENT)
				{
					activeTheme->ReadLock();
					listView->SetFont (B_FONT_ROW, &activeTheme->FontAt (F_LISTAGENT));
					activeTheme->ReadUnlock();
					listView->Invalidate();
				}
			}
			break;
			
		case M_THEME_FOREGROUND_CHANGE:
			{
				int32 which (msg->FindInt16 ("which"));
				bool refresh (false);
				switch (which)
				{
					case C_BACKGROUND:
						activeTheme->ReadLock();
						listView->SetColor (B_COLOR_BACKGROUND, activeTheme->ForegroundAt (C_BACKGROUND));
						activeTheme->ReadUnlock();
						refresh = true;
						break;
						
					case C_TEXT:
						activeTheme->ReadLock();
						listView->SetColor (B_COLOR_TEXT, activeTheme->ForegroundAt (C_TEXT));
						activeTheme->ReadUnlock();
						refresh = true;
						break;

					case C_SELECTION:						 
						activeTheme->ReadLock();
						listView->SetColor (B_COLOR_SELECTION, activeTheme->ForegroundAt (C_SELECTION));
						activeTheme->ReadUnlock();
						refresh = true;
						break;
						
					default:
						break;
				}
				if (refresh)
					Invalidate();
			}
			break;
			
		case M_STATUS_ADDITEMS:
			{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChannelListStatusBarItems"				
				BString statusLabel = B_TRANSLATE("Count");
				statusLabel += ": ";
				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (statusLabel.String(), ""), true);
				statusLabel = B_TRANSLATE("Status");
				statusLabel += ": ";				
				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (statusLabel.String(), ""), true);
				statusLabel = B_TRANSLATE("Filter");
				statusLabel += ": ";				
				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (statusLabel.String(), "", STATUS_ALIGN_LEFT), true);
 
				BString cString;
				cString << listView->CountRows();
				vision_app->pClientWin()->pStatusView()->SetItemValue (0, cString.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (1, statusStr.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (2, filter.String(), true);
			}
			break;

		case M_LIST_COMMAND:
			{
				if (!processing)
				{
					BMessage sMsg (M_SERVER_SEND);
					
					BString command ("LIST");
					
					BString params (msg->FindString ("cmd"));
					if (params != "-9z99")
					{
						command.Append (" ");
						command.Append (params);
					}

					sMsg.AddString ("data", command.String());

					fSMsgr->SendMessage (&sMsg);
					processing = true;

					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (0, "0", true);
				}
			}
			break;

		case M_LIST_BEGIN:
			{
				BMessage message (M_LIST_UPDATE);
				listUpdateTrigger = new BMessageRunner (BMessenger(this), &message, 3000000); 
				statusStr = B_TRANSLATE("Loading");
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (1, statusStr.String(), true);
			}
			break;
			
		case M_LIST_DONE:
			{
				if (listUpdateTrigger)
				{
					delete listUpdateTrigger;
					listUpdateTrigger = 0;
				}
				statusStr = B_TRANSLATE("Done");

				listView->SetSortingEnabled (true);
				listView->SetSortColumn (channelColumn, true, true);
				
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (1, statusStr.String(), true);

				mFind->SetEnabled (true);
				mFindAgain->SetEnabled (true);
				mFilter->SetEnabled (true);

				processing = false;
				
				// empty out any remaining channels that fell below the batch cut off
				AddBatch();

				BString cString;
				cString << listView->CountRows();
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (0, cString.String(), true);
			}
			break;

		case M_LIST_EVENT:
			{
				const char *channel, *users, *topic;

				msg->FindString ("channel", &channel);
				msg->FindString ("users", &users);
				msg->FindString ("topic", &topic);
				
				BRow *row (new BRow ());
				
				
				BStringField *channelField (new BStringField (channel));
				BIntegerField *userField (new BIntegerField (atoi(users)));
				BStringField *topicField (new BStringField (topic));
				
				row->SetField (channelField, channelColumn->LogicalFieldNum());
				row->SetField (userField, usersColumn->LogicalFieldNum());
				row->SetField (topicField, topicColumn->LogicalFieldNum());

				fBuildList.AddItem (row);
				
				if (fBuildList.CountItems() == LIST_BATCH_SIZE)
					AddBatch();
			}
			break;


		case M_LIST_FILTER:
			if (msg->HasString ("text"))
			{
				const char *buffer;

				msg->FindString ("text", &buffer);
				if (filter != buffer)
				{
					filter = buffer;

					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (2, filter.String(), true);

					regfree (&re);
					memset (&re, 0, sizeof (re));
					regcomp (
						&re,
						filter.String(),
						REG_EXTENDED | REG_ICASE | REG_NOSUB);
					
					BRow *currentRow;	
					BStringField *channel, *topic;

					while (hiddenItems.CountItems() != 0)
					{
						currentRow = hiddenItems.RemoveItemAt ((int32)0);
						listView->AddRow (currentRow);
					}

					if (filter != NULL)
					{
						int32 k (0);
																			
						while (k < listView->CountRows())
						{
							currentRow = listView->RowAt (k);
							channel = (BStringField *)currentRow->GetField (0);
							topic = (BStringField *)currentRow->GetField (2);
							if ((regexec (&re, channel->String(), 0, 0, 0) != REG_NOMATCH)
								|| (regexec (&re, topic->String(), 0, 0, 0) != REG_NOMATCH))
							{
								k++;
								continue;
							}
							else
							{
								listView->RemoveRow (currentRow);
								hiddenItems.AddItem (currentRow);
							}
						}
					}
					fMsgr.SendMessage (M_LIST_DONE);
					processing = true;
				}
			}
			else
			{
				PromptWindow *prompt (new PromptWindow (
					BPoint ((Window()->Frame().right/2) - 100, (Window()->Frame().bottom/2) - 50),
					"	Filter:",
					"List Filter",
					filter.String(),
					this,
					new BMessage (M_LIST_FILTER),
					new RegExValidate ("Filter"),
					true));
				prompt->Show();
			}
			break;

		case M_LIST_FIND:
			if (msg->HasString ("text"))
			{
				int32 selection (listView->IndexOf(listView->CurrentSelection()));
				const char *buffer;

				msg->FindString ("text", &buffer);

				if (strlen (buffer) == 0)
				{
					find = buffer;
					break;
				}

				if (selection < 0)
				{
					selection = 0;
				}
				else
				{
					++selection;
				}

				if (find != buffer)
				{
					regfree (&fre);
					memset (&fre, 0, sizeof (fre));
					regcomp (
						&fre,
						buffer,
						REG_EXTENDED | REG_ICASE | REG_NOSUB);
					find = buffer;
				}

				BStringField *field;
				int32 i;
				for (i = selection; i < listView->CountRows(); ++i)
				{
					field = (BStringField *)listView->RowAt (i)->GetField (0);

					if (regexec (&fre, field->String(), 0, 0, 0) != REG_NOMATCH)
						break;
				}

				if (i < listView->CountRows())
				{
					BRow* row = listView->RowAt (i);
					listView->DeselectAll();
					listView->AddToSelection (row);
					listView->ScrollTo(row);
					listView->Refresh();
				}
				else
				{
					listView->DeselectAll();
				}
			}
			else
			{
				BString label = B_TRANSLATE("Find");
				label += ":";
				PromptWindow *prompt (new PromptWindow (
					BPoint ((Window()->Frame().right / 2) - 100, (Window()->Frame().bottom/2) - 50),
					label.String(),
					B_TRANSLATE("Find"),
					find.String(),
					this,
					new BMessage (M_LIST_FIND),
					new RegExValidate ("Find:"),
					true));
				prompt->Show();
			} 
			break;

		case M_LIST_FAGAIN:
			if (find.Length())
			{
				msg->AddString ("text", find.String());
				msg->what = M_LIST_FIND;
				fMsgr.SendMessage (msg);
			}
			break;

		case M_LIST_INVOKE:
		{
			BMessage message (M_SUBMIT);
			BString buffer;
				
			BRow *row (listView->CurrentSelection());
				
			if (row)
			{
								 buffer = "/JOIN ";
								 buffer += ((BStringField *)row->GetField(0))->String();
								 message.AddBool ("history", false);
								 message.AddBool ("clear", false);
								 message.AddString ("input", buffer.String());
								 fSMsgr->SendMessage (&message);
						}
		}
		break;
		
		case M_CLIENT_QUIT:
		{
			fSMsgr->SendMessage(M_LIST_SHUTDOWN);
			BMessage deathchant (M_OBITUARY);
					deathchant.AddPointer ("agent", this);
					deathchant.AddPointer ("item", fAgentWinItem);
					vision_app->pClientWin()->PostMessage (&deathchant);
		}
		break;
		
		default:
			BView::MessageReceived (msg);
	}
}
