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
 */

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
#include "StringManip.h"
#include "ListAgent.h"
#include "WindowList.h"
#include "ClientWindow.h"

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

ListAgent::ListAgent (
  BRect frame,
  const char *title,
  BMessenger *sMsgr_)
  : BView (
      frame,
      title,
      B_FOLLOW_ALL_SIDES,
      B_WILL_DRAW | B_FRAME_EVENTS),
   sMsgr (sMsgr_),
   listUpdateTrigger (NULL),
  filter (""),
  find (""),
  processing (false)
{
  frame = Bounds();
/*
  mBar = new BMenuBar (frame, "menubar");

  BMenu *menu (new BMenu ("Channel"));

  menu->AddItem (mFind = new BMenuItem (
    "Find" B_UTF8_ELLIPSIS, 
    new BMessage (M_LIST_FIND)));
  menu->AddItem (mFindAgain = new BMenuItem (
    "Find Next", 
    new BMessage (M_LIST_FAGAIN)));
  menu->AddItem (mFilter = new BMenuItem (
    "Filter" B_UTF8_ELLIPSIS,
    new BMessage (M_LIST_FILTER)));
  mBar->AddItem (menu);

  AddChild (mBar);
  mBar->ResizeToPreferred();
  
  frame.top = mBar->Frame().bottom + 1;
*/
  BView *bgView (new BView (
    frame,
    "background",
    B_FOLLOW_ALL_SIDES,
    B_WILL_DRAW));

  bgView->SetViewColor (vision_app->GetColor (C_BACKGROUND));
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
  BStringColumn *channel (new BStringColumn ("Channel", be_plain_font->StringWidth ("Channel") * 2,
    0, frame.Width(), 0));
  listView->AddColumn (channel, 0);
  BIntegerColumn *users (new BIntegerColumn ("Users", be_plain_font->StringWidth ("Users") * 2, 0, frame.Width(), B_ALIGN_CENTER));
  listView->AddColumn (users, 1);
  BStringColumn *topic (new BStringColumn ("Topic", frame.Width() / 2,
    0, frame.Width(), 0));
  listView->AddColumn (topic, 2);
  listView->SetSelectionMode (B_SINGLE_SELECTION_LIST);
  listView->SetSortingEnabled (true);
  listView->SetSortColumn (channel, true, true);
  listView->SetColor (B_COLOR_BACKGROUND, vision_app->GetColor (C_BACKGROUND));
  listView->SetColor (B_COLOR_TEXT, vision_app->GetColor (C_TEXT));
  listView->SetColor (B_COLOR_SELECTION, vision_app->GetColor (C_SELECTION));
  
  listView->SetFont (B_FONT_ROW, vision_app->GetClientFont (F_LISTAGENT));

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
  
  delete sMsgr;
  delete agentWinItem;

  regfree (&re);
  regfree (&fre);
}

void
ListAgent::AttachedToWindow (void)
{
  msgr = BMessenger (this);
}

void
ListAgent::AllAttached (void)
{
  listView->SetTarget(this);
/*
  // target all menu items at the list agent
  // from here since SetTarget(this) fails until
  // AllAttached
  for (int32 i = 0; i < mBar->CountItems(); i++)
  {
    BMenuItem *superitem = (BMenuItem *)mBar->ItemAt(i);
    superitem->SetTarget(this);
    BMenu *submenu (superitem->Submenu());
    for (int32 j = 0; j < submenu->CountItems(); j++)
    {
      BMenuItem *item = (BMenuItem *)submenu->ItemAt(j);
      if (item != NULL)
        item->SetTarget(this);
    }    
  }
*/
}

void
ListAgent::Show (void)
{
  Window()->PostMessage (M_STATUS_CLEAR);
  this->msgr.SendMessage (M_STATUS_ADDITEMS);
  
  BView::Show();
}

void
ListAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_STATE_CHANGE:
      {
        if (msg->HasBool ("font"))
        {
          int32 which (msg->FindInt32 ("which"));
          switch (which)
          {
            case F_LISTAGENT:
              listView->Invalidate();
              break;
          }
        }
      }
      break;
      
    case M_STATUS_ADDITEMS:
      {
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem ("Count: ", ""), true);
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem ("Status: ", ""), true);
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem ("Filter: ", "", STATUS_ALIGN_LEFT), true);
        vision_app->pClientWin()->pStatusView()->SetItemValue (0, "0", false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (1, statusStr.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (2, filter.String(), true);
      }
      break;

    case M_LIST_COMMAND:
      {
        if (!processing)
        {
          BMessage sMsg (M_SERVER_SEND);

          sMsg.AddString ("data", "LIST");

          sMsgr->SendMessage (&sMsg);
          processing = true;

          if (!IsHidden())
            vision_app->pClientWin()->pStatusView()->SetItemValue (0, "0", true);
        }
      }
      break;

    case M_LIST_BEGIN:
      {
        BMessage msg (M_LIST_UPDATE);
        listUpdateTrigger = new BMessageRunner (BMessenger(this), &msg, 3000000); 
        statusStr = "Loading";
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
        statusStr = "Done";
        
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (1, statusStr.String(), true);
/*
        mFind->SetEnabled (true);
        mFindAgain->SetEnabled (true);
        mFilter->SetEnabled (true);
*/
        processing = false;

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
        
        row->SetField (channelField, 0);
        row->SetField (userField, 1);
        row->SetField (topicField, 2);
        listView->AddRow (row);
		
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (0, 0, true);
      }
      
      break;

		case M_LIST_FILTER:
/*			
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
					listView->MakeEmpty();

					BScrollBar *bar (scroller->ScrollBar (B_HORIZONTAL));
					bar->SetRange (0.0, 0.0);
					msgr.SendMessage (M_LIST_DONE);
					processing = true;
				}
			}
			else
			{
				PromptWindow *prompt (new PromptWindow (
					BPoint ((Window()->Frame().right/2) - 100, (Window()->Frame().bottom/2) - 50),
					"  Filter:",
					"List Filter",
					filter.String(),
					this,
					new BMessage (M_LIST_FILTER),
					new RegExValidate ("Filter"),
					true));
				prompt->Show();
			} */
			break;

		case M_LIST_FIND:
/*
			if (msg->HasString ("text"))
			{
				int32 selection (listView->CurrentSelection());
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

				ChannelItem *item;
				int32 i;

				for (i = selection; i < listView->CountItems(); ++i)
				{
					item = (ChannelItem *)listView->ItemAt (i);

					if (regexec (&fre, item->Channel(), 0, 0, 0) != REG_NOMATCH)
						break;
				}

				if (i < listView->CountItems())
				{
					listView->Select (i);
					listView->ScrollToSelection();
				}
				else
				{
					listView->DeselectAll();
				}
			}
			else
			{
				PromptWindow *prompt (new PromptWindow (
					BPoint ((Window()->Frame().right / 2) - 100, (Window()->Frame().bottom/2) - 50),
					"    Find:",
					"Find",
					find.String(),
					this,
					new BMessage (M_LIST_FIND),
					new RegExValidate ("Find:"),
					true));
				prompt->Show();
			} */
			break;

		case M_LIST_FAGAIN:
/*			if (find.Length())
			{
				msg->AddString ("text", find.String());
				msg->what = M_LIST_FIND;
				msgr.SendMessage (msg);
			} */
			break;

		case M_LIST_INVOKE:
		{
			BMessage msg (M_SUBMIT);
			BString buffer;
				
			BRow *row (listView->CurrentSelection());
				
			if (row)
			{
                 buffer = "/JOIN ";
                 buffer += ((BStringField *)row->GetField(0))->String();
                 msg.AddBool ("history", false);
                 msg.AddBool ("clear", false);
                 msg.AddString ("input", buffer.String());
                 sMsgr->SendMessage (&msg);
            }
		}
		break;
		
		case M_CLIENT_QUIT:
		{
		  sMsgr->SendMessage(M_LIST_SHUTDOWN);
		  BMessage deathchant (M_OBITUARY);
          deathchant.AddPointer ("agent", this);
          deathchant.AddPointer ("item", agentWinItem);
          vision_app->pClientWin()->PostMessage (&deathchant);
		}
		break;
		
		default:
			BView::MessageReceived (msg);
	}
}
