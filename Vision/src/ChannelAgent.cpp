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

#include <ScrollView.h>
#include <FilePanel.h>

#include <stdio.h>

#include "Names.h"
#include "Theme.h"
#include "ChannelAgent.h"
#include "Vision.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "ClientWindow.h"
#include "StringManip.h"
#include "VTextControl.h"
#include "ChannelOptions.h"
#include "ResizeView.h"


ChannelAgent::ChannelAgent (
  const char *id_,
  int32 sid_,
  const char *serverName_,
  int ircdtype_,
  const char *nick,
  BMessenger &sMsgr_,
  BRect &frame_)

  : ClientAgent (
    id_,
    sid_,
    serverName_,
    nick,
    sMsgr_,
    frame_),

  chanMode (""),
  chanLimit (""),
  chanLimitOld (""),
  chanKey (""),
  chanKeyOld (""),
  lastExpansion (""),
  userCount (0),
  opsCount (0),
  ircdtype (ircdtype_),
  chanOpt (0)

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
   
  namesList->ClearList();
  
}

void
ChannelAgent::AttachedToWindow(void)
{
  /*
   * Function purpose: Once the BView has been successfully attached,
                       call Init()
   */
   
  Init();
}

void
ChannelAgent::Init (void)
{
  /*
   * Function purpose: Setup everything
   */
   
  const BRect namesRect (vision_app->GetRect ("nameListRect"));

  textScroll->ResizeTo (
    Frame().Width() - ((namesRect.Width() == 0.0) ? 100 : namesRect.Width()),
    textScroll->Frame().Height());
  
  frame = Bounds();
  frame.left   = textScroll->Frame().right + 4;
  frame.right -= B_V_SCROLL_BAR_WIDTH + 1;
  frame.bottom = textScroll->Frame().bottom - 1;
  
  namesList = new NamesView (frame);
  
  activeTheme->AddView (namesList);
  
  namesScroll = new BScrollView(
    "scroll_names",
    namesList,
    B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
    0,
    false,
    true,
    B_PLAIN_BORDER);

  resize = new ResizeView (namesList, BRect (textScroll->Frame().right + 1,
    Bounds().top + 1, textScroll->Frame().right + 3, textScroll->Frame().Height()), "resize", B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM);

  AddChild (namesScroll);

  AddChild (resize);

  Display (S_CHANNEL_INIT, C_JOIN);
  Display (id.String(), C_JOIN);
  Display ("\n", C_JOIN);
}

int
ChannelAgent::FindPosition (const char *data)
{
  ASSERT (data != NULL);
  /*
   * Function purpose: Find the index of nickname {data} in the
   *                   ChannelAgent's NamesView
   */
   
  if (namesList == NULL)
    return -1;
   
  int32 count (namesList->CountItems());

  for (int32 i = 0; i < count; ++i)
  {
    NameItem *item ((NameItem *)(namesList->ItemAt (i)));
    BString nick (item->Name());

    if ((nick[0] == '@' || nick[0] == '+' || nick[0] == '%')
    &&  *data != nick[0])
      nick.Remove (0, 1);

    if ((*data == '@' || *data == '+' || *data == '%')
    &&   nick[0] != *data)
      ++data;

    if (!nick.ICompare (data))
      return i;
  }

  return -1;
}

bool
ChannelAgent::RemoveUser (const char *data)
{
  ASSERT (data != NULL);
  /*
   * Function purpose: Remove nickname {data} from the ChannelAgent's
   *                   NamesView and update the status counts
   */
   
  
  if (namesList == NULL)
    return false;
  
  int32 myIndex (FindPosition (data));

  if (myIndex != -1)
  {
    NameItem *item;

    namesList->Deselect (myIndex);
    if ((item = (NameItem *)namesList->RemoveItem (myIndex)) != NULL)
    {
      BString buffer;

      if ((item->Status() & STATUS_OP_BIT) != 0)
      {
        --opsCount;
        buffer << opsCount;
        
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

        buffer = "";
      }

      --userCount;
      buffer << userCount;
      if (!IsHidden())
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());

      delete item;
      return true;
    }
  }
  
  return false;
}


int
ChannelAgent::SortNames(const void *name1, const void *name2)
{
  /*
   * Function purpose: Help NamesView::SortItems() sort nicknames
   *
   * Order:
   *   Channel Ops
   *   Channel Helper/HalfOps
   *   Voiced
   *   Normal User
   */
   
   // clunky way to get around C++ warnings re casting from const void * to NameItem *
   
  const NameItem *firstPtr (*((NameItem * const *)name1));
  const NameItem *secondPtr (*((NameItem * const *)name2));

  // Not sure if this can happen, and we
  // are assuming that if one is NULL
  // we return them as equal.  What if one
  // is NULL, and the other isn't?
  if (!firstPtr
  ||  !secondPtr)
    return 0;

  BString first, second;

  first += (((firstPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (firstPtr)->Status()); 
  second += (((secondPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (secondPtr)->Status()); 
  first.Prepend ('0', 10 - first.Length());
  second.Prepend ('0', 10 - second.Length());



  first  += (firstPtr)->Name();
  second += (secondPtr)->Name();
  
  return first.ICompare (second);
}

void
ChannelAgent::TabExpansion (void)
{
  /*
   * Function purpose: Get the characters before the caret's current position,
   *                   and update the input VTextControl with a relevant match
   *                   from the ChannelAgent's NamesView 
   */
   
  int32 start, finish;
  static int32 lastindex;
  static BString lastNick;
  static BList myList;
  input->TextView()->GetSelection (&start, &finish);

  if (input->TextView()->TextLength()
  &&  start == finish
  &&  start == input->TextView()->TextLength())
  {
    const char *inputText (input->TextView()->Text() + input->TextView()->TextLength());
    const char *place (inputText);

    while (place > input->TextView()->Text())
    {
      if (*(place - 1) == '\x20')
        break;

      --place;
    }

    if (lastExpansion == "" 
    || lastExpansion.ICompare(place, strlen(lastExpansion.String())) != 0
    || lastNick != place)
    {
      lastindex = 0;
      lastExpansion = place;
      if (!myList.IsEmpty())
        myList.MakeEmpty();
      
      int32 count (namesList->CountItems());
      
      for (int32 i = 0; i < count ; i++)
        if (!((NameItem *)namesList->ItemAt(i))->Name().ICompare(lastExpansion.String(), strlen(lastExpansion.String())))
         myList.AddItem(namesList->ItemAt(i));
    }
  
    // We first check if what the user typed matches the channel
    // If that doesn't match, we check the names
    BString insertion;

    if (!id.ICompare (place, strlen (place)))
      insertion = id;
    else
    {
      int32 count (myList.CountItems());
      if (count > 0)
      {
        insertion = ((NameItem *)myList.ItemAt(lastindex++))->Name();
    
        if (lastindex == count) lastindex = 0;
          lastNick = insertion;
      }
    }

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


void
ChannelAgent::MessageReceived (BMessage *msg)
{
  int32 i (0);
  
  switch (msg->what)
  {
    case M_USER_QUIT:
      {
        const char *nick;

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
        const char *nick;
        bool ignore;
        int32 iStatus (STATUS_NORMAL_BIT);

        msg->FindString ("nick", &nick);
        msg->FindBool ("ignore", &ignore);

        if (ignore) iStatus |= STATUS_IGNORE_BIT;
 
        namesList->AddItem (new NameItem (nick, iStatus));
        namesList->SortItems (SortNames);

        ++userCount;
        BString buffer;
        buffer << userCount;
  
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());

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

        if (myNick.ICompare (oldNick) == 0 && !IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);

        if (((thePos = FindPosition (oldNick)) >= 0)
        &&  ((item = (static_cast<NameItem *>(namesList->ItemAt (thePos)))) != 0))
        {
          item->SetName (newNick);
          namesList->SortItems (SortNames);
        }
        else
          break;
          
        ClientAgent::MessageReceived (msg);
      }
      break;

    case M_CHANNEL_NAMES:
      {
        bool hit (false);

        for (i = 0; msg->HasString ("nick", i); ++i)
        {
          const char *nick (NULL);
          bool op (false),
            voice (false),
            helper (false),
            ignored (false);

          msg->FindString ("nick", i, &nick);
          msg->FindBool ("op", i, &op);
          msg->FindBool ("voice", i, &voice);
          msg->FindBool ("helper", i, &helper);
          msg->FindBool ("ignored", i, &ignored);
  
          if (FindPosition (nick) < 0)
          {
            int32 iStatus (ignored ? STATUS_IGNORE_BIT : 0);

            if (op)
            {
              ++nick;
              ++opsCount;
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
 
            userCount++;
 
            namesList->AddItem (new NameItem (nick, iStatus));
            hit = true;
          }
        }
      
        namesList->SortItems (SortNames);

        if (hit && !IsHidden())
        {
          BString buffer;
          buffer << opsCount;
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

          buffer = "";
          buffer << userCount;
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
        }
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
        myNick = newNick;  // update nickname (might have changed on reconnect)
			                    
        Display (S_CHANNEL_RECON_REJOIN B_UTF8_ELLIPSIS, C_ERROR, C_BACKGROUND, F_SERVER);
		
		// clean up
        namesList->ClearList();
        opsCount = 0;
        userCount = 0;
        
        // send join cmd		
        BMessage send (M_SERVER_SEND);	
        AddSend (&send, "JOIN ");
        AddSend (&send, id);	
        if (chanKey != "")
        {
          AddSend (&send, " ");
          AddSend (&send, chanKey);
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
        topic = theTopic;
      
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
          int32 pos = namesList->CurrentSelection();
          if (pos >= 0)
          {
            myUser = static_cast<NameItem *>(namesList->ItemAt (pos));
            BString targetNick = myUser->Name();
            msg->AddString ("nick", targetNick.String());
          }
        }
      
        sMsgr.SendMessage (msg);
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
        BString buffer;
        buffer += S_CHANNEL_GOT_KICKED;
        buffer += theChannel;
        buffer += S_CHANNEL_GOT_KICKED2;
        buffer += kicker;
        buffer += " (";
        buffer += rest;
        buffer += ")\n";
        PackDisplay (&wegotkicked, buffer.String(), C_QUIT, C_BACKGROUND, F_TEXT);
        // clean up
        namesList->ClearList();
        opsCount = 0;
        userCount = 0;
        
        msgr.SendMessage (&wegotkicked);

        BMessage attemptrejoin (M_DISPLAY); // "you were kicked"
        buffer = S_CHANNEL_REJOIN;
        buffer += theChannel;
        buffer += B_UTF8_ELLIPSIS"\n";
        PackDisplay (&attemptrejoin, buffer.String(), C_QUIT, C_BACKGROUND, F_TEXT);
        msgr.SendMessage (&attemptrejoin);

        BMessage send (M_SERVER_SEND);
        AddSend (&send, "JOIN ");
        AddSend (&send, theChannel);
        if (chanKey != "")
        {
          AddSend (&send, " ");
          AddSend (&send, chanKey);
        }     
        AddSend (&send, endl);
      }
      break;

    case M_CHANNEL_MODE:
      {
        ModeEvent (msg);
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

        if (id.ICompare (chan) == 0)
        {
          BString realMode (GetWord (mode, 1));
          int32 place (2);
 
          if (realMode.FindFirst ("l") >= 0)
            chanLimit = GetWord (mode, place++);

          if (realMode.FindFirst ("k") >= 0)
          {
            chanKey = GetWord (mode, place++);
            
            // u2 may not send the channel key, thats why we stored the /join cmd
            // in a string in ParseCmd
            if (chanKey == "*" && ircdtype == IRCD_UNDERNET)
            {
              BString tempId (id);
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
                
                chanKey = GetWord (tempKeyString.String(), 2);
              }
            } // end u2-kludge stuff
              
          }
          chanMode = mode;
          if (!IsHidden())
            vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, chanMode.String());  
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
        while ((i = namesList->CurrentSelection (index++)) >= 0)
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt (i));
          targetNick = myUser->Name();

          victims += " ";
          victims += targetNick;
          count++;
        }


        BString command ("/mode ");
        command += id;
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
        while ((i = namesList->CurrentSelection (index++)) >= 0)
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt (i));
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
        while ((i = namesList->CurrentSelection (index++)) >= 0)
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt (i));
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

    case M_NAMES_POPUP_DCCCHAT: 
      { 
        int32 index (0); 
        BString targetNick; 
        NameItem *myUser (NULL); 

        /// iterate /// 
        while ((i = namesList->CurrentSelection(index++)) >= 0) 
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt(i)); 
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
        while ((i = namesList->CurrentSelection(index++)) >= 0) 
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt(i)); 
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
        while ((i = namesList->CurrentSelection(index++)) >= 0)
        { 
          myUser = static_cast<NameItem *>(namesList->ItemAt(i));
          targetNick = myUser->Name();
 
           BString command ("/kick ");
           command += targetNick;
           command += " ";
           command += kickMsg;

           ParseCmd (command.String());
         }
       }
       break;

     case M_STATUS_ADDITEMS:
       {
         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           0, ""), true);
       
         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           S_STATUS_LAG, "", STATUS_ALIGN_LEFT), true);

         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           0, "", STATUS_ALIGN_LEFT), true);

         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           S_STATUS_USERS, ""), true);

         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           S_STATUS_OPS, ""), true);

         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           S_STATUS_MODES, ""), true);

         vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
           "", "", STATUS_ALIGN_LEFT), true);

         // The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
         // We send true on the last SetItemValue().
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, serverName.String(), false);
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String(), false);
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, myNick.String(), false);
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, chanMode.String(), false);
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_META, FilterCrap(topic.String(), true).String());

         BString buffer;
         buffer << userCount;
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String(), false);
         buffer = "";
         buffer << opsCount;
         vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String(), true);
       }
       break;

     case M_CHANNEL_OPTIONS_SHOW:
       {
         if (chanOpt)
           chanOpt->Activate();
         else
         {
           chanOpt = new ChannelOptions (id.String(), this);
           chanOpt->Show();
         }
       }
       break;

     case M_CHANNEL_OPTIONS_CLOSE:
       {
         chanOpt = NULL;
       }
       break;


     case M_CLIENT_QUIT:
       {
         ClientAgent::MessageReceived(msg);
         if ((msg->HasBool ("vision:part") && msg->FindBool ("vision:part"))
         ||  (msg->HasBool ("vision:winlist") && msg->FindBool ("vision:winlist")))
         {
           BMessage send (M_SERVER_SEND);
           AddSend (&send, "PART ");
           AddSend (&send, id);
           AddSend (&send, endl);
         }  
  
         BMessage deathchant (M_OBITUARY);
         deathchant.AddPointer ("agent", this);
         deathchant.AddPointer ("item", agentWinItem);
         vision_app->pClientWin()->PostMessage (&deathchant);
  
         deathchant.what = M_CLIENT_SHUTDOWN;
         sMsgr.SendMessage (&deathchant);
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
   
  lastExpansion = "";  // used by ChannelAgent::TabExpansion()

  BMessage send (M_SERVER_SEND);

  AddSend (&send, "PRIVMSG ");
  AddSend (&send, id);
  AddSend (&send, " :");

  Display ("<", C_MYNICK);
  Display (myNick.String(), C_NICKDISPLAY);
  Display ("> ", C_MYNICK);  
  
  BString sBuffer (buffer);
  
  if (sBuffer.Length() > 440)
  {
    // length isn't irc safe (512 limit), truncate and send the text after
    // character 440 on another line
    BString tempBuffer;
    int32 hit (Get440Len (buffer));

    sBuffer.MoveInto (tempBuffer, 0, hit);
    AddSend (&send, tempBuffer.String());
    AddSend (&send, endl);
  
    Display (tempBuffer.String());
    Display ("\n");
    
    Parser (sBuffer.String());
  }
  else
  {
    AddSend (&send, buffer);
    AddSend (&send, endl);

    Display (buffer);
    Display ("\n");  
  }
}

void
ChannelAgent::UpdateMode(char theSign, char theMode)
{
  char modeString[2]; // necessary C-style string
  memset(modeString, 0, sizeof(modeString));
  sprintf (modeString, "%c", theMode);
  
  if (theSign == '-')
  {
    if (theMode == 'l')
    {
      BString myTemp (chanLimit);
      myTemp.Append (" ");
      chanMode.RemoveLast (myTemp);
      myTemp = chanLimit;
      myTemp.Prepend (" ");
      chanMode.RemoveLast (myTemp);
      chanMode.RemoveLast (chanLimit);
    }
    else if (theMode == 'k')
    {
      BString myTemp (chanKey);
      myTemp.Append(" ");
      chanMode.RemoveLast (myTemp);
      myTemp = chanKey;
      myTemp.Prepend (" ");
      chanMode.RemoveLast (myTemp);
      chanMode.RemoveLast (chanKey);
    }
    
    chanMode.RemoveFirst (modeString);
  }
  else
  {
    BString theReal (GetWord(chanMode.String(), 1)),
            theRest (RestOfString(chanMode.String(), 2));
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
      if (chanLimitOld != "")
      {
        BString theOld (" ");
        theOld += chanLimitOld;
        tempString.RemoveFirst (theOld);
      }
      
      tempString.Append (" ");
      tempString.Append (chanLimit);
      chanLimitOld = chanLimit;
    }
    else if (theMode == 'k')
    {
      if (chanKeyOld != "")
      {
        BString theOld (" ");
        theOld += chanKeyOld;
        tempString.RemoveFirst (theOld);
      }
      
      tempString.Append (" ");
      tempString.Append (chanKey);
      chanKeyOld = chanKey;
    }
    
    chanMode = tempString;
  }

  if (!IsHidden())
    vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, chanMode.String());
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

  buffer += "*** ";
  buffer += theNick;
  buffer += S_CHANNEL_SET_MODE;
  buffer += mode;

  if (targetS != "-9z99")
  {
    buffer += " ";
    buffer += targetS;
  }

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
    ||  theModifier == 'v'
    ||  theModifier == 'h')
    {
      BString myTarget (GetWord (target, targetPos++));
      NameItem *item;
      int32 pos;

      if ((pos = FindPosition (myTarget.String())) < 0
      ||  (item = static_cast<NameItem *>(namesList->ItemAt (pos))) == 0)
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
          ++opsCount;

          buffer = "";
          buffer << opsCount;
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
          --opsCount;

          buffer = "";
          buffer << opsCount;
          
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
      chanLimit = "";
    }
    else if (theModifier == 'l')
    {
      BString myTarget (GetWord (target, targetPos++));
      chanLimitOld = chanLimit;
      chanLimit = myTarget;
      UpdateMode ('+', 'l');
    }
    else if (theModifier == 'k' && theOperator == '-')
    {
      UpdateMode('-', 'k');
      chanKey = "";
    }
    else if (theModifier == 'k')
    {
      BString myTarget (GetWord (target, targetPos++));
      chanKeyOld = chanKey;
      chanKey = myTarget;
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
    ||  mode[modPos] == '-')
      theOperator = mode[modPos++];
  }
 
  if (hit)
  {
    namesList->SortItems (SortNames);
    namesList->Invalidate();
  }
}
