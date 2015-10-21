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
 *                 Bjorn Oksholen
 */

#include <Entry.h>
#include <Menu.h>
#include <Notification.h>
#include <Roster.h>

#include "ClientWindow.h"
#include "NotifyList.h"
#include "ParseENums.h"
#include "ServerAgent.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Vision.h"
#include "WindowList.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

bool ServerAgent::ParseENums(const char* data, const char* sWord)
{
	int num(atoi(sWord));

	switch (num) {
	case ZERO: // 0
	{
		// wasn't a numeric, or the server is playing tricks on us
	}
		return false;

	case ERR_UNKNOWNCOMMAND: // 421
	{
		BString tempString(RestOfString(data, 4)), badCmd(GetWord(data, 4));

		if (badCmd == "VISION_LAG_CHECK") {
			int32 difference(system_time() - fLagCheck);
			if (difference > 0) {
				int32 secs(difference / 1000000);
				int32 milli(difference / 1000 - secs * 1000);
				char lag[15] = "";
				sprintf(lag, "%ld.%03ld", secs, milli);
				fMyLag = lag;
				fLagCount = 0;
				fCheckingLag = false;
				fMsgr.SendMessage(M_LAG_CHANGED);
			}
		} else {
			tempString.RemoveFirst(":");
			tempString.Append("\n");
			Display(tempString.String());
		}
	}
		return true;

	case RPL_WELCOME:  // 001
	case RPL_YOURHOST: // 002
	case RPL_CREATED:  // 003
	case RPL_MYINFO:   // 004
	{
		fIsConnecting = false;
		fIsConnected = true;
		fSMsgr.SendMessage(M_CONNECTED);
		fInitialMotd = true;
		fRetry = 0;

		if (fNetworkData.FindBool("lagCheck")) {
			fMyLag = "0.000";
			fMsgr.SendMessage(M_LAG_CHANGED);
		}
		BString theNick(GetWord(data, 3));
		fMyNick = theNick;

		if (!IsHidden())
			vision_app->pClientWin()->pStatusView()->SetItemValue(STATUS_NICK, theNick.String());

		BString theMsg(RestOfString(data, 4));
		theMsg.RemoveFirst(":");
		theMsg.Prepend("* ");
		theMsg.Append("\n");
		Display(theMsg.String());

		if (num == RPL_MYINFO) {
			// set "real" hostname
			fServerHostName = (GetWord(data, 1));
			fServerHostName.RemoveFirst(":");
			BString hostName(fId.String());
			hostName += " - [";
			hostName += fServerHostName.String();
			hostName += "]";
			fAgentWinItem->SetName(hostName.String());

			// detect IRCd
			fIrcdtype = IRCD_STANDARD;

			if (theMsg.FindFirst("hybrid") > 0) fIrcdtype = IRCD_HYBRID;
			// ultimate and unreal share the same numerics, so treat them with the same
			// identifier for now
			else if ((theMsg.FindFirst("UltimateIRCd") > 0) || (theMsg.FindFirst("Unreal") > 0))
				fIrcdtype = IRCD_ULTIMATE;
			else if (theMsg.FindFirst("comstud") > 0)
				fIrcdtype = IRCD_COMSTUD;
			else if (theMsg.FindFirst("u2.") > 0)
				fIrcdtype = IRCD_UNDERNET;
			else if (theMsg.FindFirst("PTlink") > 0)
				fIrcdtype = IRCD_PTLINK;
			else if (theMsg.FindFirst("CR") > 0)
				fIrcdtype = IRCD_CONFERENCEROOM;
			else if (theMsg.FindFirst("nn-") > 0)
				fIrcdtype = IRCD_NEWNET;
		}
	}
		return true;

	case RPL_PROTOCTL: // 005
	{
		// this numeric also serves as RPL_NNMAP on Newnet

		BString theMsg(RestOfString(data, 4));
		theMsg.RemoveFirst(":");
		theMsg.Append("\n");

		switch (fIrcdtype) {
		case IRCD_NEWNET: {
			// RPL_NNMAP
			Display(theMsg.String());
		} break;

		default: {
			// RPL_PROTOCTL
			theMsg.Prepend("* ");
			Display(theMsg.String());
		}
		}
	}
		return true;

	case RPL_LUSERHIGHESTCONN: // 250
	case RPL_LUSERCLIENT:	  // 251
	case RPL_LUSEROP:		   // 252
	case RPL_LUSERUNKNOWN:	 // 253
	case RPL_LUSERCHANNELS:	// 254
	case RPL_LUSERME:		   // 255
	case RPL_LUSERLOCAL:	   // 265
	case RPL_LUSERGLOBAL:	  // 266
	{
		BString theMsg(RestOfString(data, 4));
		theMsg.RemoveFirst(":");
		theMsg.Prepend("* ");
		theMsg.Append("\n");
		Display(theMsg.String());
	}
		return true;

	/// strip and send to server agent  ///
	case RPL_ULMAP:				  // 006
	case RPL_ULMAPEND:			  // 007
	case RPL_U2MAP:				  // 015
	case RPL_U2MAPEND:			  // 017
	case RPL_TRACELINK:			  // 200
	case RPL_TRACECONNECTING:	 // 201
	case RPL_TRACEHANDSHAKE:	  // 202
	case RPL_TRACEUNKNOWN:		  // 203
	case RPL_TRACEOPERATOR:		  // 204
	case RPL_TRACEUSER:			  // 205
	case RPL_TRACESERVER:		  // 206
	case RPL_TRACENEWTYPE:		  // 208
	case RPL_TRACECLASS:		  // 209
	case RPL_STATSLINKINFO:		  // 211
	case RPL_STATSCOMMANDS:		  // 212
	case RPL_STATSCLINE:		  // 213
	case RPL_STATSNLINE:		  // 214
	case RPL_STATSILINE:		  // 215
	case RPL_STATSKLINE:		  // 216
	case RPL_STATSQLINE:		  // 217
	case RPL_STATSYLINE:		  // 218
	case RPL_ENDOFSTATS:		  // 219
	case RPL_STATSBLINE:		  // 220
	case RPL_DALSTATSE:			  // 223
	case RPL_DALSTATSF:			  // 224
	case RPL_DALSTATSZ:			  // 225
	case RPL_DALSTATSN:			  // 226
	case RPL_DALSTATSG:			  // 227
	case RPL_STATSLLINE:		  // 241
	case RPL_STATSUPTIME:		  // 242
	case RPL_STATSOLINE:		  // 243
	case RPL_STATSHLINE:		  // 244
	case RPL_STATSSLINE:		  // 245
	case RPL_DALSTATSX:			  // 246
	case RPL_STATSXLINE:		  // 247
	case RPL_STATSPLINE:		  // 249
	case RPL_ADMINME:			  // 256
	case RPL_ADMINLOC1:			  // 257
	case RPL_ADMINLOC2:			  // 258
	case RPL_ADMINEMAIL:		  // 259
	case RPL_TRACELOG:			  // 261
	case RPL_ENDOFTRACE:		  // 262
	case RPL_SILELIST:			  // 271
	case RPL_ENDOFSILELIST:		  // 272
	case RPL_ENDOFWHO:			  // 315
	case RPL_CHANSERVURL:		  // 328
	case RPL_COMMANDSYNTAX:		  // 334
	case RPL_VERSION:			  // 351
	case RPL_WHOREPLY:			  // 352
	case RPL_BANLIST:			  // 367
	case RPL_ENDOFBANLIST:		  // 368
	case RPL_INFO:				  // 371
	case RPL_ENDOFINFO:			  // 374
	case RPL_YOUREOPER:			  // 381
	case RPL_REHASHING:			  // 382
	case RPL_TIME:				  // 391
	case ERR_NOORIGIN:			  // 409
	case ERR_NOTEXTTOSEND:		  // 412
	case ERR_TOOMANYAWAY:		  // 429
	case ERR_NICKCHANGETOOFAST:   // 438
	case ERR_TARGETCHANGETOOFAST: // 439
	case ERR_SUMMONDISABLED:	  // 445
	case ERR_USERSDISABLED:		  // 446
	case ERR_NOTREGISTERED:		  // 451
	case ERR_NEEDMOREPARMS:		  // 461
	case ERR_PASSWDMISMATCH:	  // 464
	case ERR_YOUREBANNEDCREEP:	// 465
	case ERR_NOPRIVILEGES:		  // 481
	case ERR_NOOPERHOST:		  // 491
	case ERR_USERSDONTMATCH:	  // 502
	case ERR_SILELISTFULL:		  // 511
	case ERR_TOOMANYWATCH:		  // 512
	case ERR_TOOMANYDCC:		  // 514
	case ERR_CANTINVITE:		  // 518
	case ERR_LISTSYNTAX:		  // 521
	case ERR_WHOSYNTAX:			  // 522
	case ERR_WHOLIMEXCEED:		  // 523
	case RPL_LOGON:				  // 600
	case RPL_LOGOFF:			  // 601
	case RPL_WATCHOFF:			  // 602
	case RPL_WATCHSTAT:			  // 603
	case RPL_NOWON:				  // 604
	case RPL_NOWOFF:			  // 605
	case RPL_WATCHLIST:			  // 606
	case RPL_ENDOFWATCHLIST:	  // 607
	case RPL_DCCALLOWLIST:		  // 618
	case RPL_DCCALLOWEND:		  // 619
	case RPL_DCCALLOW:			  // 620
	{
		BString tempString(RestOfString(data, 4));
		tempString.RemoveFirst(":");
		tempString.Append("\n");
		Display(tempString.String());
	}
		return true;

	case RPL_UMODEIS: // 221
	{
		BString theMode(GetWord(data, 4)), tempString(S_PENUM_CURMODE);
		tempString += theMode;
		tempString += '\n';

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_WHOIS);
		PostActive(&msg);
	}
		return true;

	/// strip and send to active agent  ///
	case RPL_TRYAGAIN:		   // 263
	case RPL_UNAWAY:		   // 305
	case RPL_NOWAWAY:		   // 306
	case ERR_NOSUCHNICK:	   // 401
	case ERR_NOSUCHSERVER:	 // 402
	case ERR_NOSUCHCHANNEL:	// 403
	case ERR_CANNOTSENDTOCHAN: // 404
	case ERR_TOOMANYCHANNELS:  // 405
	case ERR_WASNOSUCHNICK:	// 406
	case ERR_TOOMANYTARGETS:   // 407
	case ERR_NOCOLORSONCHAN:   // 408
	case ERR_YOUCANTDOTHAT:	// 460
	case ERR_CHANOPRIVSNEEDED: // 482
	{
		BString tempString("[x] ");
		if (num == ERR_CHANOPRIVSNEEDED)
			tempString += RestOfString(data, 5);
		else
			tempString += RestOfString(data, 4);
		tempString.RemoveFirst(":");
		tempString.Append("\n");

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
	}
		return true;

	case RPL_AWAY: // 301
	{
		BString theNick(GetWord(data, 4));
		BString tempString("[x] "), theReason(RestOfString(data, 5));
		theReason.RemoveFirst(":");
		tempString += "Away: ";
		tempString += theReason;
		tempString += '\n';

		if (fRemoteAwayMessages.find(theNick) != fRemoteAwayMessages.end()) {
			if (fRemoteAwayMessages[theNick] == theReason) {
				return true;
			}
		}
		fRemoteAwayMessages[theNick] = theReason;
		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
	}
		return true;

	case RPL_USERHOST: // 302
	{
		BString theHost(GetWord(data, 4)), theHostname(GetAddress(theHost.String()));
		theHost.RemoveFirst(":");

		BString tempString(RestOfString(data, 4));
		tempString.RemoveFirst(":");
		tempString.Append("\n");
		Display(tempString.String());

		if (fGetLocalIP && (tempString.IFindFirst(fMyNick.String()) == 0)) {
			fGetLocalIP = false;
			hostent* hp = gethostbyname(theHostname.String());
			if (hp != NULL) {
				char addr_buf[16];
				in_addr* addr = (in_addr*)hp->h_addr_list[0];
				strcpy(addr_buf, inet_ntoa(*addr));
				fLocalip = addr_buf;
#if 0
            printf("hostname found: %s\n", fLocalip.String());
#endif
				return true;
			} else if (isdigit(theHostname[0])) {
				fLocalip = theHostname;
#if 0
              printf("hostname found: %s\n", fLocalip.String());
#endif
				return true;
			}
#if 0
          else
          {
              printf("lookup failed, unable to resolve host\n");
          }
#endif
		}

		if (theHost != "-9z99" && theHost != "") {
			BMessage* dnsmsg(new BMessage);
			dnsmsg->AddString("lookup", theHostname.String());
			ClientAgent* client(ActiveClient());

			if (client)
				dnsmsg->AddPointer("agent", client);
			else
				dnsmsg->AddPointer("agent", this);

			thread_id lookupThread = spawn_thread(DNSLookup, "dns_lookup", B_LOW_PRIORITY, dnsmsg);

			resume_thread(lookupThread);
		}
	}
		return true;

	case RPL_ISON: // 303
	{
		BString nicks(RestOfString(data, 4));
		BString onlined, offlined;

		nicks.RemoveFirst(":");

		int hasChanged(0);

		BMessage msg(M_NOTIFYLIST_UPDATE);

		for (int32 i = 0; i < fNotifyNicks.CountItems(); i++) {
			NotifyListItem* item(((NotifyListItem*)fNotifyNicks.ItemAt(i)));

			int32 nickidx(nicks.IFindFirst(item->Text()));

			// make sure that the nick isn't a partial match.
			if ((nickidx >= 0) && ((nicks[nickidx + strlen(item->Text())] == ' ') ||
								   (nicks[nickidx + strlen(item->Text())] == '\0'))) {
				if (item->GetState() != true) {
					item->SetState(true);
					hasChanged = 1;

					if (onlined.Length()) onlined << ", ";
					onlined << item->Text();
				}
			} else {
				if (item->GetState() == true) {
					item->SetState(false);
					hasChanged = 2;

					if (offlined.Length()) offlined << ", ";
					offlined << item->Text();
				}
			}
		}

		if (offlined.Length()) {
			BNotification notification(B_INFORMATION_NOTIFICATION);
			notification.SetGroup(BString("Vision"));
			entry_ref ref = vision_app->AppRef();
			notification.SetOnClickFile(&ref);
			notification.SetTitle(fServerName.String());
			BString content;
			content << offlined;
			if (offlined.FindFirst(' ') > -1)
				content << " are offline";
			else
				content << " is offline";

			notification.SetContent(content);
			notification.Send();
		}

		if (onlined.Length()) {
			BNotification notification(B_INFORMATION_NOTIFICATION);
			notification.SetGroup(BString("Vision"));
			entry_ref ref = vision_app->AppRef();
			notification.SetOnClickFile(&ref);
			notification.SetTitle(fServerName.String());
			BString content;
			content << onlined;
			if (onlined.FindFirst(' ') > -1)
				content << " are online";
			else
				content << " is online";

			notification.SetContent(content);
			notification.Send();
		}

		fNotifyNicks.SortItems(SortNotifyItems);
		msg.AddPointer("list", &fNotifyNicks);
		msg.AddPointer("source", this);
		msg.AddInt32("change", hasChanged);
		Window()->PostMessage(&msg);
	}
		return true;

	case RPL_WHOISIDENTIFIED: // 307
	{
		BString theInfo(RestOfString(data, 5));
		theInfo.RemoveFirst(":");

		if (theInfo == "-9z99") {
			// USERIP reply? (RPL_U2USERIP)
			BString tempString(RestOfString(data, 4));
			tempString.RemoveFirst(":");
			tempString.Append("\n");
			Display(tempString.String());
			return true;
		}

		BMessage display(M_DISPLAY);
		BString buffer;

		buffer += "[x] ";
		buffer += theInfo;
		buffer += "\n";
		PackDisplay(&display, buffer.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&display);
	}
		return true;

	case RPL_WHOISADMIN:		 // 308
	case RPL_WHOISSERVICESADMIN: // 309
	case RPL_WHOISHELPOP:		 // 310
	case RPL_WHOISOPERATOR:		 // 313
	case RPL_WHOISREGNICK:		 // 320
	case RPL_WHOISACTUALLY:		 // 338
	case RPL_WHOISMASK:			 // 550
	case RPL_WHOWASIP:			 // 612
	case RPL_WHOISUSERMODESALT:  // 614
	case RPL_WHOISUSERMODES:	 // 615
	case RPL_WHOISREALHOSTNAME:  // 616
	{
		BString theInfo(RestOfString(data, 5));
		theInfo.RemoveFirst(":");

		BMessage display(M_DISPLAY);
		BString buffer;

		buffer += "[x] ";
		buffer += theInfo;
		buffer += "\n";
		PackDisplay(&display, buffer.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&display);
	}
		return true;

	case RPL_WHOISUSER: // 311
	{
		BString theNick(GetWord(data, 4)), theIdent(GetWord(data, 5)), theAddress(GetWord(data, 6)),
			theName(RestOfString(data, 8));
		theName.RemoveFirst(":");

		BMessage display(M_DISPLAY);
		BString buffer;

		buffer += "[x] ";
		buffer += theNick;
		buffer += " (";
		buffer += theIdent;
		buffer += "@";
		buffer += theAddress;
		buffer += ")\n";
		buffer += "[x] ";
		buffer += theName;
		buffer += "\n";

		PackDisplay(&display, buffer.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&display);
	}
		return true;

	case RPL_WHOISSERVER: // 312
	{
		BString theNick(GetWord(data, 4)), theServer(GetWord(data, 5)),
			theInfo(RestOfString(data, 6));
		theInfo.RemoveFirst(":");

		BMessage display(M_DISPLAY);
		BString buffer;

		buffer += "[x] Server: ";
		buffer += theServer;
		buffer += " (";
		buffer += theInfo;
		buffer += ")\n";
		PackDisplay(&display, buffer.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&display);
	}
		return true;

	case RPL_WHOWASUSER: // 314
	{
		BString theNick(GetWord(data, 4)), theIdent(GetWord(data, 5)), theAddress(GetWord(data, 6)),
			theName(RestOfString(data, 8)), tempString("[x] ");
		theName.RemoveFirst(":");
		tempString += theNick;
		tempString += " " S_PENUM_WHOWAS " (";
		tempString += theIdent;
		tempString += "@";
		tempString += theAddress;
		tempString += ")\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
	}
		return true;

	case RPL_WHOISIDLE: // 317
	{
		BString theNick(GetWord(data, 4)), tempString("[x] "), tempString2("[x] "),
			theTime(GetWord(data, 5)), signOnTime(GetWord(data, 6));

		int64 idleTime(strtoul(theTime.String(), NULL, 0));
		tempString += S_PENUM_IDLE;
		tempString += DurationString(idleTime * 1000 * 1000);
		tempString += "\n";

		int32 serverTime = strtoul(signOnTime.String(), NULL, 0);
		struct tm ptr;
		time_t st;
		char str[80];
		st = serverTime;
		localtime_r(&st, &ptr);
		strftime(str, 80, "%A %b %d %Y %I:%M %p %Z", &ptr);
		BString signOnTimeParsed(str);
		signOnTimeParsed.RemoveAll("\n");

		tempString2 += S_PENUM_SIGNON;
		tempString2 += signOnTimeParsed;
		tempString2 += "\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
		PackDisplay(&msg, tempString2.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
	}
		return true;

	case RPL_ENDOFWHOIS:  // 318
	case RPL_ENDOFNAMES:  // 366
	case RPL_ENDOFWHOWAS: // 369
	{
		// nothing
	}
		return true;

	case RPL_WHOISCHANNELS: // 319
	{
		BString theChannels(RestOfString(data, 5));
		theChannels.RemoveFirst(":");

		BMessage display(M_DISPLAY);
		BString buffer;

		buffer += S_PENUM_WHOIS_CHANNELS;
		buffer += theChannels;
		buffer += "\n";
		PackDisplay(&display, buffer.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
		PostActive(&display);
	}
		return true;

	case RPL_LISTSTART: // 321
	{
		BMessage msg(M_LIST_BEGIN);
		if (fListAgent) vision_app->pClientWin()->DispatchMessage(&msg, (BView*)fListAgent);
	}
		return true;

	case RPL_LIST: // 322
	{
		BMessage msg(M_LIST_EVENT);
		BString channel(GetWord(data, 4)), users(GetWord(data, 5)), topic(RestOfString(data, 6));
		topic.RemoveFirst(":");

		msg.AddString("channel", channel.String());
		msg.AddString("users", users.String());
		msg.AddString("topic", topic.String());

		if (fListAgent) vision_app->pClientWin()->DispatchMessage(&msg, (BView*)fListAgent);
	}
		return true;

	case RPL_LISTEND: // 323
	{
		BMessage msg(M_LIST_DONE);

		if (fListAgent) vision_app->pClientWin()->DispatchMessage(&msg, (BView*)fListAgent);
	}
		return true;

	case RPL_CHANNELMODEIS: // 324
	{
		BString theChan(GetWord(data, 4)), theMode(GetWord(data, 5)),
			tempStuff(RestOfString(data, 6));

		if (tempStuff != "-9z99") {
			theMode.Append(" ");
			theMode.Append(tempStuff); // avoid extra space w/o params
		}

		ClientAgent* aClient(ActiveClient()), *theClient(Client(theChan.String()));

		BString tempString(S_PENUM_CHANMODE);
		tempString += theChan;
		tempString += ": ";
		tempString += theMode;
		tempString += '\n';

		BMessage msg(M_CHANNEL_MODES);

		msg.AddString("msgz", tempString.String());
		msg.AddString("chan", theChan.String());
		msg.AddString("mode", theMode.String());

		if (theClient)
			theClient->fMsgr.SendMessage(&msg);
		else if (aClient)
			aClient->fMsgr.SendMessage(&msg);
		else
			Display(tempString.String(), C_OP);
	}
		return true;

	case RPL_CHANNELMLOCK: // 325
	{
		BString theChan(GetWord(data, 4)), mLock(GetWord(data, 8)), lockMessage(S_PENUM_MLOCK);
		lockMessage += theChan;
		lockMessage += ": ";
		lockMessage += mLock;
		lockMessage += "\n";

		BMessage display(M_DISPLAY);

		PackDisplay(&display, lockMessage.String(), C_OP, C_BACKGROUND, F_TEXT);
		ClientAgent* theClient(Client(theChan.String()));
		if (theClient)
			theClient->fMsgr.SendMessage(&display);
		else
			fMsgr.SendMessage(&display);
	}
		return true;

	case RPL_CHANNELCREATED: // 329
	{
		BString theChan(GetWord(data, 4)), theTime(GetWord(data, 5)), tempString;

		int32 serverTime(strtoul(theTime.String(), NULL, 0));
		struct tm ptr;
		time_t st;
		char str[80];
		st = serverTime;
		localtime_r(&st, &ptr);
		strftime(str, 80, "%a %b %d %Y %I:%M %p %Z", &ptr);
		BString theTimeParsed(str);
		theTimeParsed.RemoveAll("\n");

		tempString += theChan;
		tempString += " " S_PENUM_CHANCREATED " ";
		tempString += theTimeParsed;
		tempString += '\n';
		Display(tempString.String());
	}
		return true;

	case RPL_NOTOPIC: // 331
	{
		BString theChan(GetWord(data, 4)), tempString(S_PENUM_NO_TOPIC);
		tempString += theChan;
		tempString += '\n';

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_ERROR);
		PostActive(&msg);
	}
		return true;

	case RPL_TOPIC: // 332
	{
		BString theChannel(GetWord(data, 4)), theTopic(RestOfString(data, 5));
		ClientAgent* client(Client(theChannel.String()));

		theTopic.RemoveFirst(":");

		if (client) {
			BMessage display(M_DISPLAY);
			BString buffer;

			buffer += S_PENUM_DISP_TOPIC;
			buffer += theTopic;
			buffer += '\n';
			PackDisplay(&display, buffer.String(), C_WHOIS);

			BMessage msg(M_CHANNEL_TOPIC);
			msg.AddString("topic", theTopic.String());
			msg.AddMessage("display", &display);

			if (client->fMsgr.IsValid()) client->fMsgr.SendMessage(&msg);
		}
	}
		return true;

	case RPL_TOPICSET: // 333
	{
		BString channel(GetWord(data, 4)), user(GetWord(data, 5)), theTime(GetWord(data, 6));

		int32 serverTime(strtoul(theTime.String(), NULL, 0));
		struct tm ptr;
		time_t st;
		char str[80];
		st = serverTime;
		localtime_r(&st, &ptr);
		strftime(str, 80, "%A %b %d %Y %I:%M %p %Z", &ptr);
		BString theTimeParsed(str);
		theTimeParsed.RemoveAll("\n");

		ClientAgent* client(Client(channel.String()));

		if (client) {
			BMessage display(M_DISPLAY);
			BString buffer;

			buffer += S_PENUM_TOPIC_SET_BY;
			buffer += user;
			buffer += " @ ";
			buffer += theTimeParsed;
			buffer += '\n';
			PackDisplay(&display, buffer.String(), C_WHOIS);
			if (client->fMsgr.IsValid()) client->fMsgr.SendMessage(&display);
		}
	}
		return true;

	case RPL_INVITING: // 341
	{
		BString channel(GetWord(data, 5)), theNick(GetWord(data, 4)), tempString;

		tempString += "*** ";
		tempString += theNick;
		tempString += S_PENUM_INVITING;
		tempString += channel;
		tempString += ".\n";

		BMessage display(M_DISPLAY);

		PackDisplay(&display, tempString.String(), C_WHOIS);
		PostActive(&display);
	}
		return true;

	case RPL_NAMEREPLY: // 353
	{
		BString channel(GetWord(data, 5)), names(RestOfString(data, 6));
		ClientAgent* client(Client(channel.String()));
		names.RemoveFirst(":");

		BString tempString(S_PENUM_NAMEREPLY);
		tempString += channel;
		tempString += ": ";
		tempString += names;
		tempString += '\n';
		Display(tempString.String(), C_TEXT);

		if (client) // in the channel
		{
			BMessage msg(M_CHANNEL_NAMES);
			BString nick;
			int32 place(1);

			while ((nick = GetWord(names.String(), place)) != "-9z99") {
				const char* sNick(nick.String());
				bool founder(false), protect(false), op(false), voice(false), helper(false),
					ignored;

				if (nick[0] == '*') {
					++sNick;
					founder = true;
				} else if (nick[0] == '!') {
					++sNick;
					protect = true;
				} else if (nick[0] == '@') {
					++sNick;
					op = true;
				} else if (nick[0] == '+') {
					++sNick;
					voice = true;
				} else if (nick[0] == '%') {
					++sNick;
					helper = true;
				}

				ignored = false;
				// BMessage aMsg (M_IS_IGNORED), reply;
				// aMsg.AddString ("server", fServerName.String());
				// aMsg.AddString ("nick", sNick);

				// be_app_messenger.SendMessage (&aMsg, &reply);
				// reply.FindBool ("ignored", &ignored);

				msg.AddString("nick", nick.String());
				msg.AddBool("founder", founder);
				msg.AddBool("protect", protect);
				msg.AddBool("op", op);
				msg.AddBool("voice", voice);
				msg.AddBool("helper", helper);
				msg.AddBool("ignored", ignored);
				++place;
			}

			if (client->fMsgr.IsValid()) client->fMsgr.SendMessage(&msg);
		}
	}
		return true;

	case RPL_MOTD:			// 372
	case RPL_MOTDALT:		// 378
	case RPL_OPERMOTDSTART: // 609
	case RPL_OPERMOTD:		// 610
	case RPL_OPERENDOFMOTD: // 611
	{
		BString tempString(RestOfString(data, 4));
		tempString.RemoveFirst(":");
		tempString.Append("\n");
		Display(tempString.String(), C_SERVER, C_BACKGROUND, F_SERVER);
	}
		return true;

	case RPL_MOTDSTART: // 375
	{
		BString tempString(S_PENUM_SERVER_MOTD "\n");
		Display(tempString.String(), C_SERVER, C_BACKGROUND, F_SERVER);
	}
		return true;

	case RPL_ENDOFMOTD: // 376
	case ERR_NOMOTD:	// 422
	{
		BString tempString(RestOfString(data, 4));
		tempString.RemoveFirst(":");
		tempString.Append("\n");

		Display(tempString.String(), C_SERVER, C_BACKGROUND, F_SERVER);

		if (fInitialMotd && fCmds.Length()) {
			BMessage msg(M_SUBMIT_INPUT);
			const char* place(fCmds.String()), *eol;

			msg.AddInt32("which", PASTE_MULTI_NODELAY);

			while ((eol = strchr(place, '\n')) != 0) {
				BString line;

				line.Append(place, eol - place);
				msg.AddString("data", line.String());
				ParseAutoexecChans(line);
				place = eol + 1;
			}

			if (*place) {
				// check in case this was the only line
				ParseAutoexecChans(BString(place));
				msg.AddString("data", place);
			}

			msg.AddInt32("which", 3);
			msg.AddBool("autoexec", true);
			fMsgr.SendMessage(&msg);
		}

		if (fLocalip_private) {
			BString IPCommand("/userhost ");
			IPCommand += fMyNick;
			ParseCmd(IPCommand.String());
		}

		if (fReconnecting) {
			const char* reString;
			reString = S_PENUM_RECON_SUCCESS "\n";
			Display(reString, C_ERROR);
			DisplayAll(reString, C_ERROR, C_BACKGROUND, F_SERVER);
			fMsgr.SendMessage(M_REJOIN_ALL);
			fReconnecting = false;
		}

		fInitialMotd = false;
	}
		return true;

	case RPL_USERSSTART: // 392
	{
		// empty for now
	}
		return true;

	case RPL_USERS: // 393
	{
		// empty for now
	}
		return true;

	case ERR_ERRONEOUSNICKNAME:   // 432
	case ERR_NICKNAMEINUSE:		  // 433
	case ERR_RESOURCEUNAVAILABLE: // 437
	{
		BString theNick(GetWord(data, 4));

		if (fIsConnecting) {
			BString nextNick(GetNextNick());
			if (nextNick != "") {
				Display(S_PENUM_NICKINUSE1);
				Display(theNick.String());
				Display(S_PENUM_NICKINUSE2);
				Display(nextNick.String());
				Display("\"\n");

				BString tempString("NICK ");
				tempString += nextNick;
				SendData(tempString.String());
				return true;
			} else {
				Display(S_PENUM_ALLNICKSUSED1 "\n");
				Display(S_PENUM_ALLNICKSUSED2 "\n");
				return true;
			}
		}
		BString tempString;
		tempString += S_PENUM_NICKINUSE3;
		tempString += theNick;
		tempString += S_PENUM_NICKINUSE4 "\n";

		BMessage display(M_DISPLAY);
		PackDisplay(&display, tempString.String(), C_NICK);
		PostActive(&display);
	}
		return true;

	case ERR_USERNOTINCHANNEL: // 441
	{
		BString theChannel(GetWord(data, 5)), theNick(GetWord(data, 4)), tempString("[x] ");
		tempString += theNick;
		tempString += S_PENUM_NOTINCHANNEL;
		tempString += theChannel;
		tempString += ".\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_ERROR);
		PostActive(&msg);
	}
		return true;

	case ERR_NOTONCHANNEL: // 442
	{
		BString theChannel(GetWord(data, 4)), tempString(S_PENUM_ME_NOTINCHANNEL);
		tempString += theChannel;
		tempString += ".\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_ERROR);
		PostActive(&msg);
	}
		return true;

	case ERR_USERONCHANNEL: // 443
	{
		BString theChannel(GetWord(data, 5)), theNick(GetWord(data, 4)), tempString("[x] ");
		tempString += theNick;
		tempString += S_PENUM_ALREADYINCHANNEL;
		tempString += theChannel;
		tempString += ".\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_ERROR);
		PostActive(&msg);
	}
		return true;

	case ERR_KEYSET: // 467
	{
		BString theChannel(GetWord(data, 4)), tempString(S_PENUM_KEY_ALREADY_SET);
		tempString += theChannel;
		tempString += ".\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_ERROR);
		PostActive(&msg);
	}
		return true;

	case ERR_UNKNOWNMODE: // 472
	{
		BString theMode(GetWord(data, 4)), tempString(S_PENUM_UNKNOWNCHANMODE);
		tempString += theMode;
		tempString += "'\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_QUIT);
		PostActive(&msg);
	}
		return true;

	case ERR_INVITEONLYCHAN: // 473
	{
		BString theChan(GetWord(data, 4)), tempString("[x] "), theReason(RestOfString(data, 5));
		theReason.RemoveFirst(":");
		theReason.ReplaceLast("channel", theChan.String());
		tempString += theReason;
		tempString += S_PENUM_INVITE_ONLY "\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_QUIT, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
		RemoveAutoexecChan(theChan);
	}
		return true;

	case ERR_BANNEDFROMCHAN: // 474
	{
		BString theChan(GetWord(data, 4)), tempString("[x] "), theReason(RestOfString(data, 5));
		theReason.RemoveFirst(":");
		theReason.ReplaceLast("channel", theChan.String());

		tempString += theReason;
		tempString += S_PENUM_BANNED "\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_QUIT, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
		RemoveAutoexecChan(theChan);
	}
		return true;

	case ERR_BADCHANNELKEY: // 475
	{
		BString theChan(GetWord(data, 4)), theReason(RestOfString(data, 5)), tempString("[x] ");
		theReason.RemoveFirst(":");
		theReason.ReplaceLast("channel", theChan.String());
		tempString += theReason;
		tempString += S_PENUM_BADCHANKEY "\n";

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, tempString.String(), C_QUIT, C_BACKGROUND, F_SERVER);
		PostActive(&msg);
		RemoveAutoexecChan(theChan);
	}
		return true;

	case ERR_UMODEUNKNOWNFLAG: // 501
	{
		BMessage msg(M_DISPLAY);
		BString buffer;

		buffer += S_PENUM_UNKNOWNMODE "\n";
		PackDisplay(&msg, buffer.String(), C_QUIT);
		PostActive(&msg);
	}
		return true;

	// not sure what these numerics are,
	// but they are usually on-connect messages
	case RPL_290: // 290
	case RPL_291: // 291
	case RPL_292: // 292
	{
		BString tempString(RestOfString(data, 4));
		tempString.RemoveFirst(":");
		tempString.Append("\n");
		tempString.Prepend("- ");
		Display(tempString.String());
	}
		return true;

	case RPL_WHOISREGISTEREDBOT: // 617
	{
		// conflicts with RPL_DCCALLOWCHANGE
		BString theNick(GetWord(data, 4)), theMessage(RestOfString(data, 5)), tempString;
		theNick.RemoveFirst(":");
		theMessage.RemoveFirst(":");
		theMessage.Append("\n");

		switch (fIrcdtype) {
		case IRCD_ULTIMATE: {
			tempString += "[@] ";
			tempString += theMessage;
			BMessage msg(M_DISPLAY);
			PackDisplay(&msg, tempString.String(), C_WHOIS, C_BACKGROUND, F_SERVER);
			PostActive(&msg);
		} break;

		default: {
			tempString += theNick;
			tempString += " ";
			tempString += theMessage;
			Display(tempString.String());
		}
		}
	}
		return true;

	default:
		break;
	}

	return false;
}
