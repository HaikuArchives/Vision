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
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
 
#include "MessageAgent.h"
#include "WindowList.h"
#include "ServerAgent.h"
#include "ClientWindow.h"
#include "StatusView.h"
#include "Vision.h"
#include "VTextControl.h"

MessageAgent::MessageAgent (
  BRect &frame_,
  const char *id_,
  int32 sid_,
  const char *serverName_,
  const BMessenger &sMsgr_,
  const char *nick,
  const char *addyString,
  bool chat,
  const char *IP,
  const char *port)

  : ClientAgent (
    id_,
    sid_,
    serverName_,
    nick,
    sMsgr_,
    frame_),

  chatAddy (addyString ? addyString : ""),
  chatee (id_),
  dIP (IP),
  dPort (port),
  dChat (chat),
  dConnected (false)
{
  Init();
}

MessageAgent::~MessageAgent (void)
{
  //
}

void
MessageAgent::Init (void)
{
  // TODO handle & initiate dcc stuff here
}

void
MessageAgent::ChannelMessage (
  const char *msgz,
  const char *nick,
  const char *ident,
  const char *address)
{
//  agentWinItem->SetName (nick);

  ClientAgent::ChannelMessage (msgz, nick, ident, address);
}

void
MessageAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_CHANNEL_MSG:
      {
        const char *nick;

        if (msg->HasString ("nick"))
          msg->FindString ("nick", &nick);
        else
        {
          nick = chatee.String();
          msg->AddString ("nick", nick);
        }
      
        if (myNick.ICompare (nick) != 0)
          agentWinItem->SetName (nick);      
      
        // Send the rest of processing up the chain
        ClientAgent::MessageReceived (msg);
      }
      break;
      
    case M_MSG_WHOIS:
      {
        BMessage send (M_SERVER_SEND);

        AddSend (&send, "WHOIS ");
        AddSend (&send, chatee.String());
        AddSend (&send, " ");
        AddSend (&send, chatee.String());
        AddSend (&send, endl);      
      }

    case M_CHANGE_NICK:
      {
        const char *oldNick, *newNick;

        msg->FindString ("oldnick", &oldNick);
        msg->FindString ("newnick", &newNick);

        if (chatee.ICompare (oldNick) == 0)
        {
          const char *address;
          const char *ident;

          msg->FindString ("address", &address);
          msg->FindString ("ident", &ident);

          BString oldId (id);
          chatee = id = newNick;
        
          agentWinItem->SetName (id.String());

          if (dChat)
            id.Append(" [DCC]");
       
          ClientAgent::MessageReceived (msg);
        }
      
        else if (myNick.ICompare (oldNick) == 0 && !IsHidden())
        {
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);
          ClientAgent::MessageReceived (msg);
        }
      }
      break;

    case M_CLIENT_QUIT:
      {
        ClientAgent::MessageReceived(msg);
        BMessage deathchant (M_OBITUARY);
        deathchant.AddPointer ("agent", this);
        deathchant.AddPointer ("item", agentWinItem);
        vision_app->pClientWin()->PostMessage (&deathchant);
  
        deathchant.what = M_CLIENT_SHUTDOWN;
        sMsgr.SendMessage (&deathchant);
      }
      break;
     
    case M_STATUS_ADDITEMS:
      {
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            0, ""),
          true);
      
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            "Lag: ",
            "",
            STATUS_ALIGN_LEFT),
          true);
      
        vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
            0,
            "",
            STATUS_ALIGN_LEFT),
          true);

        // The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
        // We send true on the last SetItemValue().
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, serverName.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String(), false);
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, myNick.String(), true);
      }        
      break;
    
    default:
      ClientAgent::MessageReceived (msg);
  }
}

void
MessageAgent::Parser (const char *buffer)
{
  if (!dChat)
  {
    BMessage send (M_SERVER_SEND);

    AddSend (&send, "PRIVMSG ");
    AddSend (&send, chatee);
    AddSend (&send, " :");
    AddSend (&send, buffer);
    AddSend (&send, endl);
  }
  else if (dConnected)
  {
//    // TODO Handle DCC stuff
//    BString outTemp (buffer);
//
//    outTemp << "\n";
//    if (send(acceptSocket, outTemp.String(), outTemp.Length(), 0) < 0)
//    {
//      dConnected = false;
//      Display ("DCC chat terminated.\n", 0);
//      return;
//    }
  }
  else
    return;

  BFont msgFont (vision_app->GetClientFont (F_TEXT));
  Display ("<", &myNickColor, &msgFont, vision_app->GetBool ("timestamp"));
  Display (myNick.String(), &nickdisplayColor);
  Display ("> ", &myNickColor);

  BString sBuffer (buffer);
  Display (sBuffer.String(), 0);
  

  Display ("\n", 0);
}

void
MessageAgent::DroppedFile (BMessage *drop)
{
  //
}

void
MessageAgent::TabExpansion (void)
{
  int32 start,
        finish;

  input->TextView()->GetSelection (&start, &finish);

  if (input->TextView()->TextLength()
  &&  start == finish
  &&  start == input->TextView()->TextLength())
  {
    const char *inputText (
                  input->TextView()->Text()
                  + input->TextView()->TextLength());
    const char *place (inputText);


    while (place > input->TextView()->Text())
    {
      if (*(place - 1) == '\x20')
        break;
      --place;
    }

    BString insertion;

    if (!id.ICompare (place, strlen (place)))
    {
      insertion = id;
      insertion.RemoveLast(" [DCC]");
    }
    else if (!myNick.ICompare (place, strlen (place)))
      insertion = myNick;

    if (insertion.Length())
    {
      input->TextView()->Delete (
        place - input->TextView()->Text(),
        input->TextView()->TextLength());

      input->TextView()->Insert (insertion.String());
      input->TextView()->Select (
        input->TextView()->TextLength(),
        input->TextView()->TextLength());
    }
  }
}
