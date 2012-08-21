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
 */

#include <AppFileInfo.h>
#include <Catalog.h>
#include <sys/utsname.h>
#include <stdlib.h>

#include "Vision.h"
#include "Utilities.h"
#include "ServerAgent.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerMessages"

void
ServerAgent::ParseCTCP (BString theNick, BString theTarget, BString theMsg)
{
	BString theCTCP (GetWord(theMsg.String(), 1).ToUpper()),
					theRest (RestOfString(theMsg.String(), 2));
	theCTCP.RemoveFirst ("\1");
	theCTCP.RemoveLast ("\1");
	
	if (theCTCP == "PING")
	{
		if (theMsg == "-9z99")
			return;
		BString tempString("NOTICE ");
		tempString += theNick;
		tempString += " :";
		tempString += theMsg;
		SendData (tempString.String());
	}
	
	else if ((theCTCP == "VERSION") || (theCTCP == "CLIENTINFO"))
	{
		BString sysInfoString;
		if (!vision_app->GetBool ("versionParanoid"))
		{
#if 0
			BString librootversion;
			BFile *libroot (new BFile ("/boot/beos/system/lib/libroot.so", B_READ_ONLY));
			BAppFileInfo info (libroot);
			version_info version;
			info.GetVersionInfo (&version, B_SYSTEM_VERSION_KIND);
			librootversion = version.short_info;
			librootversion.RemoveFirst ("R");
			
			delete libroot;			
#endif			
			system_info myInfo;
			get_system_info (&myInfo);

			utsname sysname;
			uname(&sysname);
			
			BString revInfo = sysname.version;
			revInfo.Remove(revInfo.FindFirst(' '), revInfo.Length());
	
			sysInfoString = " : ";
			sysInfoString += sysname.sysname;
			sysInfoString += "/";
			sysInfoString += revInfo;
#if 0			
			if ((librootversion.FindFirst("5.0") == 0) || (librootversion == "5"))
			{
				// this is the way the BeOS 5.0.1 update checks for R5 Pro...
				bool BePro;
				BePro = true; // innocent until proven guilty
				BFile *indeo5rt (new BFile ("/boot/beos/system/add-ons/media/encoders/indeo5rt.encoder", B_READ_ONLY));
				BFile *indeo5rtmmx (new BFile ("/boot/beos/system/add-ons/media/encoders/indeo5rtmmx.encoder", B_READ_ONLY));
				BFile *mp3 (new BFile ("/boot/beos/system/add-ons/media/encoders/mp3.encoder", B_READ_ONLY));
				
				if ((indeo5rt->InitCheck() != B_OK)
				||	(indeo5rtmmx->InitCheck() != B_OK)
				||	(mp3->InitCheck() != B_OK))
					BePro = false; // *gasp*! leeches!

				delete indeo5rt;
				delete indeo5rtmmx;
				delete mp3;
				
				if (BePro)
					sysInfoString += " Pro Edition";
				else
					sysInfoString += " Personal Ed.";

			}
#endif			
			sysInfoString += " (";
		sysInfoString << myInfo.cpu_count;
			sysInfoString += "x";
			sysInfoString << myInfo.cpu_clock_speed / 1000000;
			sysInfoString += "MHz) : ";
		}
		else
			sysInfoString = " : A bird in the bush usually has a friend in there with him : ";
		
		BString tempString ("NOTICE ");
		BString tempString2;
		vision_app->VisionVersion (VERSION_VERSION, tempString2);
		tempString += theNick;
		tempString += " :\1VERSION Vision-"; 
		tempString += tempString2;
		tempString += sysInfoString;
		tempString += "http://vision.sourceforge.net";


		tempString += '\1';
		SendData (tempString.String());
	}


	else if (theCTCP == "UPTIME")
	{
		BString uptime (DurationString(system_time()));
		BString expandedString;

		const char *expansions[1];
		expansions[0] = uptime.String();

		expandedString = ExpandKeyed (vision_app->GetCommand (CMD_UPTIME).String(), "U",
		expansions);
		expandedString.RemoveFirst ("\n");

		BString tempString ("NOTICE ");
		tempString += theNick;
		tempString += " :\1UPTIME ";
		tempString += expandedString;
		tempString += '\1';
		SendData (tempString.String());
	}
	
	else if ((theCTCP == "TIME") || (theCTCP == "DATE"))
	{
		time_t st (time (0));
		struct tm curTime;
		localtime_r (&st, &curTime); 
		char str[47];
		strftime (str,47,"%A %b %d %Y %I:%M %p %Z",&curTime);

		BString tempString ("NOTICE ");
		tempString += theNick;
		tempString += " :\1TIME "; 
		tempString += str;
		tempString += '\1';
		SendData (tempString.String());
	}

	else if(theCTCP == "DCC")
	{
		BString theType = GetWord(theMsg.String(), 2);

		if (theType == "SEND")
		{
			BString theFile,
							theIP,
							thePort,
							theSize;
			int32 startPos (0), endPos (0);
			if ((startPos = theMsg.FindFirst ('\"')) != B_ERROR)
			{
				endPos = theMsg.FindFirst ('\"', startPos + 1);
				theMsg.CopyInto (theFile, startPos + 1, endPos - (startPos + 1));
				BString rest;
				theMsg.CopyInto (rest, endPos+1, theMsg.Length() - (endPos + 1));
				theIP = GetWord (rest.String(), 2);
				thePort = GetWord (rest.String(), 3);
				theSize = GetWord (rest.String(), 4);
			}
			else
			{
				theFile = GetWord(theMsg.String(), 3);
				theIP	 = GetWord(theMsg.String(), 4);
				thePort = GetWord(theMsg.String(), 5);
				theSize = GetWord(theMsg.String(), 6);
			}
			theSize.RemoveLast ("\1"); // strip CTCP char
			DCCGetDialog (theNick, theFile, theSize, theIP, thePort);
		}
		if (theType == "CHAT")
		{
					BString theIP (GetWord(theMsg.String(), 4)),
									thePort (GetWord(theMsg.String(), 5));
			thePort.RemoveLast ("\1");
			DCCChatDialog(theNick, theIP, thePort);
		}
		else if (theType == "ACCEPT")
		{
			BString file (GetWord (theMsg.String(), 3)),
							port (GetWord (theMsg.String(), 4)),
							poss (GetWord (theMsg.String(), 5));
			poss.RemoveLast("\1");

			off_t pos ((int32)0L);
			int32 i (0);
			for (i = 0; i < poss.Length(); ++i)
				pos = pos * 10 + poss[i] - '0';

			for (i = 0; i < fResumes.CountItems(); ++i)
			{
				ResumeData *data ((ResumeData *)fResumes.ItemAt (i));

				if (data->nick == theNick
				&&	data->pos	== pos
				&&	data->port == port)
				{
					fResumes.RemoveItem (i);

					BMessage msg (M_DCC_ACCEPT);
					msg.AddString ("vision:nick", data->nick.String());
					msg.AddString ("vision:file", data->file.String());
					msg.AddString ("vision:size", data->size.String());
					msg.AddString ("vision:ip",	 data->ip.String());
					msg.AddString ("vision:port", data->port.String());
					msg.AddString ("path",				data->path.String());
					msg.AddBool	 ("continue",		true);

					fMsgr.SendMessage(&msg);
					
					delete data;
					break;
				}
			}
		}
		else if (theType == "RESUME")
		{
			BString file (GetWord (theMsg.String(), 3)),
							port (GetWord (theMsg.String(), 4)),
							poss (GetWord (theMsg.String(), 5));
			poss.RemoveLast("\1");
			off_t pos ((int32)0L);

			for (int32 i = 0; i < poss.Length(); ++i)
				pos = pos * 10 + poss[i] - '0';
				

			// Have to tell the sender we can resume
			BString tempString("PRIVMSG ");
			tempString += theNick;
			tempString += " :\1DCC ACCEPT ";
			tempString += file;
			tempString += " ";
			tempString += port;
			tempString += " ";
			tempString += poss;
			tempString += "\1";
			SendData (tempString.String());
			

			BMessage bMsg (M_DCC_MESSENGER), bReply;
			be_app_messenger.SendMessage (&bMsg, &bReply);
			
			BMessenger msgr;
			bReply.FindMessenger ("msgr", &msgr);
			
			BMessage msg (M_ADD_RESUME_DATA), reply;
			msg.AddString ("vision:nick", theNick.String());
			msg.AddString ("vision:port", port.String());
			msg.AddString ("vision:file", file.String());
			msg.AddInt64 ("vision:pos", pos);
			
			// do sync.. we do not want to have the transfer
			// start before we tell it okay
			msgr.SendMessage (&msg, &reply);
			if (reply.HasBool ("hit")
			&&	reply.FindBool ("hit"))
			{
				BString buffer;
				buffer += "PRIVMSG ";
				buffer += theNick;
				buffer += " :\1DCC ACCEPT ";
				buffer += file;
				buffer += " ";
				buffer += port;
				buffer += " ";
				buffer += poss;
				buffer += "\1";
				SendData (buffer.String());
			}
		}
	}

	BMessage display (M_DISPLAY);
	BString buffer;

	buffer += "[";
	buffer += theNick;
	buffer += " ";
	if (theTarget != fMyNick)
	{
		buffer += theTarget;
		buffer += ":";
	}
	buffer << theCTCP;

	if (theCTCP == "PING" || theRest == "-9z99")
		buffer += "]\n";
	else
	{
		int32 theChars = theRest.Length();
		if (theRest[theChars - 1] == '\1')
			theRest.Truncate (theChars - 1);
			
		buffer += "] ";
		buffer += theRest;
		buffer += '\n';
	}
		
	PackDisplay (&display, buffer.String(), C_CTCP_REQ, C_BACKGROUND, F_SERVER);
	PostActive (&display);
}

void
ServerAgent::ParseCTCPResponse (BString theNick, BString theMsg)
{
	BString theResponse (theMsg);
	if (theResponse[0] == '\1')
		theResponse.Remove(0, 1);
	int32 theChars = theResponse.Length();
	if(theResponse[theChars - 1] == '\1')
		theResponse.Truncate (theChars - 1);

	BString firstWord = GetWord (theResponse.String(), 1).ToUpper();
	BString tempString;

	if (firstWord == "PING")
	{
		long curTime = time (NULL);
		long theSeconds = curTime - atoi (GetWord (theMsg.String(), 2).String());

		if (theSeconds > 10000) // catch possible conversion error(s)
		{
			theSeconds = curTime - atoi (GetWord (theMsg.String(), 2).String());
			if (theSeconds > 10000)
			{
				theSeconds = curTime - atoi (GetWord (theMsg.String(), 2).String());
				if (theSeconds > 10000)
				{
					theSeconds = curTime - atoi (GetWord (theMsg.String(), 2).String());
				}
			}
		}
		tempString += "[";
		tempString += theNick;
		tempString += " PING ";
		tempString += B_TRANSLATE("reponse");
		tempString += "]: ";
		if (theSeconds != 1)
		{
			tempString << theSeconds << " ";
			tempString += B_TRANSLATE("seconds");
			tempString += "\n";
		}
		else
		{
			tempString += "1 ";
			tempString += B_TRANSLATE("second");
			tempString += "\n";
		}
	}
	else
	{
		BString theReply = RestOfString (theResponse.String(), 2);
		tempString += "[";
		tempString += theNick;
		tempString += " ";
		tempString += firstWord;
		tempString += " ";
		tempString += B_TRANSLATE("response");
		tempString += "]: ";
		tempString += theReply;
		tempString += '\n';
	}
	
	BMessage display (M_DISPLAY);
	BString buffer;

	buffer << tempString.String();
	PackDisplay (&display, buffer.String(), C_CTCP_RPY, C_BACKGROUND, F_SERVER);
	PostActive (&display);
}
