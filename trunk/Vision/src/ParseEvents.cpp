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
 
#include "Vision.h"
#include "StringManip.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "ClientWindow.h"
#include "WindowList.h"

#include <stdio.h>

bool
ServerAgent::ParseEvents (const char *data)
{
	BString firstWord = GetWord(data, 1).ToUpper();
	BString secondWord = GetWord(data, 2).ToUpper();

	if(secondWord == "PRIVMSG")
	{
		BString theNick (GetNick (data)),
			ident (GetIdent (data)),
			address (GetAddress (data)),
			addy;


		addy << ident << "@" << address;
		//BMessage aMsg (M_IS_IGNORED), reply;
//		bool ignored (false);
//
//		aMsg.AddString ("server", serverName.String());
//		aMsg.AddString ("nick", theNick.String());
//		aMsg.AddString ("address", addy.String());
//
//		be_app_messenger.SendMessage (&aMsg, &reply);
//		reply.FindBool ("ignored", &ignored);
//
//		if (ignored)
//		{
//			BMessage msg (M_IGNORED_PRIVMSG);
//			const char *rule;
//
//			reply.FindString ("rule", &rule);
//			msg.AddString ("nick", theNick.String());
//			msg.AddString ("address", addy.String());
//			msg.AddString ("rule", rule);
//			Broadcast (&msg);
//			return true;
//		}

		BString theTarget (GetWord (data, 3).ToUpper()),
		        theTargetOrig (GetWord (data, 3)),
		        theMsg (RestOfString (data, 4));
		ClientAgent *client (0);

		theMsg.RemoveFirst(":");

		if(theMsg[0] == '\1' && GetWord(theMsg.String(), 1) != "\1ACTION") // CTCP
		{
			ParseCTCP (theNick, theTargetOrig, theMsg);
			return true;
		}

		if (theTarget[0] == '#')
			client = Client (theTarget.String());
//		else if (!(client = Client (theNick.String())))
//		{
//			BString ident = GetIdent(data);
//			BString address = GetAddress(data);
//			BString addyString;
//			addyString << ident << "@" << address;
//
//			client = new MessageWindow (
//				theNick.String(),
//				sid,
//				serverName.String(),
//				sMsgr,
//				myNick.String(),
//				addyString.String(),
//				false, // not a dcc chat
//				true); // initated by server
//
//			clients.AddItem (client);
//			client->Show();
//		}

		if (client) client->ChannelMessage (
			theMsg.String(),
			theNick.String(),
			ident.String(),
			address.String());

		return true;
	}

	if(firstWord == "NOTICE") // _server_ notice
	{
		BString theNotice (RestOfString(data, 3));
		const char *expansions[1];

		theNotice.RemoveFirst(":");

		expansions[0] = theNotice.String();
		theNotice = ExpandKeyed (events[E_SNOTICE].String(), "R", expansions);
		Display (theNotice.String(), 0, 0, true);

		return true;
	}

	if(secondWord == "NOTICE") // _user_ notice
	{
		BString theNick (GetNick (data)),
			ident (GetIdent (data)),
			address (GetAddress (data)),
			addy;

		addy << theNick << "@" << address;

//		BMessage aMsg (M_IS_IGNORED), reply;
//		bool ignored;
//
//		aMsg.AddString ("server", serverName.String());
//		aMsg.AddString ("nick", theNick.String());
//		aMsg.AddString ("address", addy.String());
//
//		be_app_messenger.SendMessage (&aMsg, &reply);
//		reply.FindBool ("ignored", &ignored);
//
//		if (ignored) return true;

		BString theNotice = RestOfString(data, 4);

		theNotice.RemoveFirst(":");

		if (theNotice[0] == '\1')
		{
			ParseCTCPResponse (theNick, theNotice);
			return true;
		}

		const char *expansions[4];
		BString tempString;

		expansions[0] = theNick.String();
		expansions[1] = theNotice.String();
		expansions[2] = ident.String();
		expansions[3] = address.String();

		tempString = ExpandKeyed (events[E_UNOTICE].String(), "NRIA", expansions);
		BMessage display (M_DISPLAY);
		PackDisplay (&display, tempString.String(), &noticeColor, 0, true);
		PostActive (&display);
		return true;
	}

	if (secondWord == "JOIN")
	{
		BString nick (GetNick (data));
		BString channel (GetWord (data, 3));

		channel.RemoveFirst (":");
		ClientAgent *client (Client (channel.String()));

		if (nick == myNick)
		{
			if (!client)
			{
				ClientWindow *window ((ClientWindow *)Window());
				window->UpdateAgentRect();
				
				window->winList->AddAgent (
   					new ChannelAgent (
      					channel.String(),
      					//const_cast<long int>(sid),
      					sid,
      					serverName.String(),
      					myNick.String(),
      					sMsgr,
      					*window->agentrect),
    				sid,
    				channel.String(),
    				WIN_CHANNEL_TYPE,
    				true);
    					
				clients.AddItem ((window->winList->Agent (sid, channel.String())));
			}
				
			BString tempString ("MODE ");
			tempString << channel;
			SendData (tempString.String());
		}
		else if (client)
		{
			// someone else
			BString ident (GetIdent (data));
			BString address (GetAddress (data));
			const char *expansions[3];
			BString tempString, addy;

			expansions[0] = nick.String();
			expansions[1] = ident.String();
			expansions[2] = address.String();

			tempString = ExpandKeyed (events[E_JOIN].String(), "NIA", expansions);
			BMessage display (M_DISPLAY);
			PackDisplay (
				&display,
				tempString.String(),
				&joinColor,
				0,
				true);

			addy << ident << "@" << address;

			bool ignored (false);
//			BMessage aMsg (M_IS_IGNORED), reply;
//			bool ignored;
//
//			aMsg.AddString ("server", serverName.String());
//			aMsg.AddString ("nick", nick.String());
//			aMsg.AddString ("address", addy.String());
//
//			be_app_messenger.SendMessage (&aMsg, &reply);
//			reply.FindBool ("ignored", &ignored);
			
			BMessenger adduser (client);
			BMessage msg (M_USER_ADD);
			msg.AddString ("nick",  nick.String());
			msg.AddBool ("ignore", ignored);
			msg.AddMessage ("display", &display);
			adduser.SendMessage (&msg);
		}
		return true;
	}
	
	
	if (secondWord == "PART")
	{
		BString nick (GetNick (data));
		BString channel (GetWord (data, 3));
		ClientAgent *client;

		if ((client = Client (channel.String())) != 0)
		{
			BString ident (GetIdent (data));
			BString address (GetAddress (data));
			const char *expansions[3];
			BString buffer;

			expansions[0] = nick.String();
			expansions[1] = ident.String();
			expansions[2] = address.String();

			buffer = ExpandKeyed (events[E_PART].String(), "NIA", expansions);

			BMessage display (M_DISPLAY);
			PackDisplay (&display, buffer.String(), &joinColor, 0, true);

			BMessage msg (M_USER_QUIT);
			msg.AddString ("nick", nick.String());
			msg.AddMessage ("display", &display);
			client->msgr.SendMessage (&msg);
		}

		return true;
	}
	

	if (secondWord == "NICK")
	{
		BString oldNick (GetNick (data)),
		        ident (GetIdent (data)),
		        address (GetAddress (data)),
		        newNick (GetWord (data, 3)),
		        buffer;
		const char *expansions[4];

		newNick.RemoveFirst (":");

		expansions[0] = oldNick.String();
		expansions[1] = newNick.String();
		expansions[2] = ident.String();
		expansions[3] = address.String();

		buffer = ExpandKeyed (events[E_NICK].String(), "NnIA", expansions);
		BMessage display (M_DISPLAY);
		PackDisplay (&display, buffer.String(), &nickColor, 0, vision_app->GetBool ("timestamp"));

		BMessage msg (M_CHANGE_NICK);
		msg.AddString ("oldnick", oldNick.String());
		msg.AddString ("newnick", newNick.String());
		msg.AddString ("ident", ident.String());
		msg.AddString ("address", address.String());
		msg.AddMessage ("display", &display);

		Broadcast (&msg);

		// Gotta change the server as well!
		if (myNick.ICompare (oldNick) == 0)
		{
			myNick = newNick;
			//status->SetItemValue (STATUS_NICK, newNick.String());
		}

		vision_app->PostMessage (&msg); // for ignores

		return true;
	}

	if (secondWord == "QUIT")
	{
		BString theNick (GetNick (data).String()),
		        theRest (RestOfString (data, 3)),
		        ident (GetIdent (data)),
		        address (GetAddress (data)),
		        theMsg,
		        firstNick;
		const char *expansions[4];

		theRest.RemoveFirst (":");
		
		expansions[0] = theNick.String();
		expansions[1] = theRest.String();
		expansions[2] = ident.String();
		expansions[3] = address.String();

		theMsg = ExpandKeyed (events[E_QUIT].String(), "NRIA", expansions);
		BMessage display (M_DISPLAY);
		PackDisplay (&display, theMsg.String(), &quitColor, 0, true);

		BMessage msg (M_USER_QUIT);
		msg.AddMessage ("display", &display);
		msg.AddString ("nick", theNick.String());

		Broadcast (&msg);
		
		// see if it was our first nickname. if so, change
		firstNick = (const char *)lnicks->ItemAt (0);
		if (theNick == firstNick)
		{
			BString tempCmd ("/nick ");
			tempCmd << firstNick;
			ParseCmd (tempCmd.String());
		}
				
		return true;
	}

	#if 0
	if (secondWord == "KICK")
	{
		BString kicker (GetNick (data)),
		        kickee (GetWord (data, 4)),
		        rest (RestOfString (data, 5)),
		        channel (GetWord (data, 3));
		ClientWindow *client (Client (channel.String()));

		rest.RemoveFirst (":");

		if ((client = Client (channel.String())) != 0
		&&   kickee == myNick)
		{
			BMessage display (M_DISPLAY); // "you were kicked"
			BString buffer;

			buffer << "*** You have been kicked from "
				<< channel << " by " << kicker << " (" << rest << ")\n";
			PackDisplay (&display, buffer.String(), &quitColor, 0, true);

			BMessage display2 (M_DISPLAY); // "attempting auto rejoin"
			buffer = "*** Attempting to automagically rejoin ";
			buffer << channel << "...\n";
			PackDisplay (&display2, buffer.String(), &quitColor, 0, true);
						

			BMessage msg (M_CHANNEL_GOT_KICKED);
			msg.AddString ("channel", channel.String());
			msg.AddMessage ("display", &display);  // "you were kicked"
			msg.AddMessage ("display2", &display2); // "attempting auto rejoin"
			client->PostMessage (&msg);
		}

		if (client && kickee != myNick)
		{
			BMessage display (M_DISPLAY);
			const char *expansions[4];
			BString buffer;

			expansions[0] = kickee.String();
			expansions[1] = channel.String();
			expansions[2] = kicker.String();
			expansions[3] = rest.String();

			buffer = ExpandKeyed (events[E_KICK], "NCnR", expansions);
			PackDisplay (&display, buffer.String(), &quitColor, 0, true);

			BMessage msg (M_USER_QUIT);
			msg.AddString ("nick", kickee.String());
			msg.AddMessage ("display", &display);
			client->PostMessage (&msg);
		}

		return true;
	}
    #endif
   
	if(secondWord == "TOPIC")
	{
		BString theNick (GetNick (data)),
		        theChannel (GetWord (data, 3)),
		        theTopic (RestOfString (data, 4));
		ClientAgent *client (Client (theChannel.String()));

		theTopic.RemoveFirst (":");

		if (client)
		{
			BString ident (GetIdent (data));
			BString address (GetAddress (data));
			const char *expansions[5];
			BString buffer;

			expansions[0] = theNick.String();
			expansions[1] = theTopic.String();
			expansions[2] = client->Id().String();
			expansions[3] = ident.String();
			expansions[4] = address.String();
			
			BMessage topic (M_CHANNEL_TOPIC);
			
			topic.AddString("topic", theTopic.String());
						
			BMessage display (M_DISPLAY);

			buffer = ExpandKeyed (events[E_TOPIC].String(), "NTCIA", expansions);
			PackDisplay (&display, buffer.String(), &whoisColor, 0, true);
			topic.AddMessage("display", &display);
			client->msgr.SendMessage (&topic);
		}
		return true;
	}

	if (secondWord == "MODE")
	{
		BString theNick (GetNick (data)),
		        theChannel (GetWord (data, 3)),
		        theMode (GetWord (data, 4)),
		        theTarget (RestOfString (data, 5));
		ClientAgent *client (Client (theChannel.String()));

		if (client)
		{
			BMessage msg (M_CHANNEL_MODE);

			msg.AddString("nick", theNick.String());
			msg.AddString("mode", theMode.String());
			msg.AddString("target", theTarget.String());

			client->msgr.SendMessage (&msg);
		}
		else
		{
			BMessage msg (M_DISPLAY);
			BString buffer;
			
			theMode.RemoveFirst(":");
	
			buffer << "*** User mode changed: " << theMode << "\n";
			PackDisplay (&msg, buffer.String(), 0, 0, true);

			PostActive (&msg);
		}

		return true;
	}

	if (firstWord == "ERROR") // server error (on connect?)
	{
		BString theError (RestOfString (data, 2));

		theError.RemoveAll (":");
		theError.Append ("\n");

		Display (theError.String(), &quitColor);
		isConnecting = false;
		return true;
	}

	if(firstWord == "PING")
	{
		BString tempString,
		        theServer (GetWord(data, 2));
		theServer.RemoveFirst(":");
		
		tempString += "PONG ";
		tempString += myNick;
		tempString += " ";
		tempString += theServer;
		SendData (tempString.String());
		
		// some noncompliant servers dont like the above (correct) reply,
		// so we send this one, too.
		tempString = "";
		tempString += "PONG ";
		tempString += theServer;
		SendData (tempString.String());
		return true;
	}


	return ParseENums (data, secondWord.String());
}

