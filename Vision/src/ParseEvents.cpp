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
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 *								 Bjorn Oksholen
 *								 Alan Ellis <alan@cgsoftware.org>
 */
 
#include <Catalog.h>
 
#include "Vision.h"
#include "Utilities.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "MessageAgent.h"
#include "ClientWindow.h"
#include "WindowList.h"

#include <stdio.h>

/* #6502 was here --
<kurros> regurg, amiyumi?
<regurg> Let's discuss further why your pizza NOW.
<kurros> uh
<slaad> ?
<kurros> yeah
*/

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerMessages"

bool
ServerAgent::ParseEvents (const char *data)
{
	BString firstWord = GetWord(data, 1).ToUpper();
	BString secondWord = GetWord(data, 2).ToUpper();

	if (secondWord == "PRIVMSG")
	{
		BString theNick (GetNick (data)),
						ident (GetIdent (data)),
						address (GetAddress (data)),
						addy;
		
		addy += ident;
		addy += "@";
		addy += address;

		BString theTarget (GetWord (data, 3).ToUpper()),
						theTargetOrig (GetWord (data, 3)),
						theMsg (RestOfString (data, 4));
		
		ClientAgent *client (0);

		theMsg.RemoveFirst(":");
		
		if (theMsg.Length() == 0)
			return true;

		if (theMsg[0] == '\1' && GetWord (theMsg.String(), 1) != "\1ACTION")
		{
			// CTCP Request
			ParseCTCP (theNick, theTargetOrig, theMsg);
			return true;
		}

		if (theTarget[0] == '#'
		||	theTarget[0] == '!'
		||	theTarget[0] == '&'
		||	theTarget[0] == '+')
			client = Client (theTarget.String());
		else if (!(client = Client (theNick.String())))
		{
			BString msgident (GetIdent (data)),
							msgaddress (GetAddress (data));
			
			client = new MessageAgent (
				*vision_app->pClientWin()->AgentRect(),
				theNick.String(),
				fId.String(),
				fSMsgr,
				fMyNick.String(),
				addy.String()),

			vision_app->pClientWin()->pWindowList()->AddAgent (client,
				theNick.String(),
				WIN_MESSAGE_TYPE,
				false);

			fClients.AddItem (client);
		}

		if (client)
		{
			BString msgident (GetIdent (data)),
							msgaddress (GetAddress (data));

			client->ChannelMessage (
				theMsg.String(),
				theNick.String(),
				msgident.String(),
				msgaddress.String());
		}

		return true;
	}

	if (firstWord == "NOTICE")
	{
		BString theNotice (RestOfString(data, 3));
		theNotice.RemoveFirst(":");

		BString tempString;
		
		const char *expansions[2];
		expansions[0] = fServerHostName.String();
		expansions[1] = theNotice.String();
		tempString = ExpandKeyed (fEvents[E_SNOTICE].String(), "NR", expansions);
		Display (tempString.String(), C_NOTICE);
		
		return true;
	}

	if (secondWord == "NOTICE")
	{
		BString theNotice (RestOfString(data, 4));
		theNotice.RemoveFirst(":");

		BString tempString;

		firstWord.RemoveFirst (":");

		if (firstWord.ICompare (fServerHostName) == 0)
		{
			const char *expansions[2];
			expansions[0] = fServerHostName.String();
			expansions[1] = theNotice.String();
			tempString = ExpandKeyed (fEvents[E_SNOTICE].String(), "NR", expansions);
			Display (tempString.String(), C_NOTICE);

			return true;
		}
		else
		{
			BString theNick (GetNick (data)),
							ident (GetIdent (data)),
							address (GetAddress (data));
			
			if ((theNotice.Length() > 0) && theNotice[0] == '\1')
			{
				// CTCP reply
				ParseCTCPResponse (theNick, theNotice);
				return true;
			}

			const char *expansions[4];
			expansions[0] = theNick.String();
			expansions[1] = theNotice.String();
			expansions[2] = ident.String();
			expansions[3] = address.String();

			tempString = ExpandKeyed (fEvents[E_UNOTICE].String(), "NRIA", expansions);
			BMessage display (M_DISPLAY);
			PackDisplay (&display, tempString.String(), C_NOTICE);
			PostActive (&display);
			return true;
		}
	}

	if (secondWord == "JOIN")
	{
		BString nick (GetNick (data)),
						channel (GetWord (data, 3));

		channel.RemoveFirst (":");
		ClientAgent *client (Client (channel.String()));

		if (nick == fMyNick)
		{
			bool activateChan (true);
			int32 chanCount (0);
			if ((chanCount = fStartupChannels.CountItems()) > 0)
			{
				for (int32 i = 0; i < chanCount; i++)
					if ((fStartupChannels.ItemAt (i)->ICompare (channel)) == 0)
					{
						 delete fStartupChannels.RemoveItemAt (i);
						 activateChan = false;
						 break;
					}
			}
			if (!client)
			{
				ChannelAgent *newAgent (new ChannelAgent (
						channel.String(),
						fId.String(),
						fIrcdtype,
						fMyNick.String(),
						fSMsgr,
						*vision_app->pClientWin()->AgentRect()));
						
				vision_app->pClientWin()->pWindowList()->AddAgent (newAgent,
					channel.String(),
					WIN_CHANNEL_TYPE,
					activateChan);

				fClients.AddItem (newAgent);
			}

			BString tempString ("MODE ");
			tempString += channel;
			SendData (tempString.String());
		}
		else if (client)
		{
			// someone else
			BString ident (GetIdent (data)),
							address (GetAddress (data)),
							tempString;
							
			const char *expansions[3];
			expansions[0] = nick.String();
			expansions[1] = ident.String();
			expansions[2] = address.String();

			tempString = ExpandKeyed (fEvents[E_JOIN].String(), "NIA", expansions);
			
			BMessage display (M_DISPLAY);
			
			PackDisplay (
				&display,
				tempString.String(),
				C_JOIN);
			
			// add ignore code here
			bool ignored (false);

			BMessage msg (M_USER_ADD);
			msg.AddString ("nick",	nick.String());
			msg.AddBool ("ignore", ignored);
			// don't bother displaying if the user puts an empty string
			if (tempString.Length() > 1)
			{
				msg.AddMessage ("display", &display);
			}
			client->fMsgr.SendMessage (&msg);
		}

		return true;
	}

	if (secondWord == "PART")
	{
		BString nick (GetNick (data)),
						channel (GetWord (data, 3)),
						reason (RestOfString(data, 4));
						

		// some servers seem to add this, shouldn't be there though
		channel.RemoveFirst(":");
		reason.RemoveFirst(":");
		
		if (reason == "-9z99")
		{
			reason = "";
		}

		ClientAgent *client;

		if ((client = Client (channel.String())) != 0)
		{
			BString ident (GetIdent (data)),
							address (GetAddress (data)),
							buffer;
			
			const char *expansions[4];
			expansions[0] = nick.String();
			expansions[1] = ident.String();
			expansions[2] = address.String();
			expansions[3] = reason.String();

			buffer = ExpandKeyed (fEvents[E_PART].String(), "NIAR", expansions);

			BMessage display (M_DISPLAY);
			PackDisplay (&display, buffer.String(), C_JOIN);

			BMessage msg (M_USER_QUIT);
			msg.AddString ("nick", nick.String());
			if (buffer.Length() > 1)
			{
				msg.AddMessage ("display", &display);
			}
			client->fMsgr.SendMessage (&msg);
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
				
		newNick.RemoveFirst (":");
										
		const char *expansions[4];
		expansions[0] = oldNick.String();
		expansions[1] = newNick.String();
		expansions[2] = ident.String();
		expansions[3] = address.String();

		buffer = ExpandKeyed (fEvents[E_NICK].String(), "NnIA", expansions);
		BMessage display (M_DISPLAY);
		PackDisplay (&display, buffer.String(), C_NICK);

		BMessage msg (M_CHANGE_NICK);
		msg.AddString ("oldnick", oldNick.String());
		msg.AddString ("newnick", newNick.String());
		msg.AddString ("ident", ident.String());
		msg.AddString ("address", address.String());
		msg.AddMessage ("display", &display);

		Broadcast (&msg);

		// Gotta change the server as well!
		if (fMyNick.ICompare (oldNick) == 0)
		{
			fMyNick = newNick;
			if (!fReacquiredNick && (fMyNick == fReconNick))
				fReacquiredNick = true;
			if (!IsHidden())
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK,
																															 newNick.String());
		}

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
	
		theRest.RemoveFirst (":");
	 
		const char *expansions[4];
		expansions[0] = theNick.String();
		expansions[1] = theRest.String();
		expansions[2] = ident.String();
		expansions[3] = address.String();

		theMsg = ExpandKeyed (fEvents[E_QUIT].String(), "NRIA", expansions);
		
		BMessage display (M_DISPLAY);
		PackDisplay (&display, theMsg.String(), C_QUIT);

		BMessage msg (M_USER_QUIT);

		if (theMsg.Length() > 1)
		{
			msg.AddMessage ("display", &display);
		}
		msg.AddString ("nick", theNick.String());

		Broadcast (&msg);

		// see if we had this nickname previously.
		// (we might have been disconnected and this could be a
		//	connection waiting to time out)
		if (theNick == fReconNick)
		{
			BString tempCmd ("/nick ");
			tempCmd += fReconNick;
			ParseCmd (tempCmd.String());
		}

		return true;
	}

	if (firstWord == "PING")
	{
		BString tempString,
						theServer (GetWord(data, 2));
		
		theServer.RemoveFirst(":");

		tempString += "PONG ";
		tempString += fMyNick;
		tempString += " ";
		tempString += theServer;
		SendData (tempString.String());

		// some noncompliant servers dont like the above (correct) reply,
		// so we send this one, too.
		// this can't be contained in an ircdtype if because the first reply
		// is sent before we receive the server version.
		tempString = "";
		tempString += "PONG ";
		tempString += theServer;
		SendData (tempString.String());
	
		return true;
	}

	if (secondWord == "KICK")
	{
		BString kicker (GetNick (data)),
						kickee (GetWord (data, 4)),
						rest (RestOfString (data, 5)),
						channel (GetWord (data, 3));

		ClientAgent *client (Client (channel.String()));

		rest.RemoveFirst (":");

		if ((client = Client (channel.String())) != 0
		&&	 kickee == fMyNick)
		{
			BMessage msg (M_CHANNEL_GOT_KICKED);
			msg.AddString ("channel", channel.String());
			msg.AddString ("kicker", kicker.String());
			msg.AddString ("rest", rest.String());
			client->fMsgr.SendMessage (&msg);
		}

		if (client && kickee != fMyNick)
		{
			BMessage display (M_DISPLAY);
			const char *expansions[4];
			BString buffer;

			expansions[0] = kickee.String();
			expansions[1] = channel.String();
			expansions[2] = kicker.String();
			expansions[3] = rest.String();

			buffer = ExpandKeyed (fEvents[E_KICK].String(), "NCnR", expansions);
			PackDisplay (&display, buffer.String(), C_QUIT);

			BMessage msg (M_USER_QUIT);
			msg.AddString ("nick", kickee.String());
			msg.AddMessage ("display", &display);
			client->fMsgr.SendMessage (&msg);
		}

		return true;
	}
	 
	if (secondWord == "TOPIC")
	{
		BString theNick (GetNick (data)),
						theChannel (GetWord (data, 3)),
						theTopic (RestOfString (data, 4));

		ClientAgent *client (Client (theChannel.String()));

		theTopic.RemoveFirst (":");

		if (client)
		{
			BString ident (GetIdent (data)),
							address (GetAddress (data)),
							buffer;

			const char *expansions[5];
			expansions[0] = theNick.String();
			expansions[1] = theTopic.String();
			expansions[2] = client->Id().String();
			expansions[3] = ident.String();
			expansions[4] = address.String();

			BMessage topic (M_CHANNEL_TOPIC);

			topic.AddString("topic", theTopic.String());

			BMessage display (M_DISPLAY);

			buffer = ExpandKeyed (fEvents[E_TOPIC].String(), "NTCIA", expansions);
			PackDisplay (&display, buffer.String(), C_WHOIS);
			topic.AddMessage("display", &display);
			client->fMsgr.SendMessage (&topic);
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

			client->fMsgr.SendMessage (&msg);
		}
		else
		{
			BMessage msg (M_DISPLAY);
			BString buffer = "*** ";

			theMode.RemoveFirst(":");

			buffer += B_TRANSLATE("User mode changed: %1");
			buffer.ReplaceFirst("%1", theMode);
			buffer += "\n";
			
			PackDisplay (&msg, buffer.String());
			PostActive (&msg);
		}

		return true;
	}

	if (firstWord == "ERROR") // server error (on connect?)
	{
		BString theError (RestOfString (data, 2));

		theError.RemoveFirst (":");
		theError.Append ("\n");

		Display (theError.String(), C_QUIT);
		
		return true;
	}

	if (secondWord == "WALLOPS")
	{
		BString theNick (GetNick (data)),
						theWall (RestOfString (data, 3)),
						tempString;

		theWall.RemoveFirst (":");
		theWall.Append ("\n");

		tempString += "!";
		tempString += theNick;
		tempString += "! ";
		tempString += theWall;
		
		if (IsHidden())
		{
			BMessage statusMsg (M_CW_UPDATE_STATUS);
			statusMsg.AddPointer ("item", fAgentWinItem);
			statusMsg.AddInt32 ("status", WIN_NEWS_BIT);
			Window()->PostMessage (&statusMsg);
		}
		
		Display (tempString.String(), C_WALLOPS);
		return true;			
	}

	if (secondWord == "INVITE")
	{
		BString tempString("*** "),
						theChannel (GetWord(data, 4));

		theChannel.RemoveFirst(":");

		tempString = B_TRANSLATE("You have been invited to %1 by %2.");
		tempString.ReplaceFirst("%1", theChannel);
		tempString.ReplaceFirst("%2", GetNick(data));
		tempString += "\n";

		BMessage msg (M_DISPLAY);

		PackDisplay (&msg, tempString.String(), C_WHOIS);
		PostActive (&msg);

		return true;
	}

	if (secondWord == "SILENCE")
	{
		BString tempString ("*** "),
						theHostmask (GetWord(data, 3));	// Could be a hostmask, a nick, whatever
		const char *hostmask = theHostmask.String();
		
		if (hostmask[0] == '+') {
				tempString += B_TRANSLATE("Hostmask added to SILENCE list: %1.");
				theHostmask.RemoveFirst("+");
		} else {
				tempString += B_TRANSLATE("Hostmask removed from SILENCE list: %1.");
					theHostmask.RemoveFirst("-");
		}

		tempString.ReplaceFirst("%1", theHostmask);
		tempString += "\n";

		BMessage msg (M_DISPLAY);

		PackDisplay (&msg, tempString.String(), C_WHOIS);
		PostActive (&msg);

		return true;
	}

	// ship off to parse numerics
	return ParseENums (data, secondWord.String());
	
}

