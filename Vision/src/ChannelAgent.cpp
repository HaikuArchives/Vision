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
 */

#ifdef GNOME_BUILD
#  include "gnome/ScrollView.h"
#  include "gnome/FilePanel.h"
#elif BEOS_BUILD
#  include <ScrollView.h>
#  include <FilePanel.h>
#endif

#include <stdio.h>

#include "Names.h"
#include "ChannelAgent.h"
#include "Vision.h"
#include "StatusView.h"
#include "ClientWindow.h"
#include "StringManip.h"
#include "VTextControl.h"
#include "ChannelOptions.h"


ChannelAgent::ChannelAgent (
  const char *id_,
  int32 sid_,
  const char *serverName_,
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
   
  textScroll->ResizeTo (
    Frame().Width() - 100,
    textScroll->Frame().Height());

  frame = Bounds();
  frame.left   = textScroll->Frame().right + 1;
  frame.right -= B_V_SCROLL_BAR_WIDTH;
  frame.bottom = textScroll->Frame().bottom - 1;

  namesList = new NamesView (frame);

  namesScroll = new BScrollView(
    "scroll_names",
    namesList,
    B_FOLLOW_RIGHT | B_FOLLOW_TOP_BOTTOM,
    0,
    false,
    true,
    B_PLAIN_BORDER);

  AddChild (namesScroll);

  joinColor = vision_app->GetColor (C_JOIN);
  quitColor = vision_app->GetColor (C_QUIT);

  Display ("*** Now talking in ", &joinColor);
  Display (id.String(), &joinColor);
  Display ("\n", &joinColor);
}

int
ChannelAgent::FindPosition (const char *data)
{
  /*
   * Function purpose: Find the index of nickname {data} in the
   *                   ChannelAgent's NamesView
   */
   
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
  /*
   * Function purpose: Remove nickname {data} from the ChannelAgent's
   *                   NamesView and update the status counts
   */
   
  int32 myIndex (FindPosition (data));

  if (myIndex != -1)
  {
    NameItem *item;

    namesList->Deselect (myIndex);
    if ((item = (NameItem *)namesList->RemoveItem (myIndex)) != 0)
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
   
  NameItem **firstPtr ((NameItem **)name1);
  NameItem **secondPtr ((NameItem **)name2);

  // Not sure if this can happen, and we
  // are assuming that if one is NULL
  // we return them as equal.  What if one
  // is NULL, and the other isn't?
  if (!firstPtr
  ||  !secondPtr
  ||  !(*firstPtr)
  ||  !(*secondPtr))
    return 0;

  BString first, second;

  first += (((*firstPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (*firstPtr)->Status()); 
  second += (((*secondPtr)->Status() & STATUS_OP_BIT) ? STATUS_OP_BIT : (*secondPtr)->Status()); 
  first.Prepend ('0', 10 - first.Length());
  second.Prepend ('0', 10 - second.Length());

  first  += (*firstPtr)->Name();
  second += (*secondPtr)->Name();

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

      break;
    }

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

      break;
    }

    case M_CHANGE_NICK:
    {
      const char *oldNick, *newNick;
      NameItem *item;
      int32 thePos;

      msg->FindString ("oldnick", &oldNick);
      msg->FindString ("newnick", &newNick);

      if ((thePos = FindPosition (oldNick)) < 0
      ||  (item = (static_cast<NameItem *>(namesList->ItemAt (thePos)))) == 0)
        return;

      item->SetName (newNick);
      namesList->SortItems (SortNames);

      if (myNick.ICompare (oldNick) == 0 && !IsHidden())
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);

      ClientAgent::MessageReceived (msg);
      break;
    }

    case M_CHANNEL_NAMES:
    {
      bool hit (false);

      for (i = 0; msg->HasString ("nick", i); ++i)
      {
        const char *nick;
        bool op, voice, helper, ignored;

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

      if (hit && !IsHidden())
      {
        namesList->SortItems (SortNames);
        BString buffer;
        buffer << opsCount;
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

        buffer = "";
        buffer << userCount;
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
      }
      
      break;
    }

    case M_CHANNEL_TOPIC:
    {
      const char *theTopic;
      BString buffer;

      msg->FindString ("topic", &theTopic);
      topic = theTopic;
      
      if (!IsHidden())
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_META, theTopic);

      BMessage display;
      
      if (msg->FindMessage ("display", &display) == B_NO_ERROR)
        ClientAgent::MessageReceived (&display);

      break;
    }

    case M_OPEN_MSGAGENT:
    {
      const char *theNick;
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
      break;
    }

    case M_CHANNEL_GOT_KICKED:
    {
      const char *theChannel, *kicker, *rest;
      msg->FindString ("channel", &theChannel);
      msg->FindString ("kicker", &kicker);
      msg->FindString ("rest", &rest);    

      BMessage wegotkicked (M_DISPLAY); // "you were kicked"
      BString buffer;
      buffer << "*** You have been kicked from "
      << " by " << kicker << " (" << rest << ")\n";
      PackDisplay (&wegotkicked, buffer.String(), &quitColor, 0, true);
      msgr.SendMessage (&wegotkicked);

      BMessage attemptrejoin (M_DISPLAY); // "you were kicked"
      buffer = "*** Attempting to rejoin ";
      buffer << theChannel << "...\n";
      PackDisplay (&attemptrejoin, buffer.String(), &quitColor, 0, true);
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
      break;
    }

    case M_CHANNEL_MODE:
    {
      ModeEvent (msg);
      break;
    }

    case M_CHANNEL_MODES:
    {
      const char *mode, *chan, *msgz;
      msg->FindString ("mode", &mode);
      msg->FindString ("chan", &chan);
      msg->FindString ("msgz", &msgz);

      if (id.ICompare (chan) == 0)
      {
        BString realMode (GetWord (mode, 1));
        int32 place (2);

        if (realMode.FindFirst ("l") >= 0)
          chanLimit = GetWord (mode, place++);

        if (realMode.FindFirst ("k") >= 0)
          chanKey = GetWord (mode, place++);

        chanMode = mode;
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, chanMode.String());
        
      }
      
      BMessage dispMsg (M_DISPLAY);
      PackDisplay (&dispMsg, msgz, &opColor, 0, vision_app->GetBool ("timestamp"));
      BMessenger display (this);
      display.SendMessage (&dispMsg);
      break;
    }

    case POPUP_MODE:
    {
      const char *inaction;
      msg->FindString ("action", &inaction);

      int32 count (0), index (0);
      
      BString victims,
              targetNick,
              action (inaction),
              modechar,
              polarity;
      
      NameItem *myUser;

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

      for(i = 0; i < count; i++)
        command += modechar;

      command += victims;

      ParseCmd (command.String());

      break;
    }

    case POPUP_CTCP:
    {
      const char *inaction;
      msg->FindString ("action", &inaction);

      int32 index (0);
      BString victims,
              targetNick,
              action (inaction);
      NameItem *myUser;
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

      break;
    }

    case POPUP_WHOIS:
    {
      int32 index (0);
      BString victims,
              targetNick;
      NameItem *myUser;

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

      break;
    }

    case POPUP_KICK:
    {
      int32 index (0);
      BString targetNick,
              kickMsg (vision_app->GetCommand (CMD_KICK));
      NameItem *myUser;

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

       break;
     }

     case M_STATUS_ADDITEMS:
     {
       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         0, ""), true);
       
       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         "Lag: ", "", STATUS_ALIGN_LEFT), true);

       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         0, "", STATUS_ALIGN_LEFT), true);

       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         "Users: ", ""), true);

       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         "Ops: ", ""), true);

       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         "Modes: ", ""), true);

       vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
         "", "", STATUS_ALIGN_LEFT), true);

       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, serverName.String());
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, "0.000");
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, myNick.String());
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_MODES, chanMode.String());
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_META, topic.String());

       BString buffer;
       buffer << userCount;
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_USERS, buffer.String());
       buffer = "";
       buffer << opsCount;
       vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_OPS, buffer.String());

       break;
     }

     case M_CHANNEL_OPTIONS_SHOW:
     {
       if (chanOpt)
         chanOpt->Activate();
       else
       {
         chanOpt = new ChannelOptions (id.String(), this);
         chanOpt->Show();
       }
       
       break;
     }

     case M_CHANNEL_OPTIONS_CLOSE:
     {
       chanOpt = 0;
       break;
     }

     case M_CLIENT_QUIT:
     {
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
       break;
     }

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
  AddSend (&send, buffer);
  AddSend (&send, endl);

  Display ("<", &myNickColor, 0, true);
  Display (myNick.String(), &nickdisplayColor);
  Display ("> ", &myNickColor);

  BString sBuffer (buffer);
  Display (sBuffer.String(), 0);

  #if 0
  int32 hit;

  do
  {
    int32 place (0);
    BString nick;

    hit = sBuffer.Length();

    for (int32 i = 0; i < namesList->CountItems(); ++i)
    {
      NameItem *item (static_cast<NameItem *>(namesList->ItemAt (i)));
      BString iNick (item->Name());

      if (myNick.ICompare (item->Name())
      && (place = FirstSingleKnownAs (sBuffer, iNick)) != B_ERROR
      &&  place < hit)
      {
        hit = place;
        nick = item->Name();
        break;
      }
    }

    BString tempString;

    if (hit < sBuffer.Length())
    {
      if (hit)
      {
        sBuffer.MoveInto (tempString, 0, hit);
        Display (tempString.String(), 0);
      }

      sBuffer.MoveInto (tempString, 0, nick.Length());
      Display (tempString.String(), &nickColor);
    }

 } while (hit < sBuffer.Length());

  if (sBuffer.Length())
    Display (sBuffer.String(), 0);
  #endif
    
  Display ("\n", 0);
}

void
ChannelAgent::UpdateMode(char theSign, char theMode)
{
  char modeString[2]; // necessary C-style string
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
  bool hit (false),
       timeStamp (vision_app->GetBool ("timestamp"));


  // TODO Change Status to bitmask -- Too hard this way
  msg->FindString ("mode", &mode);
  msg->FindString ("target", &target);
  msg->FindString ("nick", &theNick);

  BString buffer,
          targetS (target);

  buffer += "*** ";
  buffer += theNick;
  buffer += " set mode: ";
  buffer += mode;

  if (targetS != "-9z99")
  {
    buffer += " ";
    buffer += targetS;
  }

  buffer += "\n";

  BMessenger display (this);

  BMessage modeMsg (M_DISPLAY);
  PackDisplay (&modeMsg, buffer.String(), &opColor, 0, timeStamp);
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
    else if (theModifier == 'b') 
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
