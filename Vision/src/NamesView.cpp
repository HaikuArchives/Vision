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
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 *                 Seth Flaxman
 */
 
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>


#include "Vision.h"
#include "Names.h"

NamesView::NamesView(BRect frame)
	: BListView(
		frame,
		"namesList",
		B_MULTIPLE_SELECTION_LIST,
		B_FOLLOW_LEFT | B_FOLLOW_TOP_BOTTOM)
{
	myPopUp = new BPopUpMenu("User selection", false, false);
	

	BMessage *myMessage = new BMessage (POPUP_WHOIS);
	myPopUp->AddItem(new BMenuItem("Whois", myMessage));
	
	myMessage = new BMessage (M_OPEN_MSGAGENT);
	myPopUp->AddItem(new BMenuItem("Query", myMessage));
	
	myPopUp->AddSeparatorItem();
	
//	myMessage = new BMessage(SEND_ACTION);
//	myPopUp->AddItem(new BMenuItem("DCC Send", myMessage));
//	
//	myMessage = new BMessage(CHAT_ACTION);
//	myPopUp->AddItem(new BMenuItem("DCC Chat", myMessage));
		
	CTCPPopUp = new BMenu("CTCP");
	myPopUp->AddItem( CTCPPopUp );
		
	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "ping");
	CTCPPopUp->AddItem(new BMenuItem("PING", myMessage));

	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "version");
	CTCPPopUp->AddItem(new BMenuItem("VERSION", myMessage));
	
	CTCPPopUp->AddSeparatorItem();
	
	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "finger");
	CTCPPopUp->AddItem(new BMenuItem("FINGER", myMessage));

	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "time");
	CTCPPopUp->AddItem(new BMenuItem("TIME", myMessage));

	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "clientinfo");
	CTCPPopUp->AddItem(new BMenuItem("CLIENTINFO", myMessage));

	myMessage = new BMessage(POPUP_CTCP);
	myMessage->AddString("action", "userinfo");
	CTCPPopUp->AddItem(new BMenuItem("USERINFO", myMessage));

	myPopUp->AddSeparatorItem();
	
	myMessage = new BMessage(POPUP_MODE);
	myMessage->AddString("action", "op");
	myPopUp->AddItem(new BMenuItem("Op", myMessage));

	myMessage = new BMessage(POPUP_MODE);
	myMessage->AddString("action", "deop");
	myPopUp->AddItem(new BMenuItem("Deop", myMessage));
	
	myMessage = new BMessage(POPUP_MODE);
	myMessage->AddString("action", "voice");
	myPopUp->AddItem(new BMenuItem("Voice", myMessage));

	myMessage = new BMessage(POPUP_MODE);
	myMessage->AddString("action", "devoice");
	myPopUp->AddItem(new BMenuItem("Devoice", myMessage));

	myMessage = new BMessage(POPUP_KICK);
	myPopUp->AddItem(new BMenuItem("Kick", myMessage));

	
	// PopUp Menus tend to have be_plain_font
	myPopUp->SetFont (be_plain_font);
	CTCPPopUp->SetFont (be_plain_font);

	BListView::SetFont (vision_app->GetClientFont (F_NAMES));

	textColor   = vision_app->GetColor (C_NAMES);
	opColor     = vision_app->GetColor (C_OP);
	voiceColor  = vision_app->GetColor (C_VOICE);
	bgColor     = vision_app->GetColor (C_NAMES_BACKGROUND);
	ignoreColor = vision_app->GetColor (C_IGNORE);

	SetViewColor (bgColor);
}

NamesView::~NamesView (void)
{
  delete myPopUp;
}

void NamesView::AttachedToWindow (void)
{
  myPopUp->SetTargetForItems(Window());
  CTCPPopUp->SetTargetForItems(Window());
}

void
NamesView::MouseDown (BPoint myPoint)
{
  int32 selected (IndexOf (myPoint));
  bool handled (false);

  if (selected >= 0)
  {
    BMessage *inputMsg (Window()->CurrentMessage());
    int32 mousebuttons (0),
          keymodifiers (0),
          mouseclicks (0);

    inputMsg->FindInt32 ("buttons", &mousebuttons);
    inputMsg->FindInt32 ("modifiers", &keymodifiers);
    inputMsg->FindInt32 ("clicks",  &mouseclicks);

    if (mouseclicks > 1 
    &&  mousebuttons == B_PRIMARY_MOUSE_BUTTON
    && (keymodifiers & B_SHIFT_KEY)   == 0
    && (keymodifiers & B_OPTION_KEY)  == 0
    && (keymodifiers & B_COMMAND_KEY) == 0
    && (keymodifiers & B_CONTROL_KEY) == 0)
    {
      NameItem *myItem (reinterpret_cast<NameItem *>(ItemAt (selected)));
      BString theNick (myItem->Name());
      BMessage msg (M_OPEN_MSGAGENT);
	
      msg.AddString ("nick", theNick.String());
      Window()->PostMessage (&msg);
      handled = true;
    }
	
    if (mousebuttons == B_SECONDARY_MOUSE_BUTTON
    && (keymodifiers & B_SHIFT_KEY)   == 0
    && (keymodifiers & B_OPTION_KEY)  == 0
    && (keymodifiers & B_COMMAND_KEY) == 0
    && (keymodifiers & B_CONTROL_KEY) == 0)
    {
      //Select (selected, false);

      BListItem *item = ItemAt(IndexOf(myPoint));
      if (item && !item->IsSelected())
        Select(IndexOf(myPoint), false);

      myPopUp->Go (
        ConvertToScreen (myPoint),
        true,
        false,
        ConvertToScreen (ItemFrame (selected)));
      handled = true;
    }
  }

  lastSelected = selected; 
  if (!handled)
    BListView::MouseDown (myPoint);
}

void
NamesView::SetColor (int32 which, rgb_color color)
{
  int32 i;
    
  switch (which)
  {
    case C_OP:
    {
      opColor = color;

      // We try to be nice and only Invalidate Op's
      for (i = 0; i < CountItems(); ++i)
      {
        NameItem *item ((NameItem *)ItemAt (i));

        if ((item->Status() & STATUS_OP_BIT) != 0)
          InvalidateItem (i);
      }
      break;
    }

    case C_VOICE:
    {
      voiceColor = color;

      // Again.. nice.
      for (i = 0; i < CountItems(); ++i)
      {
        NameItem *item ((NameItem *)ItemAt (i));

        if ((item->Status() & STATUS_VOICE_BIT) != 0)
          InvalidateItem (i);
      }
      break;
    }

    case C_NAMES:
    {
      textColor = color;

      for (i = 0; i < CountItems(); ++i)
      {
        NameItem *item ((NameItem *)ItemAt (i));

        if ((item->Status() & (STATUS_VOICE_BIT | STATUS_OP_BIT)) == 0)
          InvalidateItem (i);
      }
      break;
    }

    case C_NAMES_BACKGROUND:
    {
      SetViewColor (bgColor = color);
      Invalidate();
      break;
    }

    case C_IGNORE:
    {
      ignoreColor = color;

      for (i = 0; i < CountItems(); ++i)
      {
        NameItem *item ((NameItem *)ItemAt (i));

        if ((item->Status() & (STATUS_IGNORE_BIT)) != 0)
          InvalidateItem (i);
      }
      break;
    }
  }
}

rgb_color
NamesView::GetColor (int32 which) const
{
  rgb_color color (textColor);

  if (which == C_OP)
    color = opColor;

  if (which == C_VOICE)
    color = voiceColor;

  if (which == C_NAMES_BACKGROUND)
    color = bgColor;

  if (which == C_IGNORE)
    color = ignoreColor;

  return color;
}

void
NamesView::SetFont (int32 which, const BFont *font)
{
  if (which == F_NAMES)
  {
    BListView::SetFont (font);
    Invalidate();
  }
}

void
NamesView::ClearList (void)
{
  BListItem *nameItem;

  while ((nameItem = LastItem()))
  {
    RemoveItem (nameItem);
    delete nameItem;
  }
}
