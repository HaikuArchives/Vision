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
 *								 John Robinson
 *								 Alan Ellis <alan@cgsoftware.org>
 *								 Francois Revol
 */

#include <UTF8.h>
#include <Autolock.h>
#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <FilePanel.h>
#include <MessageRunner.h>
#include <Path.h>
#include <String.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/select.h>

#include "ClientAgent.h"
#include "ClientAgentLogger.h"
#include "ClientWindow.h"
#include "ChannelAgent.h"
#include "DCCConnect.h"
#include "ListAgent.h"
#include "MessageAgent.h"
#include "NetworkManager.h"
#include "NotifyList.h"
#include "ServerAgent.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Vision.h"
#include "WindowList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerMessages"

int32 ServerAgent::fServerSeed = 0;

ServerAgent::ServerAgent (
	const char *id_,
	BMessage &net,
	BRect frame_)

	: ClientAgent (
		id_,
		id_,
		net.FindString ("nick"),
		frame_),
		fLocalip (""),
		fMyNick (net.FindString ("nick")),
		fMyLag ((net.FindBool ("lagCheck")) ? "0.000" : B_TRANSLATE("Disabled")),
		fLname (net.FindString ("realname")),
		fLident (net.FindString ("ident")),
		fConnectionID (-1),
		fGetLocalIP (false),
		fIsConnecting (true),
		fReconnecting (false),
		fConnected (false),
		fIsQuitting (false),
		fCheckingLag (false),
		fReacquiredNick (true),
		fRetry (1),
		fRetryLimit (47),
		fLagCheck (0),
		fLagCount (0),
		fNickAttempt (0),
		fEvents (vision_app->fEvents),
		fServerHostName (id_),
		fInitialMotd (true),
		fCmds (net.FindString ("autoexec")),
		fListAgent (NULL),
		fNetworkData (net),
		fServerIndex (0),
		fNickIndex (0),
		fLogger (NULL)
{
	fLogger = new ClientAgentLogger(fId);
}

ServerAgent::~ServerAgent (void)
{
	if (fLagRunner)
		delete fLagRunner;

	while (fStartupChannels.CountItems() != 0)
		delete fStartupChannels.RemoveItemAt ((int32)0);

	delete fLogger;

//	wait_for_thread (fLoginThread, &result);

/*
	while (fIgnoreNicks.CountItems() > 0)
		delete fIgnoreNicks.RemoveItem((int32)0);
*/
	while (fNotifyNicks.CountItems() > 0)
		delete fNotifyNicks.RemoveItemAt((int32)0);
}

void
ServerAgent::AttachedToWindow(void)
{
	Init();
	ClientAgent::AttachedToWindow();
}

void
ServerAgent::AllAttached (void)
{
	fSMsgr = BMessenger (this);
	ClientAgent::AllAttached ();

	// build notify list here since we can't send a message to ourselves sooner

	type_code type;
	int32 attrCount (0);

	BMessage updatemsg (M_NOTIFYLIST_ADD);
	BString data;

	fNetworkData.GetInfo ("notify", &type, &attrCount);

	for (int32 i = 0; i < attrCount; i++)
	{
		data += fNetworkData.FindString("notify", i);
		data += " ";
	}
	if (attrCount > 0)
	{
		updatemsg.AddString("cmd", data.String());
		fSMsgr.SendMessage(&updatemsg);
	}
}

// do nothing for now
void
ServerAgent::AddMenuItems(BPopUpMenu *)
{
}


void
ServerAgent::Init (void)
{
	BString revString;
	Display ("Vision ");
	vision_app->VisionVersion (VERSION_VERSION, revString);
	Display (revString.String(), C_MYNICK);
	Display (" built on ");
	vision_app->VisionVersion (VERSION_DATE, revString);
	Display (revString.String());
	Display ("\nThis agent goes by the name of Smith... err ");
	BString temp;
	temp << fId << "\n";
	Display (temp.String(), C_NICK);
	Display ("Have fun!\n");

	fLagRunner = new BMessageRunner (
		this,			 // target ServerAgent
		new BMessage (M_LAG_CHECK),
		10000000,	 // 10 seconds
		-1);				// forever

	SendConnectionCreate();
}


int
ServerAgent::IRCDType (void)
{
	return fIrcdtype;
}


status_t
ServerAgent::NewTimer (const char *, int32, int32)
{
// TODO: implement this once scripting is ready
	return B_OK;
}

int32
ServerAgent::Timer (void *arg)
{
	BMessage *msg (reinterpret_cast<BMessage *>(arg));
	ServerAgent *agent;
	const char *cmd;
	int32 sleeptimer,
				loops;

	if ((msg->FindString	("command", &cmd) != B_OK)
	||	(msg->FindInt32	 ("loops", &loops) != B_OK)
	||	(msg->FindInt32	 ("sleep", &sleeptimer) != B_OK)
	||	(msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
	{
		printf (":ERROR: couldn't find valid data in BMsg to Timer() -- bailing\n");
		return B_ERROR;
	}


	return B_OK;

}

int
ServerAgent::SortNotifyItems (const NotifyListItem *item1, const NotifyListItem *item2)
{
	if (!item1 || !item2)
		return 0;

	if (item1->GetState() && !item2->GetState())
		return -1;
	if (!item1->GetState() && item2->GetState())
		return 1;

	BString name (item1->Text());

	return name.ICompare(item2->Text());
}

void
ServerAgent::SendData (const char *cData)
{
	BMessage dataSend(M_SEND_CONNECTION_DATA);
	BString data(cData);
	data.Append("\r\n");

	int32 encoding = vision_app->GetInt32("encoding");
	if (encoding != B_UNICODE_CONVERSION)
	{
		char sendBuffer[2048];
		int32 state = 0;
		int32 length = data.Length();
		int32 destLength = length;
		convert_from_utf8(encoding, data.String(), &length, sendBuffer, &destLength, &state);
		data.SetTo(sendBuffer, destLength);

	}
	dataSend.AddInt32("connection", fConnectionID);
	dataSend.AddData("data", B_RAW_TYPE, data.String(), data.Length());
	BMessenger(network_manager).SendMessage(&dataSend);

	if (vision_app->fDebugSend)
	{
		data.RemoveAll("\n");
		data.RemoveAll("\r");
		printf("	 SENT: (%03" B_PRId32 ") \"%s\"\n", data.Length(), data.String());
	}

}

void
ServerAgent::ParseLine (const char *cData)
{
	char parseBuffer[2048];
	BString data = FilterCrap (cData);
	int32 length (data.Length()),
				destLength (sizeof(parseBuffer)),
				state (0);

	memset (parseBuffer, 0, sizeof (parseBuffer));
	int32 encoding = vision_app->GetInt32("encoding");
	if (encoding != B_UNICODE_CONVERSION)
	{
		convert_to_utf8 (
			encoding,
			data.String(),
			&length,
			parseBuffer,
			&destLength,
			&state);
	}
	else
	{
		if (IsValidUTF8(data.String(), destLength))
		{
			strlcpy(parseBuffer, data.String(), destLength);
		}
		else
		{
			convert_to_utf8 (
				B_ISO1_CONVERSION,
				data.String(),
				&length,
				parseBuffer,
				&destLength,
				&state);
		}
	}
	if (vision_app->fNumBench)
	{
		vision_app->fBench1 = system_time();
		if (ParseEvents (parseBuffer))
		{
			vision_app->fBench2 = system_time();
			BString bencht (GetWord (data.String(), 2));
			vision_app->BenchOut (bencht.String());
			return;
		}
	}
	else
	{
		if (ParseEvents (parseBuffer))
			return;
	}

	data.Append("\n");
	Display (data.String(), 0);
}

ClientAgent *
ServerAgent::Client (const char *cName)
{
	ClientAgent *client (0);

	for (int32 i = 0; i < fClients.CountItems(); ++i)
	{
		ClientAgent *item (fClients.ItemAt (i));
		if (strcasecmp (cName, item->Id().String()) == 0)
		{

			client = item;
			break;
		}
	}
	return client;
}

ClientAgent *
ServerAgent::ActiveClient (void)
{
	ClientAgent *client (NULL);
//	printf("finding active client\n");

	for (int32 i = 0; i < fClients.CountItems(); i++)
	{
//		printf("checking against client: %d, %s\n", i, fClients.ItemAt(i)->fId.String());
		if (!fClients.ItemAt (i)->IsHidden())
		{
//			printf("not hidden, break\n");
			client = fClients.ItemAt (i);
			break;
		}
	}
	return client;
}


void
ServerAgent::Broadcast (BMessage *msg, bool sendToServer)
{
	for (int32 i = 0; i < fClients.CountItems(); ++i)
	{
		ClientAgent *client (fClients.ItemAt (i));

		if (client != this)
			client->fMsgr.SendMessage (msg);
	}
	if (sendToServer)
	{
		fSMsgr.SendMessage(msg);
	}
}

void
ServerAgent::RepliedBroadcast (BMessage *)
{
//	TODO: implement this
//	BMessage cMsg (*msg);
//	BAutolock lock (this);
//
//	for (int32 i = 0; i < fClients.CountItems(); ++i)
//	{
//		ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));
//
//		if (client != this)
//		{
//			BMessenger fMsgr (client);
//			BMessage reply;
//			fMsgr.SendMessage (&cMsg, &reply);
//		}
//	}
}


void
ServerAgent::DisplayAll (
	const char *buffer,
	const uint32 fore,
	const uint32 back,
	const uint32 font)
{
	for (int32 i = 0; i < fClients.CountItems(); ++i)
	{
		ClientAgent *client (fClients.ItemAt (i));

		BMessage msg (M_DISPLAY);
		PackDisplay (&msg, buffer, fore, back, font);
		client->fMsgr.SendMessage (&msg);
	}

	return;
}

void
ServerAgent::PostActive (BMessage *msg)
{
	BAutolock activeLock (Window());
//	printf("postActive\n");
	ClientAgent *client (ActiveClient());
//	printf("posting to: %p\n", client);
	if (client != NULL)
		client->fMsgr.SendMessage (msg);
	else
		fSMsgr.SendMessage (msg);
}

void
ServerAgent::HandleReconnect (void)
{
	/*
	 * Function purpose: Setup the environment and attempt a new connection
	 * to the server
	 */

	if (fRetry < fRetryLimit)
	{
		BString message;
		if (fRetry > 1)
		{
			message = B_TRANSLATE("Attempting to reconnect (attempt %1 of %2), waiting %3 second(s)" B_UTF8_ELLIPSIS);
		}
		else
		{
			message = B_TRANSLATE("Attempting to connect (attempt %1 of %2)");
		}
		message.Prepend("[@] ").Append("\n");
		BString temp;
		temp << fRetry;
		message.ReplaceFirst("%1", temp);
		temp = "";
		temp << fRetryLimit;
		message.ReplaceFirst("%2", temp);
		// we are go for main engine start
		fReconnecting = true;
		fIsConnecting = true;
		fNickAttempt = 0;
		int seconds = fRetry * fRetry;
		temp = "";
		temp << seconds;
		message.ReplaceFirst("%3", temp);
		Display(message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
		ClientAgent *agent (ActiveClient());
		if (agent && (agent != this))
			agent->Display (message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
		SendConnectionCreate(seconds * 1000000);
		++fRetry;
	}
	else
	{
		// we've hit our fRetry limit. throw in the towel
		fReconnecting = false;
		fRetry = 0;
		BString message = "[@] ";
		message += B_TRANSLATE("Retry limit reached; giving up. Type /reconnect to try connecting again.");
		message += "\n";
		Display (message.String(), C_ERROR);
		ClientAgent *agent (ActiveClient());
		if (agent && (agent != this))
			agent->Display (message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
	}
}

const ServerData *
ServerAgent::GetNextServer ()
{
	type_code type;
	int32 count;
	ssize_t size;
	fNetworkData.GetInfo ("server", &type, &count);
	uint32 state = (fReconnecting) ? SERVER_SECONDARY : SERVER_PRIMARY;

	for (;;)
	{
		if (fServerIndex >= count)
		{
			fServerIndex = 0;
			state = SERVER_PRIMARY;
		}

		const ServerData *server (NULL);
		for (; fNetworkData.FindData ("server", B_RAW_TYPE, fServerIndex,
			reinterpret_cast<const void **>(&server), &size) == B_OK; fServerIndex++)
				if (server->state == state)
				{
					memset(&fCurrentServer, 0, sizeof(ServerData));
					memcpy(&fCurrentServer, server, size);
					return &fCurrentServer;
				}
	}
}

const char *
ServerAgent::GetNextNick ()
{
	type_code type;
	int32 count;
	fNetworkData.GetInfo ("nick", &type, &count);
	if (fNickIndex < count)
		return fNetworkData.FindString ("nick", fNickIndex++);
	else
	{
		fNickIndex = 0;
		return "";
	}
}

bool
ServerAgent::PrivateIPCheck (const char *ip)
{
	/*
	 * Function purpose: Compare against fLocalip to see if it is a private address
	 *									 if so, set fLocalip_private to true;
	 *
	 * Private ranges: 10.0.0.0		- 10.255.255.255
	 *								 172.16.0.0	- 172.31.255.255
	 *								 192.168.0.0 - 192.168.255.255
	 *								 (as defined in RFC 1918)
	 */

	 if (ip == NULL || strlen(ip) == 0)
	 {
		 // it is obviously a mistake we got called.
		 // setup some sane values and print an assertion
		 printf (":ERROR: PrivateIPCheck() called when there is no valid data to check!\n");
		 return true;
	 }

	 if (strcmp(ip, "127.0.0.1") == 0)
		 return true;

	 // catch 10.0.0.0 - 10.255.255.255 and 192.168.0.0 - 192.168.255.255
	 if (	(strncmp (ip, "10.", 3) == 0)
			|| (strncmp (ip, "192.168.", 8) == 0))
		 return true;

	 // catch 172.16.0.0 - 172.31.255.255
	 if (strncmp (ip, "172.", 4) == 0)
	 {
		 // check to see if characters 5-6 are (or are between) 16 and 31
		 {
			 char temp172s[3];
			 temp172s[0] = ip[4];
			 temp172s[1] = ip[5];
			 temp172s[2] = '\0';
			 int temp172n (atoi (temp172s));

			 if (temp172n >= 16 || temp172n <= 31)
				 return true;
		 }
		 return false;
	 }

	// if we got this far, its a public IP address
	return false;

}

void
ServerAgent::AddResumeData (BMessage *msg)
{
	ResumeData *data;

	data = new ResumeData;

	data->expire = system_time() + 5000000LL;
	data->nick	 = msg->FindString ("vision:nick");
	data->file	 = msg->FindString ("vision:file");
	data->size	 = msg->FindString ("vision:size");
	data->ip		 = msg->FindString ("vision:ip");
	data->port	 = msg->FindString ("vision:port");
	data->path	 = msg->FindString ("path");
	data->pos		= msg->FindInt64	("pos");

	//PRINT(("%s %s %s %s %s", data->nick.String(), data->file.String(),
	//	data->size.String(), data->ip.String(), data->port.String()));
	fResumes.AddItem (data);

	BString buffer;

	buffer << "PRIVMSG "
		<< data->nick
		<< " :\1DCC RESUME "
		<< data->file
		<< " "
		<< data->port
		<< " "
		<< data->pos
		<< "\1";

	SendData (buffer.String());
}

void
ServerAgent::ParseAutoexecChans (const BString &origLine)
{
	BString line (origLine);
	int32 chanIndex (0);
	if ((chanIndex = line.IFindFirst ("/JOIN")) != B_ERROR)
	{
		chanIndex += 6;
		line.Remove (0, chanIndex);
	}
	else if ((chanIndex = line.IFindFirst ("/J")) != B_ERROR)
	{
		chanIndex += 3;
		line.Remove (0, chanIndex);
	}
	else
		return;
	// parse out all autoexec channels to ensure we don't try to focus those
	// on join
	chanIndex = 0;
	BString *newChan (NULL);
	for (;;)
	{
		if ((chanIndex = line.FindFirst (',')) == B_ERROR)
			break;
		newChan = new BString();
		line.CopyInto (*newChan, 0, chanIndex);
		if ((*newChan)[0] != '#')
			newChan->Prepend("#");
		fStartupChannels.AddItem (newChan);
		line.Remove (0, chanIndex + 1);
	}
	newChan = new BString();
	// catch last channel (or only channel if no comma separations)
	if ((chanIndex = line.FindFirst (' ')) != B_ERROR)
		line.CopyInto (*newChan, 0, chanIndex);
	else
		*newChan = line;
	if ((*newChan)[0] != '#')
		newChan->Prepend("#");
	fStartupChannels.AddItem (newChan);
}

void
ServerAgent::SendConnectionCreate(bigtime_t timeout)
{
	BMessage msg (M_CREATE_CONNECTION);
	msg.AddMessenger("target", BMessenger(this));

	BString port;
	const ServerData *data = GetNextServer();
	port << data->port;
	vision_app->AddIdent(data->serverName, fLident);
	msg.AddString("hostname", data->serverName);
	msg.AddString("port", port);
	if (timeout > 0)
	{
		msg.AddInt64("timeout", timeout);
	}
	BString message;
	message = B_TRANSLATE("Attempting a connection to %1:%2" B_UTF8_ELLIPSIS);
	message.Prepend("[@] ").Append("\n");
	message.ReplaceFirst("%1", data->serverName);
	message.ReplaceFirst("%2", port);
	Display(message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
	BMessenger(network_manager).SendMessage(&msg);
}

void
ServerAgent::RemoveAutoexecChan (const BString &chan)
{
	int32 chanCount (fStartupChannels.CountItems());
	for (int32 i = 0; i < chanCount; i++)
		if (fStartupChannels.ItemAt (i)->ICompare(chan) == 0)
		{
			delete fStartupChannels.RemoveItemAt (i);
			return;
		}
}

void
ServerAgent::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_PARSE_LINE:
			{
				const char *buffer (NULL);
				msg->FindString ("line", &buffer);
				if (buffer)
					ParseLine (buffer);
			}
			break;

		case M_STATE_CHANGE:
			{
				Broadcast(msg);
				ClientAgent::MessageReceived (msg);
			}
			break;

		case M_SEND_RAW:
			{
				const char *buffer;
				msg->FindString ("line", &buffer);
				if (buffer)
					SendData (buffer);
			}
			break;

		case M_DISPLAY_ALL:
			{
				BString data;
				msg->FindString	("data", &data);
				DisplayAll (data.String(), msg->FindInt32 ("fore"), msg->FindInt32 ("back"), msg->FindInt32 ("font"));
			}
			break;

		case M_CONNECTION_CREATED:
			{
				int32 result = msg->FindInt32("status");
				if (result != B_OK)
				{
					fIsConnecting = false;
					fReconnecting = false;
					fConnectionID = -1;
					BString data = B_TRANSLATE("Could not create connection to address and port. Make sure your Internet connection is operational.");
					data.Prepend("[@] ").Append("\n");
					Display(data.String(), C_ERROR, C_BACKGROUND, F_SERVER);
					HandleReconnect();
				}
				else
				{
					BString message;
					message = B_TRANSLATE("Handshaking");
					message.Prepend("[@] ").Append("\n");
					Display(message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
					fIsConnecting = true;
					fConnectionID = msg->FindInt32("connection");
					BString data;

					if (strlen(fCurrentServer.password) > 0)
					{
						message = B_TRANSLATE("Sending password.");
						message.Prepend("[@] ").Append("\n");
						Display(message.String(), C_ERROR, C_BACKGROUND, F_SERVER);
						data = "PASS ";
						data += fCurrentServer.password;
						SendData(data.String());
					}

					sockaddr_storage storage;
					socklen_t slen = sizeof(storage);
					getsockname(fConnectionID, (struct sockaddr *)&storage, &slen);
					char buf[128];
					getnameinfo(reinterpret_cast<sockaddr *>(&storage), slen, buf, sizeof(buf), NULL, 0, NI_NUMERICHOST);
					fLocalip = buf;
					fLocalip_private = PrivateIPCheck(buf);
					if (fLocalip_private && vision_app->GetBool("dccPrivateCheck"))
					{
						fGetLocalIP = true;
					}

					data = "USER ";
					data.Append (fLident);
					data.Append (" localhost ");
					data.Append (fCurrentServer.serverName);
					data.Append (" :");
					data.Append (fLname);
					SendData(data.String());

					data = "NICK ";
					data += fMyNick;
					SendData(data.String());

				}
			}
			break;

		case M_CONNECTION_DISCONNECT:
			{
				if (fIsQuitting)
					break;

				fIsConnecting = false;
				BString data = B_TRANSLATE("Disconnected from %1");
				data.ReplaceFirst("%1", fServerHostName.String());
				data.Prepend("[@] ").Append("\n");
				Display(data.String(), C_ERROR, C_BACKGROUND, F_SERVER);
				HandleReconnect();
			}
			break;

		case M_CONNECTION_DATA_RECEIVED:
			{
				const char *buffer (NULL);
				ssize_t numBytes = 0;
				if (msg->FindData("data", B_RAW_TYPE, reinterpret_cast<const void **>(&buffer), &numBytes) == B_OK)
				{
					BString tempBuffer(buffer, numBytes);
					int32 pos = -1;
					while ((pos = tempBuffer.FindFirst('\n')) >= 0)
					{
						fPartialBuffer.Append(tempBuffer, pos + 1);
						tempBuffer.Remove(0, pos + 1);
						fPartialBuffer.RemoveLast("\r");
						if (vision_app->fDebugRecv)
						{
							printf ("RECEIVED: (%" B_PRId32 ":%03" B_PRId32 ") \"", fConnectionID, fPartialBuffer.Length());
							for (int32 i = 0; i < fPartialBuffer.Length(); ++i)
							{
								if (isprint (fPartialBuffer.String()[i]))
									printf ("%c", fPartialBuffer.String()[i]);
								else
									printf ("[0x%02x]", fPartialBuffer.String()[i]);
							}
							printf ("\"\n");
						}
						ParseLine(fPartialBuffer.String());
						fPartialBuffer.SetTo("");
					}
					if (tempBuffer.Length() > 0)
					{
						fPartialBuffer = tempBuffer;
					}
				}
			}
			break;

		case M_INC_RECONNECT:
			++fRetry;
			break;

		case M_SET_IP:
			{
				static BString ip;
				msg->FindString("ip", &ip);
				fLocalip = ip.String();
				fGetLocalIP = false;
			}
			break;

		case M_GET_IP:
			{
				BMessage reply;
				reply.AddBool ("private", fLocalip_private);
				reply.AddString ("ip", fLocalip);
				msg->SendReply (&reply);
			}
			break;

		case M_GET_RECONNECT_STATUS:
			{
				BMessage reply (B_REPLY);
				reply.AddInt32 ("retries", fRetry);
				reply.AddInt32 ("max_retries", fRetryLimit);
				msg->SendReply(&reply);
			}
			break;

		case M_SERVER_SEND:
			{
				BString buffer;
				int32 i;

				for (i = 0; msg->HasString ("data", i); ++i)
				{
					const char *str;

					msg->FindString ("data", i, &str);
					buffer << str;
				}

				SendData (buffer.String());
				if (msg->IsSourceWaiting())
					msg->SendReply(B_REPLY);
			}
			break;

		case M_DCC_ACCEPT:
			{
				bool cont (false);
				const char *nick,
	 								 *size,
						 *ip,
						 *port;
				BPath path;

				msg->FindString("vision:nick", &nick);
				msg->FindString("vision:size", &size);
				msg->FindString("vision:ip", &ip);
				msg->FindString("vision:port", &port);

				if (msg->HasString ("path"))
					path.SetTo (msg->FindString ("path"));
				else
				{
					const char *file;
					entry_ref ref;

					msg->FindRef ("directory", &ref);
					msg->FindString("name", &file);

					BDirectory dir (&ref);
					path.SetTo (&dir, file);
				}

				if (msg->HasBool ("continue"))
					msg->FindBool ("continue", &cont);

				DCCReceive *view;
				view = new DCCReceive (
					nick,
					path.Path(),
					size,
					ip,
					port,
					fSMsgr,
					cont);

					BMessage aMsg (M_DCC_FILE_WIN);
					aMsg.AddPointer ("view", view);
					be_app->PostMessage (&aMsg);
			}
			break;

		case M_CHOSE_FILE: // DCC send
			{
				const char *nick (NULL);
				entry_ref ref;
				off_t size;
				msg->FindString ("nick", &nick);
				msg->FindRef ("refs", &ref); // get file

				BEntry entry (&ref);

/* // TODO: resolve if symlink

				if (entry.IsSymLink())
				{
					BSymLink link (&entry);
				}
*/
				BPath path (&entry);
				// PRINT(("file path: %s\n", path.Path()));
				entry.GetSize (&size);

				BString ssize;
				ssize << size;

				DCCSend *view;

				view = new DCCSend (
								nick,
								path.Path(),
								ssize.String(),
								fSMsgr);
						BMessage message (M_DCC_FILE_WIN);
						message.AddPointer ("view", view);
						vision_app->PostMessage (&message);

			}
			break;

		case M_ADD_RESUME_DATA:
			{
				AddResumeData (msg);
			}
			break;


		case B_CANCEL:
			if (msg->HasPointer ("source"))
			{
				BFilePanel *fPanel;
				msg->FindPointer ("source", reinterpret_cast<void **>(&fPanel));
				delete fPanel;
			}
			break;


		case M_CHAT_ACCEPT:
			{
				int32 acceptDeny;
				BString theNick;
				const char *theIP, *thePort;
				msg->FindInt32("which", &acceptDeny);
				if (acceptDeny)
					return;
				msg->FindString("nick", &theNick);
				msg->FindString("ip", &theIP);
				msg->FindString("port", &thePort);

				theNick.Append (" [DCC]");
				MessageAgent *newAgent (new MessageAgent (
						*vision_app->pClientWin()->AgentRect(),
						theNick.String(),
						fId.String(),
						fSMsgr,
						fMyNick.String(),
						"",
						true,
						false,
						theIP,
						thePort));
				vision_app->pClientWin()->pWindowList()->AddAgent (newAgent,
					theNick.String(),
					WIN_MESSAGE_TYPE,
					true);

					fClients.AddItem (newAgent);
			}
			break;

		case M_CHAT_ACTION: // dcc chat
		 {
			 ClientAgent *client;
			 const char *theNick;
			 BString thePort;
			 BString theId;

			 msg->FindString ("nick", &theNick);
			 msg->FindString ("port", &thePort);
			 theId << theNick << " [DCC]";

			 if ((client = Client (theId.String())) == 0)
			 {
					MessageAgent *newAgent (new MessageAgent (
							*vision_app->pClientWin()->AgentRect(),
							theId.String(),
							fId.String(),
							fSMsgr,
							fMyNick.String(),
							"",
							true,
							true,
							"",
							thePort != "" ? thePort.String() : ""));
					vision_app->pClientWin()->pWindowList()->AddAgent (newAgent,
						theId.String(),
						WIN_MESSAGE_TYPE,
						true);

				 fClients.AddItem (newAgent);
			 }
		 }
		 break;

		case M_SLASH_RECONNECT:
			if (fConnectionID < 0 && !fIsConnecting)
			{
				HandleReconnect();
			}
			break;

		case M_STATUS_ADDITEMS:
			{
				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
						0, ""),
					true);

				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
						"Lag: ",
						"",
						STATUS_ALIGN_LEFT),
					true);

				vision_app->pClientWin()->pStatusView()->AddItem (new StatusItem (
						0,
						"",
						STATUS_ALIGN_LEFT),
					true);

				// The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
				// We send true on the last SetItemValue().
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, fId.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String(), true);
			}
			break;

		case M_LAG_CHECK:
			{
				if (fConnectionID >= 0 && fConnected)
				{
					if (fNetworkData.FindBool ("lagCheck"))
					{
						BMessage lagSend (M_SERVER_SEND);
						AddSend (&lagSend, "VISION_LAG_CHECK");
						AddSend (&lagSend, endl);
						if (!fCheckingLag)
						{
							fLagCheck = system_time();
							fLagCount = 1;
							fCheckingLag = true;
						}
						else
						{
							if (fLagCount > 4)
							{
								// we've waited 50 seconds
								// connection problems?
								fMyLag = B_TRANSLATE("CONNECTION PROBLEM");
								fMsgr.SendMessage (M_LAG_CHANGED);
							}
							else
							{
								// wait some more
								char lag[15] = "";
								sprintf (lag, "%" B_PRId32 "0.000+", fLagCount);	// assuming a 10 second runner
								fMyLag = lag;
								++fLagCount;
								fMsgr.SendMessage (M_LAG_CHANGED);
							}
						}
					}
					if (fNotifyNicks.CountItems() > 0)
					{
						BString cmd ("ISON ");
						for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
						{
							cmd += " ";
							cmd += fNotifyNicks.ItemAt(i)->Text();
						}
						BMessage dataSend (M_SERVER_SEND);
						dataSend.AddString ("data", cmd.String());
						fSMsgr.SendMessage (&dataSend);
					}
				}
			}
			break;

		case M_LAG_CHANGED:
			{
				if (!IsHidden())
					vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), true);

				BMessage newmsg (M_LAG_CHANGED);
				newmsg.AddString ("lag", fMyLag);
				Broadcast (&newmsg);
			}
			break;

		case M_REJOIN_ALL:
			{
				for (int32 i = 0; i < fClients.CountItems(); ++i)
				{
					ClientAgent *client (fClients.ItemAt (i));

					if (dynamic_cast<ChannelAgent *>(client))
					{
						BMessage rejoinMsg (M_REJOIN);
						rejoinMsg.AddString ("nickname", fMyNick.String());
						client->fMsgr.SendMessage (&rejoinMsg);
					}
				}
			}
			break;

		case M_OPEN_MSGAGENT:
			{
				ClientAgent *client;
				const char *theNick;

				msg->FindString ("nick", &theNick);

				if (!(client = Client (theNick)))
				{
					MessageAgent *newAgent (new MessageAgent (
							*vision_app->pClientWin()->AgentRect(),
							theNick,
							fId.String(),
							fSMsgr,
							fMyNick.String(),
							""));
					vision_app->pClientWin()->pWindowList()->AddAgent (
						newAgent,
						theNick,
						WIN_MESSAGE_TYPE,
						true);

					client = newAgent;

					fClients.AddItem (newAgent);
				}
				else
					client->fAgentWinItem->ActivateItem();

				if (msg->HasMessage ("msg"))
				{
					BMessage buffer;

					msg->FindMessage ("msg", &buffer);
					client->fMsgr.SendMessage (&buffer);
				}
			}
			break;

		case M_CLIENT_QUIT:
			{
				bool shutingdown (false);

				if (msg->HasBool ("vision:shutdown"))
					msg->FindBool ("vision:shutdown", &shutingdown);

				if (msg->HasString ("vision:quit"))
				{
					const char *quitstr;
					msg->FindString ("vision:quit", &quitstr);
					fQuitMsg = quitstr;
				}

				if (!fIsQuitting && fConnectionID >= 0)
				{
					if (fQuitMsg.Length() == 0)
					{
						const char *expansions[1];
						BString version;
						vision_app->VisionVersion(VERSION_VERSION, version);
						expansions[0] = version.String();
						fQuitMsg << "QUIT :" << ExpandKeyed (vision_app->GetCommand (CMD_QUIT).String(), "V", expansions);
					}
					SendData (fQuitMsg.String());
				}

				fIsQuitting = true;

				if (fClients.CountItems() > 0)
				{
					Broadcast(msg);
					BMessenger listMsgr(fListAgent);
					listMsgr.SendMessage(M_CLIENT_QUIT);
				}
				else
					ClientAgent::MessageReceived(msg);
			}
			break;

		case M_CLIENT_SHUTDOWN:
			{
				ClientAgent *deadagent;

				if (msg->FindPointer ("agent", reinterpret_cast<void **>(&deadagent)) != B_OK)
				{
					printf (":ERROR: error getting valid pointer from M_CLIENT_SHUTDOWN -- bailing\n");
					break;
				}

				fClients.RemoveItem (deadagent);

				BMessage deathchant (M_OBITUARY);
				deathchant.AddPointer ("agent", deadagent);
				deathchant.AddPointer ("item", deadagent->fAgentWinItem);
				vision_app->pClientWin()->PostMessage (&deathchant);

				if (fIsQuitting && (fClients.CountItems() == 0) &&
					 (dynamic_cast<ServerAgent *>(deadagent) != this))
				{
					fSMsgr.SendMessage (M_CLIENT_QUIT);
				}

			}
			break;

		case M_LIST_COMMAND:
			{
				if (fListAgent)
					break;
				vision_app->pClientWin()->pWindowList()->AddAgent (
					(fListAgent = new ListAgent (
						*vision_app->pClientWin()->AgentRect(),
						fServerHostName.String(), new BMessenger(this))),
					"Channels",
					WIN_LIST_TYPE,
					true);
				// kind of a hack since Agent() returns a pointer of type ClientAgent, of which
				// ListAgent is not a subclass...
				BMessenger listMsgr(fListAgent);
				listMsgr.SendMessage(msg);
			}
			break;

		case M_LIST_SHUTDOWN:
			fListAgent = NULL;
			break;

		case M_REGISTER_LOGGER:
			{
				const char *logName;
				msg->FindString ("name", &logName);
				fLogger->RegisterLogger (logName);
			}
			break;

		case M_UNREGISTER_LOGGER:
			{
				const char *logName;
				msg->FindString ("name", &logName);
				fLogger->UnregisterLogger (logName);
			}
			break;

		case M_CLIENT_LOG:
			{
				const char *logName;
				const char *data;
				msg->FindString ("name", &logName);
				msg->FindString ("data", &data);
				fLogger->Log (logName, data);
			}
			break;

		case M_IGNORE_ADD:
			{
				BString cmd (msg->FindString("cmd"));
				BString curNick;
				int32 idx (-1);
				int32 i (0);


				type_code type;
				int32 attrCount;

				// make sure this nick hasn't already been added
				fNetworkData.GetInfo ("ignore", &type, &attrCount);

				// TODO: print notification message to user
				while ((idx = cmd.IFindFirst(" ")) > 0)
				{
					cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
					// remove leading space
					cmd.Remove(0, 1);
					for (i = 0; i < attrCount; i++)
						if (curNick.ICompare(fNetworkData.FindString("ignore", i)) == 0)
							break;
					// no dupes found, add it
					if (i == attrCount)
					{
						vision_app->AddIgnoreNick(fNetworkData.FindString("name"), curNick.String());
						fNetworkData.AddString("ignore", curNick.String());
					}
					curNick = "";
				}
				// catch last one
				if (cmd.Length() > 0)
				{
					for (i = 0; i < attrCount; i++)
						if (cmd.ICompare(fNetworkData.FindString("ignore", i)) == 0)
							break;
					// no dupes found, add it
					if (i == fNotifyNicks.CountItems())
					{
						vision_app->AddIgnoreNick(fNetworkData.FindString("name"), curNick.String());
						fNetworkData.AddString("ignore", curNick.String());
					}
				}
			}
			break;

		case M_IGNORE_REMOVE:
			{
			}
			break;

		case M_EXCLUDE_ADD:
			{
			}
			break;

		case M_EXCLUDE_REMOVE:
			{
			}
			break;

		case M_NOTIFYLIST_ADD:
			{
				BString cmd (msg->FindString("cmd"));
				BString curNick;
				int32 idx (-1);
				int32 i (0);

				// TODO: print notification message to user
				while ((idx = cmd.IFindFirst(" ")) > 0)
				{
					cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
					// remove leading space
					cmd.Remove(0, 1);
					for (i = 0; i < fNotifyNicks.CountItems(); i++)
						if (curNick.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0)
							break;
					// no dupes found, add it
					if (i == fNotifyNicks.CountItems())
					{
						vision_app->AddNotifyNick(fNetworkData.FindString("name"), curNick.String());
						NotifyListItem *item (new NotifyListItem (curNick.String(), false));
						fNotifyNicks.AddItem (item);
					}
					curNick = "";
				}
				// catch last one
				if (cmd.Length() > 0)
				{
					for (i = 0; i < fNotifyNicks.CountItems(); i++)
						if (cmd == fNotifyNicks.ItemAt(i)->Text())
							break;
					// no dupes found, add it
					if (i == fNotifyNicks.CountItems())
					{
						NotifyListItem *item (new NotifyListItem (cmd.String(), false));
						vision_app->AddNotifyNick(fNetworkData.FindString("name"), cmd.String());
						fNotifyNicks.AddItem (item);
					}
				}

				fNotifyNicks.SortItems(SortNotifyItems);
				BMessage updMsg (M_NOTIFYLIST_UPDATE);
				updMsg.AddPointer ("list", &fNotifyNicks);
				updMsg.AddPointer ("source", this);
				Window()->PostMessage (&updMsg);
			}
			break;

		case M_NOTIFYLIST_REMOVE:
			{
				BString cmd (msg->FindString("cmd"));
				BString curNick;
				int32 idx (-1);

				// TODO: print notification message to user
				while ((idx = cmd.IFindFirst(" ")) > 0)
				{
					cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
					// remove leading space
					cmd.Remove(0, 1);
					vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), curNick.String());
					for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
						if (curNick.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0)
						{
							delete fNotifyNicks.RemoveItemAt(i);
						}
					curNick = "";
				}
				// catch last one
				if (cmd.Length() > 0)
				{
					vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), cmd.String());
					for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
						if (cmd.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0)
						{
							delete fNotifyNicks.RemoveItemAt(i);
						}
				}
				BMessage updMsg (M_NOTIFYLIST_UPDATE);
				updMsg.AddPointer ("list", &fNotifyNicks);
				updMsg.AddPointer ("source", this);
				Window()->PostMessage (&updMsg);

			}
			break;

		case M_NOTIFYLIST_UPDATE:
			{
				// force agent to update (used for winlist switches)
				BMessage newMsg (M_NOTIFYLIST_UPDATE);
				newMsg.AddPointer("list", &fNotifyNicks);
				newMsg.AddPointer("source", this);
				Window()->PostMessage(&newMsg);
			}
			break;
#if 0
		case M_IGNORE_ADD:
			{
				BString cmd (msg->FindString("cmd"));
				for (int32 i = 0; i < fIgnoreNicks.CountItems(); i++)
				{
					if (cmd == *((BString *)fIgnoreNicks.ItemAt(i)))
						break;
				}
				fIgnoreNicks.AddItem (new BString(cmd));
			}
			break;
#endif
		default:
			ClientAgent::MessageReceived (msg);
	}
}

