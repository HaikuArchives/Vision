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

#include <TextControl.h>
#include <ScrollView.h>

#include <stdio.h>
#include <ctype.h>

#include "Vision.h"
#include "HistoryMenu.h"
#include "IRCView.h"
#include "ClientWindow.h"
#include "ClientAgent.h"
#include "ClientInputFilter.h"
#include "StringManip.h"
#include "ServerAgent.h"
#include "WindowList.h"


const char *ClientAgent::endl						("\1\1\1\1\1");

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
    B_WILL_DRAW | B_FRAME_EVENTS),
    
  id (id_),
  sid (sid_),
  serverName (serverName_),
  myNick (myNick_),
  timeStampState (vision_app->GetBool ("timestamp")),
  frame (frame_),
  sMsgr (sMsgr_)
{
  Init();
}

ClientAgent::~ClientAgent (void)
{
  //
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

//  printf ("clientagent::allattached (%s)\n", id.String());
}

void
ClientAgent::Show (void)
{
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
	textColor		= vision_app->GetColor (C_TEXT);
	nickColor		= vision_app->GetColor (C_NICK);
	ctcpReqColor	= vision_app->GetColor (C_CTCP_REQ);
	quitColor		= vision_app->GetColor (C_QUIT);
	errorColor		= vision_app->GetColor (C_ERROR);
	whoisColor		= vision_app->GetColor (C_WHOIS);
	joinColor		= vision_app->GetColor (C_JOIN);
	myNickColor		= vision_app->GetColor (C_MYNICK);
	actionColor		= vision_app->GetColor (C_ACTION);
	opColor			= vision_app->GetColor (C_OP);
	inputColor		= vision_app->GetColor (C_INPUT);
	
	myFont			= *(vision_app->GetClientFont (F_TEXT));
	serverFont		= *(vision_app->GetClientFont (F_SERVER));
	inputFont   	= *(vision_app->GetClientFont (F_INPUT));

  input = new BTextControl (
                BRect (
                  0,
                  frame.top, // tmp. will be moved
                  frame.right - 117,
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
           frame.bottom - input->Frame().Height());

  input->TextView()->AddFilter (new ClientInputFilter (this));
  AddChild (input);

  input->TextView()->SetViewColor (vision_app->GetColor (C_INPUT_BACKGROUND));
  input->Invalidate(); 
  
  history = new HistoryMenu (BRect (
    frame.right - 11,
    input->Frame().top,
    frame.right - 1,
    input->Frame().bottom - 1));

  BRect textrect (
    0,
    frame.top - 1,
    frame.right - 117 - B_V_SCROLL_BAR_WIDTH,
    frame.bottom - input->Frame().Height() - 1);

  text = new IRCView (
    textrect,
    textrect.InsetByCopy (5, 3),
    input);
  text->SetViewColor (vision_app->GetColor (C_BACKGROUND));
  

  textScroll = new BScrollView (
    "textscroll",
    text,
    B_FOLLOW_ALL_SIDES,
    0,
    false,
    true,
    B_NO_BORDER);
  AddChild (textScroll);

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

  for (i = 0; msg->HasString ("data", i); ++i)
  {
    const char *data;

    msg->FindString ("data", i, &data);

    // :TODO: wade 020101 move locks to ParseCmd?
    if (window->Lock())
    {
      if (!agent->SlashParser (data))
        agent->Parser (data);

      window->Unlock();

      // A small attempt to appease the
      // kicker gods
      snooze (75000);
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

    #if 0
	if (isLogging)
	{
		BString printbuf;
		if (timeStamp)
		{
			printbuf << TimeStamp().String();
		}
		
		printbuf << buffer;
		
		off_t len = strlen (printbuf.String());
		logFile.Write (printbuf.String(), len);
	}
	#endif

    // :TODO: wade 020101: tie to settings
    timeStampState = vision_app->GetBool ("timestamp");
    //

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

			break;
		}


		// This could probably be used for some scripting
		// capability -- oooh, neato!
		case M_SUBMIT_RAW:
		{
			bool lines;

			msg->FindBool ("lines", &lines);

			if (lines)
			{
				BMessage *buffer (new BMessage (*msg));
				thread_id tid;

				buffer->AddPointer ("client", this);

				tid = spawn_thread (
					TimedSubmit,
					"Timed Submit",
					B_LOW_PRIORITY,
					buffer);

				resume_thread (tid);
			}
			else
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
					{
						input->TextView()->Delete (start, finish);
					}
					
					
					if ((start == 0) && (finish == 0))
					{
						input->TextView()->Insert (input->TextView()->TextLength(), buffer.String(), buffer.Length());
						input->TextView()->Select (input->TextView()->TextLength(),
							input->TextView()->TextLength());
					}
					else
					{				
						input->TextView()->Insert (start, buffer.String(), buffer.Length());
						input->TextView()->Select (start + buffer.Length(), start + buffer.Length()); 				
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

			break;
		}

		case M_PREVIOUS_INPUT:
			history->PreviousBuffer (input);
			break;

		case M_NEXT_INPUT:
			history->NextBuffer (input);
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
			break;
		}

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
			break;
		}
		
		case M_CHANNEL_MSG:
		{
			const char *theNick;
			const char *theMessage;
			bool me (false), gotOther (false), gotNick (false);
			BString knownAs;

			msg->FindString("nick", &theNick);
			msg->FindString("msgz", &theMessage);

			if (FirstKnownAs (theNick, knownAs, &me) != B_ERROR && !me)
				gotOther = true;


			if (theMessage[0] == '\1')
			{
				BString aMessage (theMessage);

				aMessage.Truncate (aMessage.Length() - 1);				
				aMessage.RemoveFirst ("\1ACTION ");
				aMessage.RemoveLast ("\1");	// JAVirc appends an illegal space at
											// the end, so .Truncate doesn't remove
											// the \1
				
				BString tempString("* ");
				tempString += theNick;
				tempString += " ";
				tempString += aMessage;
				tempString += '\n';
				Display (tempString.String(), &actionColor, 0, true);
			}
			else
			{
				Display ("<", 0, 0, true);

				Display (
					theNick,
					theNick == myNick ? &myNickColor : &nickColor);

				BString tempString;
				tempString += "> ";
				tempString += theMessage;
				tempString += '\n';

				int32 place;

				while ((place = FirstKnownAs (tempString, knownAs, &me)) !=
					B_ERROR)
				{
					BString buffer;

					if (place)
					{
						tempString.MoveInto (buffer, 0, place);
						Display (buffer.String(), 0);
					}

					tempString.MoveInto (buffer, 0, knownAs.Length());

					if (me)
					{
						Display (buffer.String(), &myNickColor);
						gotNick = true;
					}
					else
					{
						Display (buffer.String(), &nickColor);
						gotOther = true;
					}
				}

				Display (tempString.String(), 0);
			}

			if (IsHidden())
			{
				if (gotNick || gotOther)
				{ 
    				BMessage statusMsg (M_UPDATE_STATUS);
    				statusMsg.AddPointer ("item", agentWinItem);
    				statusMsg.AddInt32 ("status", WIN_NICK_BIT);
    				Window()->PostMessage (&statusMsg);
				}
			}
			
			break;
		}
		
		case M_CHANGE_NICK:
		{
		    printf ("M_CHANGE_NICK\n");
			const char *oldNick;

			msg->FindString ("oldnick", &oldNick);

			if (myNick.ICompare (oldNick) == 0)
				myNick = msg->FindString ("newnick");

			BMessage display;
			if (msg->FindMessage ("display", &display) == B_NO_ERROR)
				ClientAgent::MessageReceived (&display);

			break;
		}
		
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
  return const_cast<long int>(sid);
}

int32
ClientAgent::FirstKnownAs (
	const BString &data,
	BString &result,
	bool *me)
{
  // :TODO: wade 020401: make also known as work...
  BString alsoKnownAs ("-9z99");
  BString otherNick ("-9z99");
  //
	
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

  for (i = 1; (target = GetWord (alsoKnownAs.String(), i)) != "-9z99"; ++i)
  {
    if ((place = FirstSingleKnownAs (data, target)) != B_ERROR
    &&   place < hit)
    {
      result = target;
      hit = place;
      *me = true;
    }
  }

  for (i = 1; (target = GetWord (otherNick.String(), i)) != "-9z99"; ++i)
  {
    if ((place = FirstSingleKnownAs (data, target)) != B_ERROR
    &&   place < hit)
    {
      result = target;
      hit = place;
      *me = false;
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

	if (nick)    msg.AddString ("nick", nick);
	if (ident)   msg.AddString ("ident", ident);
	if (address) msg.AddString ("address", address);

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
  BString theCTCP = GetWord(theMsg.String(), 1).ToUpper();
  BString theRest = RestOfString(theMsg.String(), 2);
	
  BString tempString ("[CTCP->");
	
  tempString << theTarget << "] " << theCTCP;
	
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
