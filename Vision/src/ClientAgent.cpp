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

#ifdef GNOME_BUILD
#  include "gnome/ScrollView.h"
#  include "gnome/PopUpMenu.h"
#  include "gnome/MenuItem.h"
#elif BEOS_BUILD
#  include <ScrollView.h>
#  include <PopUpMenu.h>
#  include <MenuItem.h>
#endif

#include <stdio.h>
#include <ctype.h>

#include "VTextControl.h"
#include "Vision.h"
#include "HistoryMenu.h"
#include "IRCView.h"
#include "MessageAgent.h"
#include "ChannelAgent.h"
#include "ClientWindow.h"
#include "StatusView.h"
#include "ClientAgent.h"
#include "ClientAgentInputFilter.h"
#include "StringManip.h"
#include "ServerAgent.h"
#include "WindowList.h"


const char *ClientAgent::endl               ("\1\1\1\1\1");

ClientAgent::ClientAgent (
  const char *id_,
  int32 sid_,
  const char *serverName_,
  const char *myNick_,
  BRect frame_)
  
  : BView (
    frame_,
    id_,
    B_FOLLOW_ALL_SIDES,
    B_WILL_DRAW | B_FRAME_EVENTS),
    
  id (id_),
  sid (sid_),
  serverName (serverName_),
  myNick (myNick_),
  timeStampState (vision_app->GetBool ("timestamp")),
  isLogging (vision_app->GetBool ("log_enabled")),
  frame (frame_)
{
  Init();
}

ClientAgent::ClientAgent (
  const char *id_,
  int32 sid_,
  const char *serverName_,
  const char *myNick_,
  const BMessenger &sMsgr_,
  BRect frame_)
  
  : BView (
    frame_,
    id_,
    B_FOLLOW_ALL_SIDES,
    B_WILL_DRAW),
    
  id (id_),
  sid (sid_),
  serverName (serverName_),
  myNick (myNick_),
  timeStampState (vision_app->GetBool ("timestamp")),
  isLogging (vision_app->GetBool ("log_enabled")),
  frame (frame_),
  sMsgr (sMsgr_)
{
  myLag = "0.000";
  Init();
}

ClientAgent::~ClientAgent (void)
{
  if (logger)
    delete logger;
  delete agentWinItem;
}


void
ClientAgent::AttachedToWindow (void)
{
}

void
ClientAgent::AllAttached (void)
{
  msgr = BMessenger (this);
  
  ServerAgent *sagent;
  if ((sagent = dynamic_cast<ServerAgent *>(this)))
  {
    sMsgr = BMessenger (this);
  } 

}

void
ClientAgent::Show (void)
{
  Window()->PostMessage (M_STATUS_CLEAR);
  this->msgr.SendMessage (M_STATUS_ADDITEMS);
  
  BMessage statusMsg (M_UPDATE_STATUS);
  statusMsg.AddPointer ("item", agentWinItem);
  statusMsg.AddInt32 ("status", WIN_NORMAL_BIT);
  statusMsg.AddBool ("hidden", false);
  Window()->PostMessage (&statusMsg);

  BView::Show();
}


void
ClientAgent::Init (void)
{    
  textColor        = vision_app->GetColor (C_TEXT);
  nickColor        = vision_app->GetColor (C_NICK);
  ctcpReqColor     = vision_app->GetColor (C_CTCP_REQ);
  quitColor        = vision_app->GetColor (C_QUIT);
  errorColor       = vision_app->GetColor (C_ERROR);
  whoisColor       = vision_app->GetColor (C_WHOIS);
  joinColor        = vision_app->GetColor (C_JOIN);
  myNickColor      = vision_app->GetColor (C_MYNICK);
  nickdisplayColor = vision_app->GetColor (C_NICKDISPLAY);
  actionColor      = vision_app->GetColor (C_ACTION);
  opColor          = vision_app->GetColor (C_OP);
  inputColor       = vision_app->GetColor (C_INPUT);

  myFont     = *(vision_app->GetClientFont (F_TEXT));
  serverFont = *(vision_app->GetClientFont (F_SERVER));
  inputFont  = *(vision_app->GetClientFont (F_INPUT));

  input = new VTextControl (
                BRect (
                  0,
                  frame.top, // tmp. will be moved
                  frame.right - frame.left,
                  frame.bottom),
                "Input", 0, 0,
                0,
                B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
  input->TextView()->SetFontAndColor (&inputFont, B_FONT_ALL, 
    &inputColor);
  input->SetDivider (0); 
  input->ResizeToPreferred(); 
  input->MoveTo (
           0,
           frame.bottom - input->Frame().Height() + 1);

  input->TextView()->AddFilter (new ClientAgentInputFilter (this));
  AddChild (input);

  input->TextView()->SetViewColor (vision_app->GetColor (C_INPUT_BACKGROUND));
  input->Invalidate(); 
  
  history = new HistoryMenu (BRect (
    frame.right - 11,
    input->Frame().top,
    frame.right - 1,
    input->Frame().bottom - 1));

  BRect textrect (
    1,
    frame.top - 2,
    frame.right - frame.left - 1 - B_V_SCROLL_BAR_WIDTH,
    frame.bottom - input->Frame().Height() - 1);

  text = new IRCView (
    textrect,
    textrect.InsetByCopy (5, 3),
    input,
    this);
  text->SetViewColor (vision_app->GetColor (C_BACKGROUND));
  

  textScroll = new BScrollView (
    "textscroll",
    text,
    B_FOLLOW_ALL_SIDES,
    0,
    false,
    true,
    B_PLAIN_BORDER);
  AddChild (textScroll);

  if (isLogging)
    logger = new Logger (id, serverName);
  else
    logger = NULL;
}

void
ClientAgent::ScrollRange (float *min, float *max)
{
  textScroll->ScrollBar(B_VERTICAL)->GetRange (min, max);
}


float
ClientAgent::ScrollPos (void)
{
  return textScroll->ScrollBar (B_VERTICAL)->Value();
}


void
ClientAgent::SetScrollPos (float value)
{
  textScroll->ScrollBar (B_VERTICAL)->SetValue(value);
}

void
ClientAgent::AddMenuItems (BPopUpMenu *pmenu)
{
  BMenuItem *item;
  
  ChannelAgent *channelagent;
  MessageAgent *messageagent;
  
  if ((channelagent = dynamic_cast<ChannelAgent *>(this)))
  {
    // Channel Options
    item = new BMenuItem("Channel Options", new BMessage (M_CHANNEL_OPTIONS_SHOW));
    item->SetTarget (this);
    pmenu->AddItem (item);
    
    pmenu->AddSeparatorItem();
  }

  if ((messageagent = dynamic_cast<MessageAgent *>(this)))
  {
    // Whois
    item = new BMenuItem("Whois", new BMessage (M_MSG_WHOIS));
    item->SetTarget (this);
    if (messageagent->Id().FindFirst (" [DCC]") >= 0)  // dont enable for dcc sessions
      item->SetEnabled (false);
    pmenu->AddItem (item);
    
    pmenu->AddSeparatorItem();
  }

}


void
ClientAgent::Submit (
  const char *buffer,
  bool clear,
  bool historyAdd)
{
  BString cmd;

  if (historyAdd)
    cmd = history->Submit (buffer);
  else
    cmd = buffer;

  if (clear) input->SetText ("");
  if (cmd.Length()
  && !SlashParser (cmd.String())
  &&   cmd[0] != '/')
    Parser (cmd.String());
}


int32
ClientAgent::TimedSubmit (void *arg)
{
  BMessage *msg (reinterpret_cast<BMessage *>(arg));
  ClientAgent *agent;
  ClientWindow *window;
  BString buffer;
  int32 i;

  if (msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK)
  {
    printf (":ERROR: no valid agent pointer found in TimedSubmit, bailing...\n");
    return -1;
  }
  
  if (msg->FindPointer ("window", reinterpret_cast<void **>(&window)) != B_OK)
  {
    printf (":ERROR: no valid window pointer found in TimedSubmit, bailing...\n");
    return -1;
  }

  BMessenger agentMsgr (agent);
  for (i = 0; msg->HasString ("data", i); ++i)
  {
    const char *data;

    msg->FindString ("data", i, &data);

    // :TODO: wade 020101 move locks to ParseCmd?
    if (agentMsgr.IsValid() && agent->LockLooper())
    {
      if (!agent->SlashParser (data))
        agent->Parser (data);

      agent->UnlockLooper();

      // A small attempt to appease the
      // kicker gods
      snooze (1000000);
    }
  }

  delete msg;
  return 0;
}


void
ClientAgent::PackDisplay (
  BMessage *msg,
  const char *buffer,
  const rgb_color *color,
  const BFont *font,
  bool timestamp)
{
  BMessage packed;

  packed.AddString ("msgz", buffer);

  if (color)
    packed.AddData (
      "color",
      B_RGB_COLOR_TYPE,
      color,
      sizeof (color));

  if (font)
    packed.AddPointer ("font", font);

  if (timestamp)
    packed.AddBool ("timestamp", timestamp);

  if (msg->HasMessage ("packed"))
    msg->ReplaceMessage ("packed", &packed);
  else  
    msg->AddMessage ("packed", &packed);
}


void
ClientAgent::Display (
  const char *buffer,
  const rgb_color *color,
  const BFont *font,
  bool timeStamp)
{

  timeStampState = vision_app->GetBool ("timestamp");

  if (isLogging)
  {
    BString printbuf;
    if (timeStamp && timeStampState)
      printbuf += TimeStamp().String();
        
    printbuf += buffer;
    logger->Log (printbuf.String());
  }
  
  if (timeStamp && timeStampState)
    text->DisplayChunk (
      TimeStamp().String(),
      &textColor,
      &myFont);
  
  text->DisplayChunk (
    buffer,
    color ? color : &textColor,
    font  ? font  : &myFont);

  if (IsHidden())
  {
    BMessage statusMsg (M_UPDATE_STATUS);
    statusMsg.AddPointer ("item", agentWinItem);
    statusMsg.AddInt32 ("status", WIN_NEWS_BIT);
    Window()->PostMessage (&statusMsg);
  }
}


void
ClientAgent::Parser (const char *)
{
  // do nothing
}


void
ClientAgent::TabExpansion (void)
{
 // do nothing
}


void
ClientAgent::DroppedFile (BMessage *)
{
 // do nothing
}

bool
ClientAgent::SlashParser (const char *data)
{
  BString first (GetWord (data, 1).ToUpper());

  if (ParseCmd (data))
    return true;

  return false;
}


void
ClientAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    // 22/8/99: this will now look for "text" to add to the
    // input view. -jamie
    case M_INPUT_FOCUS:
      {
        if (msg->HasString ("text"))
        {
          BString newtext;
          newtext = input->Text();
          newtext.Append (msg->FindString("text"));
          input->SetText (newtext.String());
        }
        input->MakeFocus (true);
        // We don't like your silly selecting-on-focus.
        input->TextView()->Select (input->TextView()->TextLength(),
        input->TextView()->TextLength()); 
      }
      break;
    
    case M_CLIENT_QUIT:
      {
        bool shuttingdown (false);
        if (msg->HasBool ("vision:shutdown_in_progress"))
          msg->FindBool ("vision:shutdown_in_progress", &shuttingdown);
        
        if (logger)
          logger->isQuitting = shuttingdown;
      }
      break;

    // This could probably be used for some scripting
    // capability -- oooh, neato!
    case M_SUBMIT_RAW:
      {
        bool lines (false);
        int32 which (0);
        msg->FindBool ("lines", &lines);
        msg->FindInt32 ("which", &which);
        if (msg->HasPointer ("invoker"))
        {
          BInvoker *invoker;
          msg->FindPointer ("invoker", reinterpret_cast<void **>(&invoker));
          delete invoker;
        }

        if (which == 1)
        {
          BMessage *buffer (new BMessage (*msg));
          thread_id tid;

          buffer->AddPointer ("agent", this);
          buffer->AddPointer ("window", Window());

          tid = spawn_thread (
            TimedSubmit,
            "Timed Submit",
            B_LOW_PRIORITY,
            buffer);

          resume_thread (tid);
        }
        else if ((which == 2) || (!lines))
        {
          BString buffer;
          for (int32 i = 0; msg->HasString ("data", i); ++i)
          {
            const char *data;

            msg->FindString ("data", i, &data);
            buffer += (i ? " " : "");
            buffer += data;
          }

          int32 start, finish;
          
          if (msg->FindInt32 ("selstart", &start) == B_OK)
          {
            msg->FindInt32 ("selend", &finish);
            if (start != finish)
              input->TextView()->Delete (start, finish);

            if ((start == 0) && (finish == 0))
            {
              input->TextView()->Insert (input->TextView()->TextLength(),
                buffer.String(), buffer.Length());
              input->TextView()->Select (input->TextView()->TextLength(),
                input->TextView()->TextLength());
            }
            else
            {
              input->TextView()->Insert (start, buffer.String(), buffer.Length());
              input->TextView()->Select (start + buffer.Length(),
                start + buffer.Length());
            }
          }
          else
          {
            input->TextView()->Insert (buffer.String());
            input->TextView()->Select (input->TextView()->TextLength(),
            input->TextView()->TextLength());
          }
        
          input->TextView()->ScrollToSelection();
        }
      }
      break;

    case M_PREVIOUS_INPUT:
      {
        history->PreviousBuffer (input);
      }
      break;

    case M_NEXT_INPUT:
      {
        history->NextBuffer (input);
      }
      break;

    case M_SUBMIT:
      {
        const char *buffer;
        bool clear (true),
        add2history (true);
    
        msg->FindString ("input", &buffer);
    
        if (msg->HasBool ("clear"))
          msg->FindBool ("clear", &clear);

        if (msg->HasBool ("history"))
          msg->FindBool ("history", &add2history);

        Submit (buffer, clear, add2history);
      } 
      break;

    case M_LAG_CHANGED:
      {
        msg->FindString ("lag", &myLag);
        
        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, myLag.String());
      }
      break;

    case M_DISPLAY:
      {
        const char *buffer;

        for (int32 i = 0; msg->HasMessage ("packed", i); ++i)
        {
          BFont *font (&myFont);
          const rgb_color *color (&textColor); 
          ssize_t size (sizeof (rgb_color));
          bool timeStamp (false);
          BMessage packed;

          msg->FindMessage ("packed", i, &packed);

          if (packed.HasPointer ("font"))
            packed.FindPointer ("font", reinterpret_cast<void **>(&font));

          if (packed.HasData ("color", B_RGB_COLOR_TYPE))
            packed.FindData ("color",
              B_RGB_COLOR_TYPE,
              reinterpret_cast<const void **>(&color),
              &size);

          if (packed.HasBool ("timestamp"))
            packed.FindBool ("timestamp", &timeStamp);

          packed.FindString ("msgz", &buffer);
          Display (buffer, color, font, timeStamp);
        }
      }
      break;

    case M_CHANNEL_MSG:
      {
        const char *theNick;
        const char *theMessage;
        bool hasNick (false);
        bool me;
        BString knownAs;

        msg->FindString("nick", &theNick);
        msg->FindString("msgz", &theMessage);

        if (theMessage[0] == '\1')
        {
          BString aMessage (theMessage);

          int32 theChars (aMessage.Length());
          aMessage.Truncate (theChars - 1);
          
          // this next if() is a quirk fix for JAVirc.
          // it appends an (illegal) space to actions, so the last
          // truncate doesn't remove the \1
          if (aMessage[theChars - 2] == '\1')
            aMessage.Truncate (theChars - 2);
      
          aMessage.RemoveFirst ("\1ACTION ");
          
          BString tempString("* ");
          tempString += theNick;
          tempString += " ";
          tempString += aMessage;
          tempString += '\n';
          Display (tempString.String(), &actionColor, 0, true);
        }
        else
        {
          Display ("<", theNick == myNick ? &myNickColor : &nickColor, 0, true);
          Display (theNick, &nickdisplayColor);
          Display (">", theNick == myNick ? &myNickColor : &nickColor);

          BString tempString;
          tempString += " ";
          tempString += theMessage;
          tempString += '\n';

          // soley for the purpose of iterating through the words
          int32 place;
          BString tempString2 (tempString);
          while ((place = FirstKnownAs (tempString2, knownAs, &me)) != B_ERROR)
          {
            BString buffer;

            if (place)
              tempString2.MoveInto (buffer, 0, place);

            tempString2.MoveInto (buffer, 0, knownAs.Length());

            if (me)
              hasNick = true;
          }

          Display (tempString.String(), 0);
        }

        if (IsHidden())
        {
          if (hasNick)
          { 
            BMessage statusMsg (M_UPDATE_STATUS);
            statusMsg.AddPointer ("item", agentWinItem);
            statusMsg.AddInt32 ("status", WIN_NICK_BIT);
            Window()->PostMessage (&statusMsg);
          }
        }
      }
      break;

    case M_CHANGE_NICK:
      {
        const char *oldNick;

        msg->FindString ("oldnick", &oldNick);

        if (myNick.ICompare (oldNick) == 0)
          myNick = msg->FindString ("newnick");

        BMessage display;
        if (msg->FindMessage ("display", &display) == B_NO_ERROR)
          ClientAgent::MessageReceived (&display);
      }
      break;

    case M_LOOKUP_WEBSTER:
      {
        BString lookup;
        msg->FindString ("string", &lookup);
        lookup = StringToURI (lookup.String());
        lookup.Prepend ("http://www.dictionary.com/cgi-bin/dict.pl?term=");      
        vision_app->LoadURL (lookup.String());
      }
      break;

    case M_LOOKUP_GOOGLE:
      {
        BString lookup;
        msg->FindString ("string", &lookup);
        lookup = StringToURI (lookup.String());
        lookup.Prepend ("http://www.google.com/search?q=");      
        vision_app->LoadURL (lookup.String());
      }
      break;

    default:
      BView::MessageReceived (msg);
  }
}


const BString &
ClientAgent::Id (void) const
{
  return id;
}

int32
ClientAgent::Sid (void) const
{
  //return const_cast<long int>(sid);
  return sid;
}

int32
ClientAgent::FirstKnownAs (
  const BString &data,
  BString &result,
  bool *me)
{
  BString myAKA (vision_app->GetString ("alsoKnownAs"));

  int32 hit (data.Length()),
        i,
        place;
  BString target;

  if ((place = FirstSingleKnownAs (data, myNick)) != B_ERROR)
  {
    result = myNick;
    hit = place;
    *me = true;
  }

  for (i = 1; (target = GetWord (myAKA.String(), i)) != "-9z99"; ++i)
  {
    if ((place = FirstSingleKnownAs (data, target)) != B_ERROR
    &&   place < hit)
    {
      result = target;
      hit = place;
      *me = true;
    }
  }

  return hit < data.Length() ? hit : B_ERROR;
}


int32
ClientAgent::FirstSingleKnownAs (const BString &data, const BString &target)
{
  int32 place;

  if ((place = data.IFindFirst (target)) != B_ERROR
  &&  (place == 0
  ||   isspace (data[place - 1])
  ||   ispunct (data[place - 1]))
  &&  (place + target.Length() == data.Length()
  ||   isspace (data[place + target.Length()])
  ||   ispunct (data[place + target.Length()])))
    return place;

  return B_ERROR;
}

void
ClientAgent::AddSend (BMessage *msg, const char *buffer)
{
  if (strcmp (buffer, endl) == 0)
  {
    if (sMsgr.IsValid())
      sMsgr.SendMessage (msg);
    else
      this->MessageReceived (msg);

    msg->MakeEmpty();
  }
  else
    msg->AddString ("data", buffer);
}


void
ClientAgent::AddSend (BMessage *msg, const BString &buffer)
{
  AddSend (msg, buffer.String());
}


void
ClientAgent::AddSend (BMessage *msg, int32 value)
{
  BString buffer;

  buffer << value;
  AddSend (msg, buffer.String());
}

void
ClientAgent::ChannelMessage (
  const char *msgz,
  const char *nick,
  const char *ident,
  const char *address)
{
  BMessage msg (M_CHANNEL_MSG);

  msg.AddString ("msgz", msgz);

  if (nick)
    msg.AddString ("nick", nick);
  
  if (ident)
    msg.AddString ("ident", ident);
  
  if (address)
    msg.AddString ("address", address);

  msgr.SendMessage (&msg);
}

void
ClientAgent::ActionMessage (
  const char *msgz,
  const char *nick)
{
  BMessage actionSend (M_SERVER_SEND);

  AddSend (&actionSend, "PRIVMSG ");
  AddSend (&actionSend, id);
  AddSend (&actionSend, " :\1ACTION ");
  AddSend (&actionSend, msgz);
  AddSend (&actionSend, "\1");
  AddSend (&actionSend, endl);

  BString theAction ("\1ACTION ");
  theAction += msgz;
  theAction += "\1";

  ChannelMessage (theAction.String(), nick);
}


void
ClientAgent::CTCPAction (BString theTarget, BString theMsg)
{
  BString theCTCP (GetWord (theMsg.String(), 1).ToUpper()),
          theRest (RestOfString (theMsg.String(), 2)),
          tempString ("[CTCP->");

  tempString += theTarget;
  tempString += "] ";
  tempString += theCTCP;

  if (theRest != "-9z99")
  {
    tempString += " ";
    tempString += theRest;
    tempString += '\n';
  }
  else
    tempString += '\n';

  Display (tempString.String(), &ctcpReqColor, &serverFont);
}
