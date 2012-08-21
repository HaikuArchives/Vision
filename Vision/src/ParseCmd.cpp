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
 *								 Alan Ellis <alan@cgsoftware.org>
 */

#include <Catalog.h>
#include <File.h>
#include <FilePanel.h>
#include <Path.h>
#include <Roster.h>
#include <FindDirectory.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#include <map>

#include "Vision.h"
#include "VisionBase.h"
#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "MessageAgent.h"
#include "Names.h"
#include "Utilities.h"
#include "ClientAgent.h"
#include "ClientWindow.h"
#include "RunView.h"
#include "WindowList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "CommandParser"

static void
DisplayCommandError(ClientAgent *agent, const char *commandName)
{
	BString tempString = "[x] ";
	tempString += commandName;
	tempString += " ";
	tempString += B_TRANSLATE("Error: Invalid parameters");
	agent->Display(tempString.String(), C_ERROR);
}

bool
ClientAgent::ParseCmd (const char *data)
{
	BString firstWord (GetWord(data, 1).ToUpper());
	BMessage sendMsg (M_SERVER_SEND);

	if (vision_app->HasAlias(firstWord))
	{
		return ParseCmd(vision_app->ParseAlias(data, fId).String());
	}
	
	if (firstWord == "/ADDALIAS")
	{
		vision_app->AddAlias(GetWord(data, 2).ToUpper(), RestOfString(data, 3));
		return true;
	}
	
	if (firstWord == "/DELALIAS")
	{
		vision_app->RemoveAlias(GetWord(data, 2).ToUpper());
		return true;
	}

	if (firstWord == "/WALLOPS"	// we need to insert a ':' before parm2
	||	firstWord == "/SQUIT"		// for the user
	||	firstWord == "/PRIVMSG")
	{
		BString theCmd (firstWord.RemoveAll ("/")),
						theRest (RestOfString (data, 2));

		AddSend (&sendMsg, theCmd);
		if (theRest != "-9z99")
		{
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, theRest);
		}
		AddSend (&sendMsg, endl);
		return true;
	}

	if (firstWord == "/KILL") // we need to insert a ':' before parm3
	{												 // for the user
		BString theCmd (firstWord.RemoveAll ("/")),
					 theTarget (GetWord (data, 2)),
					 theRest (RestOfString (data, 3));

		AddSend (&sendMsg, theCmd);
		AddSend (&sendMsg, " ");
		AddSend (&sendMsg, theTarget);
		if (theRest != "-9z99")
		{
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, theRest);
		}
		AddSend (&sendMsg, endl);
		return true;
	}


	// some quick aliases for scripts, these will of course be
	// moved to an aliases section eventually

	if (firstWord == "/SOUNDPLAY"
	||	firstWord == "/CL-AMP")
	{
		app_info ai;
		be_app->GetAppInfo (&ai);

		BEntry entry (&ai.ref);
		if (entry.InitCheck() != B_OK)
			return true;
		BPath path;
		entry.GetPath (&path);
		path.GetParent (&path);
		path.Append ("scripts");
 
		if (firstWord == "/SOUNDPLAY")
			path.Append ("soundplay-hey");
		else
			path.Append ("cl-amp-clr");

		BString *theCmd (new BString (path.Path()));
			
		BMessage *execmsg (new BMessage);
		execmsg->AddPointer ("exec", theCmd);
		execmsg->AddPointer ("agent", this);

		thread_id execThread = spawn_thread (
			ExecPipe,
			"exec_thread",
			B_LOW_PRIORITY,
			execmsg);

		resume_thread (execThread);
		return true;
	}

	if (firstWord == "/ABOUT")
	{
		vision_app->PostMessage (B_ABOUT_REQUESTED);
		return true;
	}

	if (firstWord == "/GAWAY")
	{
		BString theReason (RestOfString (data, 2)),
						tempString;

		if (theReason == "-9z99")
			theReason = "BRB"; // Todo: make a default away msg option
		 
		BString theCmd ("/AWAY ");
		theCmd += theReason;

		BMessage cmdMsg (M_SUBMIT);
		cmdMsg.AddBool("clear", true);
		cmdMsg.AddBool("history", true);
		cmdMsg.AddString("input", theCmd.String());
		vision_app->pClientWin()->ServerBroadcast(&cmdMsg);
		return true;
	}
	
	if (firstWord == "/AWAY")
	{
		BString theReason (RestOfString (data, 2)),
						tempString;

		if (theReason == "-9z99")
			theReason = "BRB"; // Todo: make a default away msg option

		const char *expansions[1];
		expansions[0] = theReason.String();

		tempString = ExpandKeyed (vision_app->GetCommand (CMD_AWAY).String(), "R",
			expansions);
		tempString.RemoveFirst("\n");

		AddSend (&sendMsg, "AWAY");
		AddSend (&sendMsg, " :");
		AddSend (&sendMsg, theReason.String());
		AddSend (&sendMsg, endl);

		if (fId != fServerName)
			ActionMessage (tempString.String(), fMyNick.String());
		return true;
	}
	
	if (firstWord == "/GBACK")
	{
		BString theCmd ("/BACK");

		BMessage cmdMsg (M_SUBMIT);
		cmdMsg.AddBool("clear", true);
		cmdMsg.AddBool("history", true);
		cmdMsg.AddString("input", theCmd.String());
		vision_app->pClientWin()->ServerBroadcast(&cmdMsg);
		return true;
	}


	if (firstWord == "/BACK")
	{
		AddSend (&sendMsg, "AWAY");
		AddSend (&sendMsg, endl);

		if (fId != fServerName)
			ActionMessage (vision_app->GetCommand (CMD_BACK).String(), fMyNick.String());
		return true;
	}
	
	
	if (firstWord == "/CLEAR")
	{
		fText->Clear ();
		return true;
	}

	if (firstWord == "/NETCLEAR")
	{
		BLooper *looper (NULL);
		ServerAgent *currentserver (dynamic_cast<ServerAgent *>(fSMsgr.Target(&looper)));
		if (currentserver != NULL)
		{
			BMessage msg (M_SUBMIT);
			msg.AddString("input", "/clear");
			msg.AddBool("clear", true);
			msg.AddBool("history", false); 
			currentserver->Broadcast(&msg, true);
		}
		return true;
	}
 
	if (firstWord == "/ACLEAR")
	{
		BMessage msg (M_SUBMIT);
		msg.AddString("input", "/netclear");
		msg.AddBool("clear", true);
		msg.AddBool("history", true);
		vision_app->pClientWin()->ServerBroadcast(&msg);
		return true;
	}


	if (firstWord == "/GOOGLE")
	{
		BString buffer (RestOfString (data, 2));
		if (buffer != "-9z99")
		{
			BMessage lookup (M_LOOKUP_GOOGLE);
			lookup.AddString ("string", buffer);
			fMsgr.SendMessage (&lookup);
		}
		else
			vision_app->LoadURL ("http://www.google.com");
		return true;
	}
	
	if (firstWord == "/ACRONYM" || firstWord == "/ACRO")
	{
		BString buffer (RestOfString (data, 2));
		if (buffer != "-9z99")
		{
			BMessage lookup (M_LOOKUP_ACRONYM);
			lookup.AddString ("string", buffer);
			fMsgr.SendMessage (&lookup);
		}
		else
			vision_app->LoadURL ("http://www.acronymfinder.com");
		return true;
	}

	if (firstWord == "/FIND" || firstWord == "/SEARCH")
	{
		BString buffer (RestOfString(data, 2));
		if (buffer != "-9z99")
		{
			fText->FindText(buffer.String());
		}
		return true;
	}

	if (firstWord == "/CTCP")
	{
		BString theTarget (GetWord (data, 2)),
						theAction (RestOfString (data, 3));

		if (theAction != "-9z99")
		{
			theAction.ToUpper();

			if (theAction.ICompare ("PING") == 0)
			{
				time_t now (time (0));
				theAction << " " << now;
			}

			CTCPAction (theTarget, theAction);

			AddSend (&sendMsg, "PRIVMSG ");
			AddSend (&sendMsg, theTarget << " :\1" << theAction << "\1");
			AddSend (&sendMsg, endl);
		}
		else
			DisplayCommandError(this, "/ctcp");
		return true;
	}


	if (firstWord == "/DCC")
	{
		BString secondWord (GetWord (data, 2)),
						theNick (GetWord (data, 3)),
						theFile (RestOfString(data, 4));

		if (secondWord.ICompare ("SEND") == 0
		&&	theNick != "-9z99")
		{
			BMessage *msg (new BMessage (M_CHOSE_FILE));
			msg->AddString ("nick", theNick.String());
			BPath sendPath;
			if (theFile != "-9z99")
			{
				if (theFile.ByteAt (0) != '/')
				{
					find_directory (B_USER_DIRECTORY, &sendPath, false);
					sendPath.Append (theFile.String(), true);
				} 
				else
					sendPath.SetTo (theFile.String(), NULL, true);

				// the BFile is used to verify if the file exists
				// based off the documentation get_ref_for_path *should*
				// return something other than B_OK if the file doesn't exist
				// but that doesn't seem to be working correctly

				BFile sendFile (sendPath.Path(), B_READ_ONLY);

				// if the file exists, sendMsg, otherwise drop to the file panel

				if (sendFile.InitCheck() == B_OK)
				{
					sendFile.Unset();
					entry_ref ref;
					get_ref_for_path(sendPath.Path(), &ref);
					msg->AddRef("refs", &ref);
					fSMsgr.SendMessage(msg);
					return true;
				}
			}
			BFilePanel *myPanel (new BFilePanel (B_OPEN_PANEL, NULL, NULL, 0, false));

			BString myTitle (B_TRANSLATE("Sending a file to %1"));
			myTitle.ReplaceFirst("%1", theNick);

			myPanel->Window()->SetTitle (myTitle.String());
			myPanel->SetTarget (fSMsgr);
			myPanel->SetMessage (msg);

			myPanel->SetButtonLabel (B_DEFAULT_BUTTON, B_TRANSLATE("Send"));
			myPanel->Show();
		}

		if (secondWord.ICompare ("CHAT") == 0
			&&			 theNick != "-9z99")
		{
			if (theNick.ICompare(fMyNick) == 0)
				return false;
			BString thePort (GetWord (data, 4));
			BMessage msg (M_CHAT_ACTION);
			msg.AddString ("nick", theNick.String());
			if (thePort != "-9z99")
				msg.AddString ("port", thePort.String());
			fSMsgr.SendMessage (&msg);
		}
		return true;
	}

	if (firstWord == "/DOP" || firstWord == "/DEOP" || firstWord == "/DEVOICE")
	{
		BString theNick (RestOfString (data, 2));
		int32 current (2),
					last (2);
		if (theNick != "-9z99")
		{
			BString command ("MODE ");
			command += fId;
			command += " -";
			while (GetWord(data, current) != "-9z99")
			{
				AddSend (&sendMsg, command.String());
				for (; GetWord(data, current) != "-9z99" && (current - last != 4); current++)
					AddSend (&sendMsg, ((firstWord == "/DEVOICE") ? "v" : "o"));
				AddSend (&sendMsg, " ");
				for (; last < current; last++)
				{
					BString curNick (GetWord(data, last));
					if (curNick != "-9z99")
					{
						AddSend (&sendMsg, curNick);
						AddSend (&sendMsg, " ");
					}
				}
				AddSend (&sendMsg, endl);
				sendMsg.MakeEmpty();
			}
		}
		else
			DisplayCommandError(this, firstWord.ToLower());
		return true;
	}


	if (firstWord == "/DESCRIBE")
	{
		BString theTarget (GetWord (data, 2)),
						theAction (RestOfString (data, 3));

		if (theAction != "-9z99")
		{
			AddSend (&sendMsg, "PRIVMSG ");
			AddSend (&sendMsg, theTarget);
			AddSend (&sendMsg, " :\1ACTION ");
			AddSend (&sendMsg, theAction);
			AddSend (&sendMsg, "\1");
			AddSend (&sendMsg, endl);

			BString theActionMessage ("[ACTION]-> ");
			theActionMessage << theTarget << " -> " << theAction << "\n";

			Display (theActionMessage.String());
		}
		return true;
	}


	if (firstWord == "/DNS")
	{
		BString parms (GetWord(data, 2));

		ChannelAgent *channelagent;
		MessageAgent *messageagent;

		if ((channelagent = dynamic_cast<ChannelAgent *>(this)))
		{
			const NamesView *namesList (channelagent->pNamesList());
			int32 count (namesList->CountItems());

			for (int32 i = 0; i < count; ++i)
			{
				NameItem *item ((NameItem *)(namesList->ItemAt (i)));

				if (!item->Name().ICompare (parms.String(), strlen (parms.String()))) //nick
				{
					AddSend (&sendMsg, "USERHOST ");
					AddSend (&sendMsg, item->Name().String());
					AddSend (&sendMsg, endl);
					return true;
				}
			}
		}
		else if ((messageagent = dynamic_cast<MessageAgent *>(this)))
		{
			BString eid (fId);
			eid.RemoveLast (" [DCC]");
			if (!ICompare(eid, parms) || !ICompare(fMyNick, parms))
			{
				AddSend (&sendMsg, "USERHOST ");
				AddSend (&sendMsg, parms.String());
				AddSend (&sendMsg, endl);
				return true;
			}
		}

		if (parms != "-9z99")
		{
			BMessage *lookupmsg (new BMessage);
			lookupmsg->AddString ("lookup", parms.String());
			lookupmsg->AddPointer ("agent", this);

			thread_id lookupThread = spawn_thread (
				DNSLookup,
				"dns_lookup",
				B_LOW_PRIORITY,
				lookupmsg);

			resume_thread (lookupThread);
		}
		else
			DisplayCommandError(this, "/dns");
		return true;
	}
	
	if (firstWord == "/PEXEC" || firstWord == "/RRUN") // piped exec
	{
		BString *theCmd (new BString (RestOfString (data, 2)));

		if (*theCmd != "-9z99")
		{
			BMessage *msg (new BMessage);
			msg->AddPointer ("exec", theCmd);
			msg->AddPointer ("agent", this);

			thread_id execThread = spawn_thread (
				ExecPipe,
				"exec_thread",
				B_LOW_PRIORITY,
				msg);

			resume_thread (execThread);
		}
		else
		{
			DisplayCommandError(this, "/pexec");
			delete theCmd;
		}
		return true;
	}


	#if 1
	if (firstWord == "/EXCLUDE")
	{
		{
			BString second (GetWord (data, 2)),
							rest (RestOfString (data, 3));

			if (rest != "-9z99" && rest != "-9z99")
			{
				BMessage msg (M_EXCLUDE_ADD);
				msg.AddString ("second", second.String());
				msg.AddString ("cmd", rest.String());
				msg.AddString ("server", fServerName.String());
				msg.AddRect ("frame", Frame());
				vision_app->PostMessage (&msg);
			}
		}
		return true;
	}
	#endif
	

	if (firstWord == "/EXIT")
	{
		Window()->PostMessage (B_QUIT_REQUESTED);
		return true;
	}
	

	if (firstWord == "/IGNORE")
	{
		BString rest (RestOfString (data, 2));
			
		// strip trailing spaces
		int32 count (rest.Length() - 1);
		while (rest[count--] == ' ')
			rest.RemoveLast(" ");

		if (rest != "-9z99")
		{
			BMessage msg (M_IGNORE_ADD);
			msg.AddString ("cmd", rest.String());
			fSMsgr.SendMessage(&msg);
		}
		return true;
	}

	if (firstWord == "/INVITE" || firstWord == "/I")
	{
		BString theUser (GetWord (data, 2));

		if (theUser != "-9z99")
		{
			BString theChan (GetWord (data, 3));

			if (theChan == "-9z99")
				theChan = fId;

			AddSend (&sendMsg, "INVITE ");
			AddSend (&sendMsg, theUser << " " << theChan);
			AddSend (&sendMsg, endl);
		}
		else
			DisplayCommandError(this, "/invite");
		return true;
	}


	if (firstWord == "/JOIN" || firstWord == "/J")
	{
		// Bugs: Will not handle passing more than one channel key
		BString channel (GetWord (data, 2));

		if (channel != "-9z99")
		{
			if (channel[0] != '#'
			&&	channel[0] != '!'
			&&	channel[0] != '&'
			&&	channel[0] != '+')
				channel.Prepend ("#");

			AddSend (&sendMsg, "JOIN ");
			AddSend (&sendMsg, channel);

			BString key (GetWord (data, 3));
			if (key != "-9z99")
			{
				AddSend (&sendMsg, " ");
				AddSend (&sendMsg, key);
			}

			AddSend (&sendMsg, endl);
				
				
			if (key != "-9z99")
			{
				// used for keeping track of channel keys on u2 ircds
				// (not included as part of the mode reply on join)
				ServerAgent *fatherServer (vision_app->pClientWin()->GetTopServer (fAgentWinItem));
				if (fatherServer != NULL)
				{
					if (fatherServer->IRCDType() == IRCD_UNDERNET)
					{
						vision_app->pClientWin()->joinStrings.Append (",");
						vision_app->pClientWin()->joinStrings.Append (data);
					}
				}
			}
		}
		else
			DisplayCommandError(this, "/join");
		return true;
	}


	if (firstWord == "/KICK" || firstWord == "/K")
	{
		BString theNick (GetWord (data, 2));

		if (theNick != "-9z99")
		{
			BString theReason (RestOfString (data, 3));

			if (theReason == "-9z99")
			{
				// No expansions
				theReason = vision_app->GetCommand (CMD_KICK);
			}

			AddSend (&sendMsg, "KICK ");
			AddSend (&sendMsg, fId);
			AddSend (&sendMsg, " ");
			AddSend (&sendMsg, theNick);
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, theReason);
			AddSend (&sendMsg, endl);
		}
		else
			DisplayCommandError(this, "/kick");
		return true;
	}


	if (firstWord == "/LIST")
	{
		BString theArgs (RestOfString (data, 2));
		
		BMessage msg (M_LIST_COMMAND);
		msg.AddString ("cmd", theArgs);
		fSMsgr.SendMessage (&msg);
		return true;
	}


	if (firstWord == "/M")
	{
		BString theMode (RestOfString (data, 2));

		AddSend (&sendMsg, "MODE ");

		if (fId == fServerName)
			AddSend (&sendMsg, fMyNick);
		else if (fId[0] == '#' || fId[0] == '!' || fId[0] == '&' || fId[0] == '+')
			AddSend (&sendMsg, fId);
		else
			AddSend (&sendMsg, fMyNick);
 
		if (theMode != "-9z99")
		{
			AddSend (&sendMsg, " ");
			AddSend (&sendMsg, theMode);
		}

		AddSend (&sendMsg, endl);
		return true;
	}


	if (firstWord == "/ME")
	{
		BString theAction (RestOfString (data, 2));

		if (theAction != "-9z99")
			ActionMessage (theAction.String(), fMyNick.String());
		else
			DisplayCommandError(this, "/me");
		return true;
	}
	

	if (firstWord == "/MODE")
	{
		BString theMode (RestOfString (data, 3)),
						theTarget (GetWord (data, 2));

		if (theTarget != "-9z99")
		{
			AddSend (&sendMsg, "MODE ");

			if (theMode == "-9z99")
				AddSend (&sendMsg, theTarget);
			else
				AddSend (&sendMsg, theTarget << " " << theMode);

			AddSend (&sendMsg, endl);
		}
		else
			DisplayCommandError(this, "/mode");
		return true;
	}


	if (firstWord == "/MSG")
	{
		BString theRest (RestOfString (data, 3));
		BString theNick (GetWord (data, 2));

		if (theRest != "-9z99")
		{
			if (vision_app->GetBool ("queryOnMsg"))
			{
				BMessage msg (M_OPEN_MSGAGENT);
				BMessage buffer (M_SUBMIT);

				buffer.AddString ("input", theRest.String());
				msg.AddMessage ("msg", &buffer);
				msg.AddString ("nick", theNick.String());
				fSMsgr.SendMessage (&msg);
			}
			else
			{
				BString tempString;
				tempString << "[M]-> " << theNick << " > " << theRest << "\n";
				Display (tempString.String());

				AddSend (&sendMsg, "PRIVMSG ");
				AddSend (&sendMsg, theNick);
				AddSend (&sendMsg, " :");
				AddSend (&sendMsg, theRest);
				AddSend (&sendMsg, endl);
			}
		}
		return true;
	}

	if (firstWord == "/NICK")
	{
		BString newNick (GetWord (data, 2));

		if (newNick != "-9z99")
		{
			BString tempString = "*** ";
			tempString += B_TRANSLATE("Trying new nick %1.");
			tempString.ReplaceFirst("%1", newNick.String());
			tempString += "\n";
			Display (tempString.String());

			AddSend (&sendMsg, "NICK ");
			AddSend (&sendMsg, newNick);
			AddSend (&sendMsg, endl);
		}
		else
			DisplayCommandError(this, "/nick");
		return true;
	}

	if (firstWord == "/NOTICE")
	{
		BString theTarget (GetWord (data, 2)),
						theMsg (RestOfString (data, 3));

		if (theMsg != "-9z99")
		{
			AddSend (&sendMsg, "NOTICE ");
			AddSend (&sendMsg, theTarget);
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, theMsg);
			AddSend (&sendMsg, endl);

			BString tempString ("[N]-> ");
			tempString += theTarget;
			tempString += " -> ";
			tempString += theMsg;
			tempString += '\n';
			Display (tempString.String());
		}
		else
			DisplayCommandError(this, "/notice");
		return true;
	}


	if (firstWord == "/NOTIFY")
	{
		BString rest (RestOfString (data, 2));
			
		// strip trailing spaces
		int32 count (rest.Length() - 1);
		while (rest[count--] == ' ')
			rest.RemoveLast(" ");

		if (rest != "-9z99")
		{
			BMessage msg (M_NOTIFYLIST_ADD);
			msg.AddString ("cmd", rest.String());
			fSMsgr.SendMessage(&msg);
		}
		return true;
	}
	
	if (firstWord == "/OP" || firstWord == "/VOICE")
	{
		BString theNick (RestOfString (data, 2));
		int32 current (2),
					last (2);
		if (theNick != "-9z99")
		{
			BString command ("MODE ");
			command += fId;
			command += " +";
			while (GetWord(data, current) != "-9z99")
			{
				AddSend (&sendMsg, command.String());
				for (; GetWord(data, current) != "-9z99" && (current - last != 4); current++)
					AddSend (&sendMsg, ((firstWord == "/OP") ? "o" : "v"));
				AddSend (&sendMsg, " ");
				for (; last < current; last++)
				{
					BString curNick (GetWord(data, last));
					if (curNick != "-9z99")
					{
						AddSend (&sendMsg, curNick);
						AddSend (&sendMsg, " ");
					}
				}
				AddSend (&sendMsg, endl);
				sendMsg.MakeEmpty();
			}
		}
		else
			DisplayCommandError(this, firstWord.ToLower());
		return true;
	}
	
	if (firstWord == "/PART")
	{
		BMessage msg (M_CLIENT_QUIT);
		msg.AddBool ("vision:part", true);
		BString secondWord (GetWord(data, 2));
		BString partmsg;
		if (secondWord == fId)
		{
			partmsg = RestOfString(data, 3);
		}
		else
		{
			partmsg = RestOfString(data, 2);
		}
		
		if (partmsg != "-9z99")
		{
			msg.AddString("vision:partmsg", partmsg);
		}
		fMsgr.SendMessage (&msg);
		return true;
	}

	if (firstWord == "/PING")
	{
		BString theNick (GetWord (data, 2));

		if (theNick != "-9z99")
		{
			long theTime (time (0));
			BString tempString ("/CTCP ");
			tempString << theNick << " PING " << theTime;
			ParseCmd (tempString.String());
		}
		return true;
	}

	if (firstWord == "/QUERY" || firstWord == "/Q")
	{
		BString theNick (GetWord (data, 2)),
						theMsg (RestOfString (data, 3));
			
		if (theNick != "-9z99")
		{
			BMessage msg (M_OPEN_MSGAGENT);
			msg.AddString ("nick", theNick.String());

			if (theMsg != "-9z99")
			{
				BMessage buffer (M_SUBMIT);
				buffer.AddString ("input", theMsg.String());
				msg.AddMessage ("msg", &buffer);
			}
				
			fSMsgr.SendMessage (&msg);
		}
		return true;
	}

	if (firstWord == "/QUI" || firstWord == "/QUIT")
	{
		BString theRest (RestOfString (data, 2)),
						buffer;
						 
		if (theRest != "-9z99")
		{
			buffer += "QUIT :";
			buffer += theRest; 
		}

		BMessage msg (M_CLIENT_QUIT);
		msg.AddString ("vision:quit", buffer.String());
		if (fSMsgr.IsValid())
			fSMsgr.SendMessage (&msg);
		return true;
	}

	if (firstWord == "/RAW" || firstWord == "/QUOTE")
	{
		BString theRaw (RestOfString (data, 2));
			
		if (theRaw != "-9z99")
		{
			AddSend (&sendMsg, theRaw);
			AddSend (&sendMsg, endl);

			BString tempString ("[R]-> ");
			tempString << theRaw << '\n';

			Display (tempString.String());
		}
		else
			DisplayCommandError(this, "/raw");
		return true;
	}


	if (firstWord == "/RECONNECT")
	{
		fSMsgr.SendMessage (M_SLASH_RECONNECT);
		return true;
	}


	if (firstWord == "/SETBOOL")
	{
		BString var (GetWord (data, 2)),
						value (GetWord (data, 3));
		bool newvalue (false),
				 caught (false);
		value.ToLower();

		if (value != "-9z99")
		{
			if (value == "true")
			{
				caught = true;
				newvalue = true;
			}
			else if (value == "false")
			{
				caught = true;
				newvalue = false;
			}
		}

		if (!caught || value == "-9z99")
		{
			DisplayCommandError(this, "/setbool");
		}
		else
		{
			status_t returned (vision_app->SetBool (var.String(), newvalue));
			BString temp = "[x] /setbool: ";
			if (returned == B_OK)
				temp += B_TRANSLATE("Boolean has been set");
			else
				temp += B_TRANSLATE("Error setting boolean");
			temp += "\n";
			Display (temp.String(), C_ERROR);
		}
		return true;
	}
	
	if (firstWord == "/SLEEP")
	{
		BString rest (RestOfString (data, 2));

		if (rest != "-9z99")
		{
			// this basically locks up the window its called from,
			// but I can't think of a better way with our current
			// commands implementation
			int32 sleeptime = atoi(rest.String());
			snooze (sleeptime * 1000 * 100); // deciseconds? 10 = one second
		}
		return true;
	}
	
	if (firstWord == "/T")
	{
		BString theChan (fId),
						 cmd (RestOfString (data, 2));
		
		theChan += " ";
		theChan.Prepend ("/TOPIC ");
		theChan += cmd;
		ParseCmd(theChan.String());
		return true;
	}

	if (firstWord == "/TOPIC")
	{
		BString theChan (GetWord(data, 2));
		BString theTopic (RestOfString (data, 3));
		if (theChan != "-9z99")
		{
			AddSend (&sendMsg, "TOPIC ");
			AddSend (&sendMsg, theChan);
			if (theTopic != "-9z99")
			{
				AddSend (&sendMsg, " :");
				AddSend (&sendMsg, theTopic);
			}
			AddSend (&sendMsg, endl);
		}
		// TODO: print nice error message about topic parameters here
		return true;
	}

	if (firstWord == "/UNIGNORE")
	{
		BString rest (RestOfString (data, 2));
			
		// strip trailing spaces
		int32 count (rest.Length() - 1);
		while (rest[count--] == ' ')
			rest.RemoveLast(" ");

		if (rest != "-9z99")
		{
			BMessage msg (M_IGNORE_REMOVE);
			msg.AddString ("cmd", rest.String());
			fSMsgr.SendMessage(&msg);
		}
		return true;
	}


	if (firstWord == "/UNNOTIFY")
	{
		{
			BString rest (RestOfString (data, 2));
			
			// strip trailing spaces
			int32 count (rest.Length() - 1);
			while (rest[count--] == ' ')
				rest.RemoveLast(" ");

			if (rest != "-9z99")
			{
				BMessage msg (M_NOTIFYLIST_REMOVE);
				msg.AddString ("cmd", rest.String());
				fSMsgr.SendMessage (&msg);
			}
		}
		return true;
	}

	if (firstWord == "/VUPTIME")
	{
		BString parms (GetWord(data, 2)),
						clientUptime (DurationString(vision_app->VisionUptime())),
						expandedString (B_TRANSLATE("Vision has been running for %1"));
		expandedString.ReplaceFirst("%1", clientUptime);

		if ((fId != fServerName) && (parms == "-9z99"))
		{
			AddSend (&sendMsg, "PRIVMSG ");
			AddSend (&sendMsg, fId);
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, expandedString.String());
			AddSend (&sendMsg, endl);
				
			ChannelMessage (expandedString.String(), fMyNick.String());
		}
		else if ((parms == "-l") || (fId == fServerName)) // echo locally
		{
			BString tempString;
			tempString << "Vision Uptime: " << clientUptime.String() << "\n";
			Display (tempString.String(), C_WHOIS);
		}
		return true;

	}

	if (firstWord == "/UPTIME")
	{
		BString parms (GetWord(data, 2)),
						uptime (DurationString (system_time())),
						expandedString;
							
		const char *expansions[1];
		expansions[0] = uptime.String();
		expandedString = ExpandKeyed (vision_app->GetCommand (CMD_UPTIME).String(), "U",
			expansions);
		expandedString.RemoveFirst("\n");

		if ((fId != fServerName) && (parms == "-9z99"))
		{
			AddSend (&sendMsg, "PRIVMSG ");
			AddSend (&sendMsg, fId);
			AddSend (&sendMsg, " :");
			AddSend (&sendMsg, expandedString.String());
			AddSend (&sendMsg, endl);
				
			ChannelMessage (expandedString.String(), fMyNick.String());
		}
		else if ((parms == "-l") || (fId == fServerName)) // echo locally
		{
			BString tempString;
			tempString << "Uptime: " << expandedString << "\n";
			Display (tempString.String(), C_WHOIS);
		}
		return true;
	}


	if (firstWord == "/VERSION"
	||	firstWord == "/TIME")
	{
		BString theCmd (firstWord.RemoveFirst ("/")),
						theNick (GetWord (data, 2));
		theCmd.ToUpper();

		// the "." check is because the user might specify a server name

		if (theNick != "-9z99" && theNick.FindFirst(".") < 0)
		{
			BString tempString ("/CTCP ");
			tempString << theNick << " " << theCmd;
			ParseCmd (tempString.String());
		}
		else
		{
			AddSend (&sendMsg, theCmd);

			if (theNick != "-9z99")
			{
				AddSend (&sendMsg, " ");
				AddSend (&sendMsg, theNick);
			}

			AddSend (&sendMsg, endl);
		}
		return true;
	}


	if (firstWord == "/VISIT")
	{
		BString buffer (GetWord (data, 2));

		if (buffer != "-9z99")
			vision_app->LoadURL (buffer.String());
		else
			DisplayCommandError(this, "/visit");
		return true;
	}

	if (firstWord == "/WEBSTER" || firstWord =="/DICTIONARY")
	{
		BString buffer (RestOfString (data, 2));
		
		if (buffer != "-9z99")
		{
			BMessage lookup (M_LOOKUP_WEBSTER);
			lookup.AddString ("string", buffer);
			fMsgr.SendMessage (&lookup);
		}
		else
			vision_app->LoadURL ("http://www.m-w.com");
		return true;
	}

	if (firstWord != "" && firstWord[0] == '/')
	// != "" is required to prevent a nasty crash with firstWord[0]
	{
		BString theCmd (firstWord.RemoveAll ("/")),
						theRest (RestOfString (data, 2));

		if (theCmd == "W")
			theCmd = "WHOIS";

		AddSend (&sendMsg, theCmd);

		if (theRest != "-9z99")
		{
			AddSend (&sendMsg, " ");
			AddSend (&sendMsg, theRest);
		}
			
		AddSend (&sendMsg, endl);
		return true;
	}

	return false;	// we couldn't handle this message
}


int32
ClientAgent::ExecPipe (void *arg)
{
	BMessage *msg (reinterpret_cast<BMessage *>(arg));
	BString *exec;
	ClientAgent *agent;

	if ((msg->FindPointer ("exec", reinterpret_cast<void **>(&exec)) != B_OK)
	||	(msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
	{
		printf (":ERROR: couldn't find valid data in BMsg to ExecPipe() -- bailing\n");
		return B_ERROR;
	}

	// re use message
	msg->MakeEmpty();
	msg->what = M_SUBMIT;
	msg->AddString("input", "");
	msg->AddBool("clear", false);
	msg->AddBool("add2history", false);
	BMessenger self_destruct_in_15_seconds(agent);

	FILE *fp = popen (exec->String(), "r");
	
	if(fp == NULL)
	{
		msg->what = M_DISPLAY;
		BString temp = "[x] /pexec: ";
		temp += B_TRANSLATE("command failed");
		temp += "\n";
		PackDisplay(msg, temp.String(), C_ERROR);
		self_destruct_in_15_seconds.SendMessage(msg);
	}
	else
	{
		char data[768]; // should be long enough for any line...
	
		// read one less just in case we need to offset by a char (prepended '/')
		while (fgets(data, 767, fp))
		{
			data[strlen(data)-1] = '\0'; // strip termination
			if (data[0] == '/')
			{
				memmove(data + 1, data, strlen(data));
				data[0] = ' ';			
			}
			
			// ship off to agent
			msg->ReplaceString("input", data);
			self_destruct_in_15_seconds.SendMessage(msg);
		}

		pclose(fp);
	}	

	delete exec;
	delete msg;

	return B_OK;
}

int32
ClientAgent::DNSLookup (void *arg)
{

	BMessage *msg (reinterpret_cast<BMessage *>(arg));
	const char *lookup;
	ClientAgent *agent;

	if ((msg->FindString ("lookup", &lookup) != B_OK)
	||	(msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
	{
		printf (":ERROR: couldn't find valid data in BMsg to DNSLookup() -- bailing\n");
		return B_ERROR;
	} 

	BString resolve (lookup),
					output ("[x] ");
					
	
	struct addrinfo *info;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	bool reverse = false;

	struct in_addr addr;
	if (resolve.FindFirst(":") != B_ERROR || inet_aton(resolve.String(), &addr) == 1)
	{
		reverse = true;
	}
	
	int result = getaddrinfo(resolve.String(), NULL, &hints, &info);
	char addr_buf[1024];
	memset(addr_buf, 0, sizeof(addr_buf));

	if (result == 0)
	{
		result = getnameinfo(info->ai_addr, info->ai_addrlen, addr_buf,
				sizeof(addr_buf), NULL, 0, reverse ? NI_NAMEREQD : NI_NUMERICHOST);
		freeaddrinfo(info);
	}
	
	if (result == 0)
	{
		output += B_TRANSLATE("Resolved %1 to %2");
		output.ReplaceFirst("%1", resolve.String());
		output.ReplaceFirst("%2", addr_buf);
	}
	else
	{
		output += B_TRANSLATE("Unable to resolve %1 (%2)");
		output.ReplaceFirst("%1", resolve.String());
		output.ReplaceFirst("%2", gai_strerror(result));
	}
	output += "\n";

	delete msg;

	BMessage dnsMsg (M_DISPLAY);
	agent->PackDisplay (&dnsMsg, output.String(), C_WHOIS);
	agent->fMsgr.SendMessage (&dnsMsg);

	return B_OK;
}
