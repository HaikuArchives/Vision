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

#include <Beep.h>
#include <File.h>
#include <MenuItem.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <ScrollView.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "VTextControl.h"
#include "Vision.h"
#include "HistoryMenu.h"
#include "Theme.h"
#include "RunView.h"
#include "MessageAgent.h"
#include "ChannelAgent.h"
#include "ClientWindow.h"
#include "StatusView.h"
#include "ClientAgent.h"
#include "ClientAgentInputFilter.h"
#include "StringManip.h"
#include "ServerAgent.h"
#include "WindowList.h"
#include "ClientAgentLogger.h"
#include "ResizeView.h"

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

  fCancelMLPaste(false),
  id (id_),
  sid (sid_),
  serverName (serverName_),
  myNick (myNick_),
  timeStampState (vision_app->GetBool ("timestamp")),
  isLogging (vision_app->GetBool ("log_enabled")),
  frame (frame_)
{
  Init();
  SetViewColor (B_TRANSPARENT_COLOR);
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
  BView::AttachedToWindow();
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

  // we initialize the color constants for the input control here
  // because BTextControl ignores them prior to being attached for some reason
  rgb_color inputColor (vision_app->GetColor (C_INPUT));
  input->TextView()->SetFontAndColor (vision_app->GetClientFont (F_INPUT), B_FONT_ALL,
    &inputColor);
  input->TextView()->SetViewColor (vision_app->GetColor (C_INPUT_BACKGROUND));
  input->TextView()->SetColorSpace (B_RGB32);
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
  
  const BRect *agentRect (((ClientWindow *)Window())->AgentRect());
  
  if (*agentRect != Frame())
  {
    ResizeTo (agentRect->Width(), agentRect->Height());
    MoveTo (agentRect->left, agentRect->top);
  }
  ChannelAgent *agent (dynamic_cast<ChannelAgent *>(this));
  if (agent)
  {
    const BRect namesListRect (vision_app->GetRect ("namesListRect"));
    int32 difference ((int32)(((BView *)agent->namesList)->Bounds().Width() - namesListRect.Width()));
    if (difference != 0.0)
    {
      agent->resize->MoveBy (difference, 0.0);
      textScroll->ResizeBy (difference, 0.0);
      agent->namesScroll->ResizeBy (-difference, 0.0);
      agent->namesScroll->MoveBy (difference, 0.0);
      Sync();
    }
  }
  // make RunView recalculate itself
  text->Show();
  BView::Show();
}


void
ClientAgent::Init (void)
{
  input = new VTextControl (
                BRect (
                  0,
                  frame.top, // tmp. will be moved
                  frame.right - frame.left,
                  frame.bottom),
                "Input", 0, 0,
                0,
                B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
  
  input->SetDivider (0);
  input->ResizeToPreferred();
  input->MoveTo (
           0,
           frame.bottom - input->Frame().Height() + 1);
  AddChild (input);
  input->TextView()->AddFilter (new ClientAgentInputFilter (this));
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
  
  Theme *activeTheme (vision_app->ActiveTheme());
  text = new RunView (
    textrect,
    id.String(),
    activeTheme,
    B_FOLLOW_ALL);
   
  text->SetClippingName (id.String());
  
  if (vision_app->GetBool ("timestamp"))
    text->SetTimeStampFormat (vision_app->GetString ("timestamp_format"));
 
 
  activeTheme->AddView (text);
  
  textScroll = new BScrollView (
    "textscroll",
    text,
    B_FOLLOW_ALL,
    0,
    false,
    true,
    B_PLAIN_BORDER);
  
  AddChild (textScroll);

  if (isLogging)
    logger = new ClientAgentLogger (id, serverName);
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
ClientAgent::SetServerName (const char *name)
{
  serverName = name;
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

BString
ClientAgent::FilterCrap (const char *data, bool force)
{
  BString outData ("");
  int32 theChars (strlen (data));
  bool ViewCodes (false);

  for (int32 i = 0; i < theChars; ++i)
  {
    if (data[i] == 3 && !force && !vision_app->GetBool ("stripcolors"))
      outData << data[i];
    else if (data[i] > 1 && data[i] < 32)
    {
      // TODO Get these codes working
      if (data[i] == 3)
      {
        if (ViewCodes)
          outData << "[0x03]{";

        ++i;
        while (i < theChars
        &&   ((data[i] >= '0'
        &&     data[i] <= '9')
        ||     data[i] == ','))
        {
          if (ViewCodes)
          outData << data[i];

          ++i;
        }
        
        --i;
        
        if (ViewCodes)
          outData << "}";
      }
      else if (ViewCodes)
      {
        char buffer[16];
        sprintf (buffer, "[0x%02x]", data[i]);
        outData << buffer;
      }
    }
    else
      outData << data[i];
  }

  return outData;
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

  bool delay (!msg->HasBool ("delay"));

  BMessenger agentMsgr (agent);
  BMessage submitMsg (M_SUBMIT);
  for (i = 0; (msg->HasString ("data", i)) && (false == agent->CancelMultilineTextPaste()); ++i)
  {
    const char *data;

    msg->FindString ("data", i, &data);
    if (!submitMsg.HasString ("input"))
      submitMsg.AddString ("input", data);
    else
      submitMsg.ReplaceString ("input", data);
    // :TODO: wade 020101 move locks to ParseCmd?
    if (agentMsgr.IsValid())
    {
      agentMsgr.SendMessage (&submitMsg);

      // A small attempt to appease the
      // kicker gods
      if (delay)
        snooze (300000);
    }
  }

  delete msg;
  return 0;
}


void
ClientAgent::PackDisplay (
  BMessage *msg,
  const char *buffer,
  uint32 fore,
  uint32 back,
  uint32 font)
{
  BMessage packed;
  
  packed.AddString ("msgz", buffer);

  packed.AddInt32 ("fore", fore);
  packed.AddInt32 ("back", back);
  packed.AddInt32 ("font", font);
  
  if (msg->HasMessage ("packed"))
    msg->ReplaceMessage ("packed", &packed);
  else
    msg->AddMessage ("packed", &packed);
}


void
ClientAgent::Display (
  const char *buffer,
  uint32 fore,
  uint32 back,
  uint32 font)
{

  if (isLogging)
    logger->Log (buffer);
  
  // displays normal text if no color codes are present
  // (i.e. if the text has already been filtered by ServerAgent::FilterCrap
  ParsemIRCColors (buffer, fore, back, font);

  if (IsHidden())
  {
    BMessage statusMsg (M_UPDATE_STATUS);
    statusMsg.AddPointer ("item", agentWinItem);
    statusMsg.AddInt32 ("status", WIN_PAGESIX_BIT);
    Window()->PostMessage (&statusMsg);
  }
}

void
ClientAgent::ParsemIRCColors (
  const char *buffer,
  uint32 fore,
  uint32 back,
  uint32 font)
{
  int mircFore = fore;
  int mircBack = back;
  int mircFont = font;
  while (buffer && *buffer)
  {
   
    const char *start = buffer;
    while (*buffer)
    {
      if (*buffer != 3)
      {
        ++buffer;
        continue;
      }
      if (*buffer == 3 && start != buffer)
        break;
      ++buffer;
      if (!isdigit (*buffer))
      {
        // reset
        mircFore = fore;
        mircBack = back;
        mircFont = font;
      }
      else
      {
        // parse colors
        mircFore = 0;
        while (isdigit (*buffer))
          mircFore = mircFore * 10 + *buffer++ - '0';
        mircFore = (mircFore % 16) + C_MIRC_WHITE;
        if (*buffer == ',')
        {
          ++buffer;
          mircBack = 0;
          while (isdigit (*buffer))
            mircBack = mircBack * 10 + *buffer++ - '0';
          mircBack = (mircBack % 16) + C_MIRC_WHITE;
        }
      }
      // set start to text portion (we have recorded the mirc stuff)
      start = buffer;
    }
    text->Append (start, buffer - start, mircFore, mircBack, mircFont);
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
      
    case M_STATE_CHANGE:
      {
        int32 which (msg->FindInt32 ("which"));
        if (msg->HasBool ("color"))
        {
          switch (which)
          {
            case C_INPUT:
              rgb_color inputColor (vision_app->GetColor (C_INPUT));
              input->TextView()->SetFontAndColor (vision_app->GetClientFont(F_INPUT), B_FONT_ALL,
                &inputColor);
              input->TextView()->Invalidate();
              break;
            
            case C_INPUT_BACKGROUND:
              input->TextView()->SetViewColor (vision_app->GetColor (C_INPUT_BACKGROUND));
              input->TextView()->Invalidate();
              break;
                
            default:
              break;
          }
          Invalidate();
        }
        else if (msg->HasBool ("font"))
        {
          switch (which)
          {
            case F_INPUT:
              rgb_color inputColor (vision_app->GetColor (C_INPUT));
              input->TextView()->SetFontAndColor (vision_app->GetClientFont (F_INPUT), B_FONT_ALL,
                &inputColor);
              break;
            
            default:
              break;
          }
        }
        else if (msg->HasBool ("bool"))
        {
          if (timeStampState = vision_app->GetBool ("timestamp"))
            text->SetTimeStampFormat (vision_app->GetString ("timestamp_format"));
          else
            text->SetTimeStampFormat (NULL);
         
          bool logging (vision_app->GetBool ("log_enabled"));
          if (logging != isLogging)
          {
            if (isLogging)
            {
              isLogging = false;
              delete logger;
              logger = 0;
            }
            else
            {
              logger = new ClientAgentLogger (id, serverName);
              isLogging = true;
            }
          }
        }
      }
      break;
     
    case M_SUBMIT_INPUT:
      {
      	fCancelMLPaste = false;
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

        if ((which == 0) && msg->HasInt32 ("which"))
          break;

        if (which == 1 || which == 3)
        {
          BMessage *buffer (new BMessage (*msg));
          thread_id tid;

          buffer->AddPointer ("agent", this);
          buffer->AddPointer ("window", Window());
          if (which == 3)
            buffer->AddBool ("delay", false);
          tid = spawn_thread (
            TimedSubmit,
            "Timed Submit",
            B_LOW_PRIORITY,
            buffer);

          resume_thread (tid);
        }
        else if ((which == 2) || !lines)
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
          BMessage packed;

          msg->FindMessage ("packed", i, &packed);
          packed.FindString ("msgz", &buffer);
          Display (buffer, packed.FindInt32 ("fore"), packed.FindInt32 ("back"), packed.FindInt32 ("font"));
        }
      }
      break;

    case M_CHANNEL_MSG:
      {
        BString theNick;
        const char *theMessage;
        bool hasNick (false);
        bool isAction (false);
        BString knownAs;

        msg->FindString("nick", &theNick);
        msg->FindString("msgz", &theMessage);

        if (theNick.FindFirst (" [DCC]") != B_ERROR)
          theNick.RemoveFirst (" [DCC]");
        
        BString tempString;
        BString nickString;
        
        if (theMessage[0] == '\1')
        {
          BString aMessage (theMessage);    
          aMessage.RemoveFirst ("\1ACTION ");
          aMessage.RemoveLast ("\1");
          
          tempString = " ";
          tempString += aMessage;
          tempString += "\n";
          
          nickString = "* ";
          nickString += theNick;
          isAction = true;
        }
        else
        {
          Display ("<", theNick == myNick ? C_MYNICK : C_NICK);
          Display (theNick.String(), C_NICKDISPLAY);
          Display (">", theNick == myNick ? C_MYNICK : C_NICK);
          tempString += " ";
          tempString += theMessage;
          tempString += '\n';
        }

        // soley for the purpose of iterating through the words
        // TODO: rewrite this so nicks highlight without breaking URLs
/*        int32 place;
        while ((place = FirstKnownAs (tempString, knownAs, &hasNick)) != B_ERROR)
        {
          BString buffer;
          if (place)
          {
            tempString.MoveInto (buffer, 0, place);
            Display (buffer.String(), isAction ? C_ACTION : C_TEXT);
            buffer = "";
          }
          
          tempString.MoveInto (buffer, 0, knownAs.Length());
          Display (buffer.String(), C_MYNICK);
          buffer = "";
        } */
        
        
        // scan for presence of nickname, highlight if present
        FirstKnownAs (tempString, knownAs, &hasNick);

        tempString.Prepend (nickString);
 
        int32 dispColor = C_TEXT;
        if (hasNick)
          dispColor = C_MYNICK;
        else if (isAction)
          dispColor = C_ACTION;
        
        Display (tempString.String(), dispColor);

        if (IsHidden())
        {
          BMessage statusMsg (M_UPDATE_STATUS);
          statusMsg.AddPointer ("item", agentWinItem);

          if (hasNick || dynamic_cast<MessageAgent *>(this))
          {
            statusMsg.AddInt32 ("status", WIN_NICK_BIT);
            system_beep(kSoundEventNames[(uint32)seNickMentioned]);
          }
          else
            statusMsg.AddInt32 ("status", WIN_NEWS_BIT);

          Window()->PostMessage (&statusMsg);
        }
        else if (!Window()->IsActive())
          if (hasNick || dynamic_cast<MessageAgent *>(this))
            system_beep(kSoundEventNames[(uint32)seNickMentioned]);

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
    case M_RESIZE_VIEW:
      {
        ChannelAgent *currentAgent (dynamic_cast<ChannelAgent *>(this));
        if (currentAgent)
        {
          BPoint point;
          msg->FindPoint ("loc", &point);
          point.x -= currentAgent->Frame().left;
          int32 offset ((int32)(point.x - ((BView *)currentAgent->namesScroll)->Frame().left));
          currentAgent->resize->MoveBy (offset, 0.0);
          textScroll->ResizeBy (offset, 0.0);
          currentAgent->namesScroll->ResizeBy (-offset, 0.0);
          currentAgent->namesScroll->MoveBy (offset, 0.0);
          BRect namesRect (0, 0, currentAgent->namesScroll->Bounds().Width(), 0);
          vision_app->SetRect ("namesListRect", namesRect);
        } 
      }
      break;

	case B_ESCAPE:
	  	fCancelMLPaste = true;
		break;
	
	case M_DCC_COMPLETE:
	  {
          /// set up ///
        BString nick,
          file,
          size,
          type,
          message ("[@] "),
          fAck;
        int32 rate,
          xfersize;
        bool completed (true);

        msg->FindString ("nick", &nick);
        msg->FindString ("file", &file);
        msg->FindString ("size", &size);
        msg->FindString ("type", &type);
        msg->FindInt32 ("transferred", &xfersize);
        msg->FindInt32 ("transferRate", &rate);
				
        BPath pFile (file.String());

        fAck << xfersize;
								
        if (size.ICompare (fAck))
          completed = false;


          /// send mesage ///				
        if (completed)
          message << "Completed ";
        else message << "Terminated ";
				
        if (type == "SEND")
          message << "send of " << pFile.Leaf() << " to ";
        else message << "receive of " << pFile.Leaf() << " from ";
				
        message << nick << " (";

        if (!completed)
          message << fAck << "/";
				
        message << size << " bytes), ";
        message	<< rate << " cps\n";
					
        Display (message.String(), C_CTCP_RPY);
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

  Display (tempString.String(), C_CTCP_REQ, C_BACKGROUND, F_SERVER);
}
