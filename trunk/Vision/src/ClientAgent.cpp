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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ClientAgentInputFilter.h"
#include "ClientAgentLogger.h"
#include "HistoryList.h"
#include "ResizeView.h"
#include "RunView.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Theme.h"
#include "Vision.h"
#include "VTextControl.h"
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
  fCancelMLPaste(false),
  fActiveTheme (vision_app->ActiveTheme()),
  fId (id_),
  fSid (sid_),
  fServerName (serverName_),
  fMyNick (myNick_),
  fTimeStampState (vision_app->GetBool ("timestamp")),
  fIsLogging (vision_app->GetBool ("log_enabled")),
  fFrame (frame_)
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
  fActiveTheme (vision_app->ActiveTheme()),
  fId (id_),
  fSid (sid_),
  fServerName (serverName_),
  fMyNick (myNick_),
  fTimeStampState (vision_app->GetBool ("timestamp")),
  fIsLogging (vision_app->GetBool ("log_enabled")),
  fFrame (frame_),
  fSMsgr (sMsgr_)
{
  fMyLag = "0.000";
  Init();
  // force server agent to post a lag meter update immediately
  fSMsgr.SendMessage (M_LAG_CHANGED);
}

ClientAgent::~ClientAgent (void)
{
  delete fAgentWinItem;
}


void
ClientAgent::AttachedToWindow (void)
{
  BView::AttachedToWindow();
  fActiveTheme->WriteLock();
  fActiveTheme->AddView (this);  
  fActiveTheme->WriteUnlock();
}

void
ClientAgent::DetachedFromWindow (void)
{
  BView::DetachedFromWindow ();
  fActiveTheme->WriteLock();
  fActiveTheme->RemoveView (this);  
  fActiveTheme->WriteUnlock();
}

void
ClientAgent::AllAttached (void)
{
  fMsgr = BMessenger (this);

  // we initialize the color constants for the fInput control here
  // because BTextControl ignores them prior to being attached for some reason
  fActiveTheme->ReadLock();
  rgb_color fInputColor (fActiveTheme->ForegroundAt (C_INPUT));
  fInput->TextView()->SetFontAndColor (&fActiveTheme->FontAt (F_INPUT), B_FONT_ALL,
    &fInputColor);
  fInput->TextView()->SetViewColor (fActiveTheme->ForegroundAt (C_INPUT_BACKGROUND));
  fInput->TextView()->SetColorSpace (B_RGB32);
  fActiveTheme->ReadUnlock();

  if (fIsLogging)
  {
    BMessage logMessage (M_REGISTER_LOGGER);
    logMessage.AddString ("name", fId.String());
    fSMsgr.SendMessage (&logMessage);
  }
}

void
ClientAgent::Show (void)
{
  Window()->PostMessage (M_STATUS_CLEAR);
  this->fMsgr.SendMessage (M_STATUS_ADDITEMS);

  BMessage statusMsg (M_CW_UPDATE_STATUS);
  statusMsg.AddPointer ("item", fAgentWinItem);
  statusMsg.AddInt32 ("status", WIN_NORMAL_BIT);
  statusMsg.AddBool ("hidden", false);
  Window()->PostMessage (&statusMsg);
  
  const BRect *agentRect (((ClientWindow *)Window())->AgentRect());
  
  if (*agentRect != Frame())
  {
    ResizeTo (agentRect->Width(), agentRect->Height());
    MoveTo (agentRect->left, agentRect->top);
  }

  // make RunView recalculate itself
  fText->Show();
  BView::Show();
}


void
ClientAgent::Init (void)
{
  fInput = new VTextControl (
                BRect (
                  0,
                  fFrame.top, // tmp. will be moved
                  fFrame.right - fFrame.left,
                  fFrame.bottom),
                "Input", 0, 0,
                0,
                B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM);
  
  fInput->SetDivider (0);
  fInput->ResizeToPreferred();
  fInput->MoveTo (
           0,
           fFrame.bottom - fInput->Frame().Height() + 1);
  AddChild (fInput);
  fInput->TextView()->AddFilter (new ClientAgentInputFilter (this));
  fInput->Invalidate();
  
  fHistory = new HistoryList ();
  
  BRect textrect (
    1,
    fFrame.top - 2,
    fFrame.right - fFrame.left - 1 - B_V_SCROLL_BAR_WIDTH,
    fFrame.bottom - fInput->Frame().Height() - 1);
  
  fText = new RunView (
    textrect,
    fId.String(),
    fActiveTheme,
    B_FOLLOW_ALL);
   
  fText->SetClippingName (fId.String());
  
  if (vision_app->GetBool ("timestamp"))
    fText->SetTimeStampFormat (vision_app->GetString ("timestamp_format"));
  
  fTextScroll = new BScrollView (
    "textscroll",
    fText,
    B_FOLLOW_ALL,
    0,
    false,
    true,
    B_PLAIN_BORDER);
  
  AddChild (fTextScroll);
}

void
ClientAgent::ScrollRange (float *scrollMin, float *scrollMax)
{
  fTextScroll->ScrollBar(B_VERTICAL)->GetRange (scrollMin, scrollMax);
}


float
ClientAgent::ScrollPos (void)
{
  return fTextScroll->ScrollBar (B_VERTICAL)->Value();
}


void
ClientAgent::SetScrollPos (float value)
{
  fTextScroll->ScrollBar (B_VERTICAL)->SetValue(value);
}

void
ClientAgent::SetServerName (const char *name)
{
  fServerName = name;
}

BString
ClientAgent::FilterCrap (const char *data, bool force)
{
  BString outData ("");
  int32 theChars (strlen (data));
  bool ViewCodes (false);
  int32 i (0), j(0);

  for (i = 0; i < theChars; ++i)
  {
    if (data[i] == 3 && !force && !vision_app->GetBool ("stripcolors"))
      outData << data[i];
    else if (data[i] > 1 && data[i] < 32)
    {
      if (data[i] == 3)
      {
        if (ViewCodes)
          outData << "[0x03]{";

        ++i;
        
        // filter foreground
        for (j = 0; j < 2; j++)
          if (data[i] >= '0' && data[i] <= '9')
          {
            if (ViewCodes)
              outData << data[i];
              ++i;
          }
          else break;
          
        if (data[i] == ',')
        {
            if (ViewCodes)
              outData << data[i];
              ++i;
            for (j = 0; j < 2; j++)
            if (data[i] >= '0' && data[i] <= '9')
            {
              if (ViewCodes)
                outData << data[i];
                ++i;
            }
            else break;
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
  bool fHistoryAdd)
{
  BString cmd;

  if (fHistoryAdd)
    cmd = fHistory->Submit (buffer);
  else
    cmd = buffer;

  if (clear) fInput->SetText ("");
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
  bool addtofHistory (true);

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
  bool autoexec (msg->FindBool ("autoexec"));
  if (autoexec)
    addtofHistory = false;

  BMessenger agentMsgr (agent);
  BMessage submitMsg (M_SUBMIT);
  submitMsg.AddBool ("history", addtofHistory);
  for (i = 0; (msg->HasString ("data", i)) && (agentMsgr.IsValid()) && (false == agent->CancelMultilineTextPaste()); ++i)
  {
    BString data;


    msg->FindString ("data", i, &data);
    
    // add a space so /'s don't get triggered as commands
    if (!autoexec)
      data.Prepend (" ");
    
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
  // displays normal text if no color codes are present
  // (i.e. if the text has already been filtered by ServerAgent::FilterCrap
  ParsemIRCColors (buffer, fore, back, font);

  if (IsHidden())
  {
    BMessage statusMsg (M_CW_UPDATE_STATUS);
    statusMsg.AddPointer ("item", fAgentWinItem);
    statusMsg.AddInt32 ("status", WIN_PAGESIX_BIT);
    Window()->PostMessage (&statusMsg);
  }

  if (fIsLogging)
  {
    BMessage logMessage (M_CLIENT_LOG);
    logMessage.AddString ("name", fId.String());
    logMessage.AddString ("data", buffer);
    fSMsgr.SendMessage (&logMessage);
  }

}

void
ClientAgent::ParsemIRCColors (
  const char *buffer,
  uint32 fore,
  uint32 back,
  uint32 font)
{
  int mircFore (fore),
        mircBack (back),
        mircFont (font),
        i (0);
        
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
        break;
      }
      else
      {
        // parse colors
        mircFore = 0;
        for (i = 0; i < 2; i++)
        {
          if (!isdigit (*buffer))
            break;
          mircFore = mircFore * 10 + *buffer++ - '0';
        }
        mircFore = (mircFore % 16) + C_MIRC_WHITE;
        
        if (*buffer == ',')
        {
          ++buffer;
          mircBack = 0;
          for (i = 0; i < 2; i++)
          {
            if (!isdigit (*buffer))
              break;
            mircBack = mircBack * 10 + *buffer++ - '0';
          }
          mircBack = (mircFore % 16) + C_MIRC_WHITE;
        }
      }
      // set start to text portion (we have recorded the mirc stuff)
      start = buffer;
    }
    if (buffer - start > 0)
      fText->Append (start, buffer - start, mircFore, mircBack, mircFont);
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
ClientAgent::UpdateStatus (int32 status)
{
  BMessage statusMsg (M_CW_UPDATE_STATUS);
  statusMsg.AddPointer ("item", fAgentWinItem);
  statusMsg.AddInt32 ("status", status);
  Window()->PostMessage (&statusMsg);
  if (status == WIN_NICK_BIT)
    system_beep(kSoundEventNames[(uint32)seNickMentioned]);
}

void
ClientAgent::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    // 22/8/99: this will now look for "text" to add to the
    // fInput view. -jamie
    case M_INPUT_FOCUS:
      {
        if (msg->HasString ("text"))
        {
          BString newtext;
          newtext = fInput->Text();
          newtext.Append (msg->FindString("text"));
          fInput->SetText (newtext.String());
        }
        fInput->MakeFocus (true);
        // We don't like your silly selecting-on-focus.
        fInput->TextView()->Select (fInput->TextView()->TextLength(),
        fInput->TextView()->TextLength());
      }
      break;

    case M_CLIENT_QUIT:
      {
        if (fIsLogging)
        {
          BMessage logMessage (M_UNREGISTER_LOGGER);
          logMessage.AddString ("name", fId.String());
          fSMsgr.SendMessage (&logMessage);
        }
      }
      break;

    case M_THEME_FOREGROUND_CHANGE:
      {
        int16 which (msg->FindInt16 ("which"));
        if (which == C_INPUT || which == C_INPUT_BACKGROUND)
        {
          fActiveTheme->ReadLock();
          rgb_color fInputColor (fActiveTheme->ForegroundAt (C_INPUT));
          fInput->TextView()->SetFontAndColor (&fActiveTheme->FontAt(F_INPUT), B_FONT_ALL,
             &fInputColor);
          fInput->TextView()->SetViewColor (fActiveTheme->ForegroundAt (C_INPUT_BACKGROUND));
          fActiveTheme->ReadUnlock();
          fInput->TextView()->Invalidate();
        }
      }
      break;
     
    case M_THEME_FONT_CHANGE:
      {
        int16 which (msg->FindInt16 ("which"));
        if (which == F_INPUT)
        {
          fActiveTheme->ReadLock();
          rgb_color fInputColor (fActiveTheme->ForegroundAt (C_INPUT));
          fInput->TextView()->SetFontAndColor (&fActiveTheme->FontAt (F_INPUT), B_FONT_ALL,
            &fInputColor);
          fActiveTheme->ReadUnlock();
          Invalidate();
        }
      }
      break; 

    case M_STATE_CHANGE:
      {
        if (msg->HasBool ("bool"))
        {
          bool shouldStamp (vision_app->GetBool ("timestamp"));
          if (fTimeStampState != shouldStamp)
          {
            if ((fTimeStampState = shouldStamp))
              fText->SetTimeStampFormat (vision_app->GetString ("timestamp_format"));
            else
              fText->SetTimeStampFormat (NULL);
          }
          
          bool shouldLog = vision_app->GetBool ("log_enabled");
          
          if (fIsLogging != shouldLog)
          {
            if ((fIsLogging = shouldLog))
            {
              BMessage logMessage (M_REGISTER_LOGGER);
              logMessage.AddString ("name", fId.String());
              fSMsgr.SendMessage (&logMessage);
            } 
            else
            {
              BMessage logMessage (M_UNREGISTER_LOGGER);
              logMessage.AddString ("name", fId.String());
              fSMsgr.SendMessage (&logMessage);
            }
          }
        }
      }
      break;
     
    case M_SUBMIT_INPUT:
      {
      	fCancelMLPaste = false;
        int32 which (0);

        msg->FindInt32 ("which", &which);

        if (msg->HasPointer ("invoker"))
        {
          BInvoker *invoker (NULL);
          msg->FindPointer ("invoker", reinterpret_cast<void **>(&invoker));
          delete invoker;
        }
        
        switch (which)
        {
        	case PASTE_CANCEL:
              break;
            
            case PASTE_MULTI:
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
            break;
            
            case PASTE_SINGLE:
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
                  fInput->TextView()->Delete (start, finish);

                if ((start == 0) && (finish == 0))
                {
                   fInput->TextView()->Insert (fInput->TextView()->TextLength(),
                     buffer.String(), buffer.Length());
                   fInput->TextView()->Select (fInput->TextView()->TextLength(),
                     fInput->TextView()->TextLength());
                }
                else
                {
                  fInput->TextView()->Insert (start, buffer.String(), buffer.Length());
                  fInput->TextView()->Select (start + buffer.Length(),
                    start + buffer.Length());
                }
              }
              else
              {
                fInput->TextView()->Insert (buffer.String());
                fInput->TextView()->Select (fInput->TextView()->TextLength(),
                fInput->TextView()->TextLength());
              }
              fInput->TextView()->ScrollToSelection();
            }
            break;
        
          default:
            break;
        }
      }
      break;

    case M_PREVIOUS_INPUT:
      {
        fHistory->PreviousBuffer (fInput);
      }
      break;

    case M_NEXT_INPUT:
      {
        fHistory->NextBuffer (fInput);
      }
      break;

    case M_SUBMIT:
      {
        const char *buffer (NULL);
        bool clear (true),
        add2fHistory (true);

        msg->FindString ("input", &buffer);

        if (msg->HasBool ("clear"))
          msg->FindBool ("clear", &clear);

        if (msg->HasBool ("history"))
          msg->FindBool ("history", &add2fHistory);

        Submit (buffer, clear, add2fHistory);
      }
      break;

    case M_LAG_CHANGED:
      {
        msg->FindString ("lag", &fMyLag);

        if (!IsHidden())
          vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String());
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
          Display ("<", theNick == fMyNick ? C_MYNICK : C_NICK);
          Display (theNick.String(), C_NICKDISPLAY);
          Display (">", theNick == fMyNick ? C_MYNICK : C_NICK);
          tempString += " ";
          tempString += theMessage;
          tempString += '\n';
        }

        // scan for presence of nickname, highlight if present
        FirstKnownAs (tempString, knownAs, &hasNick);

        tempString.Prepend (nickString);
 
        int32 dispColor = C_TEXT;
        if (hasNick)
          dispColor = C_MYNICK;
        else if (isAction)
          dispColor = C_ACTION;
        
        Display (tempString.String(), dispColor);
      }
      break;

    case M_CHANGE_NICK:
      {
        const char *oldNick;

        msg->FindString ("oldnick", &oldNick);

        if (fMyNick.ICompare (oldNick) == 0)
          fMyNick = msg->FindString ("newnick");
         
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
    
    case M_LOOKUP_ACRONYM:
      {
        BString lookup;
        msg->FindString ("string", &lookup);
        lookup = StringToURI (lookup.String());
        lookup.Prepend ("http://www.acronymfinder.com/af-query.asp?String=exact&Acronym=");
        lookup.Append ("&Find=Find");
        vision_app->LoadURL (lookup.String());
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
          completionMsg ("[@] "),
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
          completionMsg << S_CLIENT_DCC_SUCCESS;
        else completionMsg << S_CLIENT_DCC_FAILED;
				
        if (type == "SEND")
          completionMsg << S_CLIENT_DCC_SENDTYPE << pFile.Leaf() << S_CLIENT_DCC_TO;
        else completionMsg << S_CLIENT_DCC_RECVTYPE << pFile.Leaf() << S_CLIENT_DCC_FROM;
				
        completionMsg << nick << " (";

        if (!completed)
          completionMsg << fAck << "/";
				
        completionMsg << size << S_CLIENT_DCC_SIZE_UNITS "), ";
        completionMsg	<< rate << S_CLIENT_DCC_SPEED_UNITS "\n";
					
        Display (completionMsg.String(), C_CTCP_RPY);
	  }
	  break;

    default:
      BView::MessageReceived (msg);
  }
}


const BString &
ClientAgent::Id (void) const
{
  return fId;
}

int32
ClientAgent::Sid (void) const
{
  //return const_cast<long int>(fSid);
  return fSid;
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
  
  if ((place = FirstSingleKnownAs (data, fMyNick)) != B_ERROR)
  {
    result = fMyNick;
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
  ||   ispunct (data[place + target.Length()])
  ||   (int)data[place + target.Length()] <= 0xa)) // null or newline
    return place;

  return B_ERROR;
}

void
ClientAgent::AddSend (BMessage *msg, const char *buffer)
{
  if (strcmp (buffer, endl) == 0)
  {
    if (fSMsgr.IsValid())
      fSMsgr.SendMessage (msg);
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

  fMsgr.SendMessage (&msg);
}

void
ClientAgent::ActionMessage (
  const char *msgz,
  const char *nick)
{
  BMessage actionSend (M_SERVER_SEND);

  AddSend (&actionSend, "PRIVMSG ");
  AddSend (&actionSend, fId);
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
