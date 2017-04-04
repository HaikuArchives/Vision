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
 *                 John Robinson
 *                 Alan Ellis <alan@cgsoftware.org>
 *                 Francois Revol
 */

#include <UTF8.h>
#include <Autolock.h>
#include <Directory.h>
#include <Entry.h>
#include <FilePanel.h>
#include <MessageRunner.h>
#include <Path.h>
#include <String.h>
#include <Socket.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "ClientAgent.h"
#include "ClientAgentLogger.h"
#include "ClientWindow.h"
#include "ChannelAgent.h"
#include "DCCConnect.h"
#include "ListAgent.h"
#include "MessageAgent.h"
#include "NotifyList.h"
#include "ServerAgent.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Vision.h"
#include "WindowList.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ServerAgent"

class failToLock
{/* exception in Establish */
};

int32 ServerAgent::fServerSeed = 0;

ServerAgent::ServerAgent(const char* id_, BMessage& net, BRect frame_)
	: ClientAgent(id_, id_, net.FindString("nick"), frame_),
	  fLocalip(""),
	  fMyNick(net.FindString("nick")),
	  fMyLag((net.FindBool("lagCheck")) ? "0.000" : B_TRANSLATE("Disabled")),
	  fLname(net.FindString("realname")),
	  fLident(net.FindString("ident")),
	  fLocalip_private(false),
	  fGetLocalIP(false),
	  fIsConnected(false),
	  fIsConnecting(true),
	  fReconnecting(false),
	  fIsQuitting(false),
	  fCheckingLag(false),
	  fReacquiredNick(true),
	  fEstablishHasLock(false),
	  fRetry(0),
	  fRetryLimit(47),
	  fLagCheck(0),
	  fLagCount(0),
	  fNickAttempt(0),
	  fSocket(NULL),
	  fLoginThread(-1),
	  fSenderThread(-1),
	  fEvents(vision_app->fEvents),
	  fServerHostName(id_),
	  fInitialMotd(true),
	  fCmds(net.FindString("autoexec")),
	  fListAgent(NULL),
	  fNetworkData(net),
	  fServerIndex(0),
	  fNickIndex(0),
	  fLogger(NULL)
{
	fLogger = new ClientAgentLogger(fId);
}

ServerAgent::~ServerAgent(void)
{
	if (fLagRunner)
		delete fLagRunner;

	while (fStartupChannels.CountItems() != 0)
		delete fStartupChannels.RemoveItemAt(0L);

	delete fLogger;

	delete_sem(fSendSyncSem);

	//  wait_for_thread (fLoginThread, &result);

	/*
	  while (fIgnoreNicks.CountItems() > 0)
		delete fIgnoreNicks.RemoveItem(0L);
	*/
	while (fNotifyNicks.CountItems() > 0)
		delete fNotifyNicks.RemoveItemAt(0L);
}

void ServerAgent::AttachedToWindow(void)
{
	Init();
	ClientAgent::AttachedToWindow();
}

void ServerAgent::AllAttached(void)
{
	fSMsgr = BMessenger(this);
	ClientAgent::AllAttached();

	// build notify list here since we can't send a message to ourselves sooner

	type_code type;
	int32 attrCount(0);

	BMessage updatemsg(M_NOTIFYLIST_ADD);
	BString data;

	fNetworkData.GetInfo("notify", &type, &attrCount);

	for (int32 i = 0; i < attrCount; i++) {
		data += fNetworkData.FindString("notify", i);
		data += " ";
	}
	if (attrCount > 0) {
		updatemsg.AddString("cmd", data.String());
		fSMsgr.SendMessage(&updatemsg);
	}
}

// do nothing for now
void ServerAgent::AddMenuItems(BPopUpMenu*)
{
}

void ServerAgent::Init(void)
{
	BString revString;
	Display("Vision ");
	vision_app->VisionVersion(VERSION_VERSION, revString);
	Display(revString.String(), C_MYNICK);
	Display(" built on ");
	vision_app->VisionVersion(VERSION_DATE, revString);
	Display(revString.String());
	Display("\nThis agent goes by the name of Smith... err ");
	BString temp;
	temp << fId << "\n";
	Display(temp.String(), C_NICK);
	Display("Have fun!\n");

	fLagRunner = new BMessageRunner(this, // target ServerAgent
									new BMessage(M_LAG_CHECK),
									10000000, // 10 seconds
									-1);	  // forever

	CreateSenderThread();
	CreateEstablishThread();
}

void ServerAgent::CreateSenderThread(void)
{
	fPendingSends = new BObjectList<BString>();
	fSendLock = new BLocker();
	fSendSyncSem = create_sem(0, "VisionSendSync");

	BString name;

	name = "t>";

	name += (rand() % 2) ? "Tima" : "Kenichi";

	fSenderThread = spawn_thread(Sender, name.String(), B_NORMAL_PRIORITY, this);

	if (fSenderThread >= B_OK)
		resume_thread(fSenderThread);
	else {
		printf("ERROR: could not create transmitter thread, aborting\n");
		delete_sem(fSendSyncSem);
	}
}

void ServerAgent::CreateEstablishThread(void)
{
	BString name;

	vision_app->GetThreadName(THREAD_S, name);

	fLoginThread = spawn_thread(Establish, name.String(), B_NORMAL_PRIORITY, new BMessenger(this));

	if (fLoginThread >= B_OK)
		resume_thread(fLoginThread);
	else {
		printf("ERROR: could not create login/establish thread, aborting\n");
	}
}

int ServerAgent::IRCDType(void)
{
	return fIrcdtype;
}

status_t ServerAgent::NewTimer(const char*, int32, int32)
{
	// TODO: implement this once scripting is ready
	return B_OK;
}

int32 ServerAgent::Timer(void* arg)
{
	BMessage* msg(reinterpret_cast<BMessage*>(arg));
	ServerAgent* agent;
	const char* cmd;
	int32 sleeptimer, loops;

	if ((msg->FindString("command", &cmd) != B_OK) || (msg->FindInt32("loops", &loops) != B_OK) ||
		(msg->FindInt32("sleep", &sleeptimer) != B_OK) ||
		(msg->FindPointer("agent", reinterpret_cast<void**>(&agent)) != B_OK)) {
		printf(":ERROR: couldn't find valid data in BMsg to Timer() -- bailing\n");
		return B_ERROR;
	}

	return B_OK;
}

int32 ServerAgent::Sender(void* arg)
{
	ServerAgent* agent(reinterpret_cast<ServerAgent*>(arg));
	BMessenger msgr(agent);
	sem_id sendSyncLock(-1);
	BLocker* sendDataLock(NULL);
	BObjectList<BString>* pendingSends(NULL);

	BMessage reply;
	if (msgr.IsValid() && (msgr.SendMessage(M_GET_SENDER_DATA, &reply) == B_OK)) {
		sendSyncLock = reply.FindInt32("sendSyncLock");
		reply.FindPointer("sendDataLock", reinterpret_cast<void**>(&sendDataLock));
		reply.FindPointer("pendingSends", reinterpret_cast<void**>(&pendingSends));
	} else
		return B_ERROR;

	BString* data(NULL);
	while (acquire_sem(sendSyncLock) == B_NO_ERROR) {
		sendDataLock->Lock();
		if (!pendingSends->IsEmpty()) {
			data = pendingSends->RemoveItemAt(0);
			sendDataLock->Unlock();
			agent->AsyncSendData(data->String());
			delete data;
			data = NULL;
		} else
			sendDataLock->Unlock();
	}

	// sender takes possession of pending sends and sendDataLock structures
	// allows for self-contained cleanups
	while (pendingSends->CountItems() > 0) delete pendingSends->RemoveItemAt(0L);
	delete pendingSends;
	delete sendDataLock;

	return B_OK;
}

int32 ServerAgent::Establish(void* arg)
{
	BMessenger* sMsgrE(reinterpret_cast<BMessenger*>(arg));
	AutoDestructor<BMessenger> msgrKiller(sMsgrE);
	BMessage getMsg;
	BString remoteIP;
	int32 serverSid;
	BSocket* serverSock = NULL;
	if (!(sMsgrE->IsValid() && (sMsgrE->SendMessage(M_GET_ESTABLISH_DATA, &getMsg) == B_OK))) {
		printf(":ERROR: sMsgr not valid in Establish() -- bailing\n");
		return B_ERROR;
	}

	BMessage statMsg(M_DISPLAY);
	BString statString;

	try {
		BMessage reply;
		BString connectId, connectPort, ident, name, connectNick;
		const ServerData* serverData(NULL);
		ssize_t size;

		getMsg.FindData("server", B_ANY_TYPE, reinterpret_cast<const void**>(&serverData), &size);
		// better safe than sorry, seems under certain circumstances the SendMessage can fail
		// "silently"
		if (serverData == NULL)
			throw failToLock();
		connectId = serverData->serverName;
		connectPort << serverData->port;
		getMsg.FindString("ident", &ident);
		getMsg.FindString("name", &name);
		getMsg.FindString("nick", &connectNick);
		serverSid = getMsg.FindInt32("sid");

		if (sMsgrE->SendMessage(M_GET_RECONNECT_STATUS, &reply) == B_OK) {
			int retrycount(reply.FindInt32("retries"));

			if (retrycount) {
				statString = B_TRANSLATE("[@] Waiting ");
				statString << (retrycount * retrycount);
				statString += B_TRANSLATE(" second");
				if (retrycount > 1) statString += B_TRANSLATE("s");
				statString += B_TRANSLATE(" before next attempt" B_UTF8_ELLIPSIS "\n");
				ClientAgent::PackDisplay(&statMsg, statString.String(), C_ERROR);
				sMsgrE->SendMessage(&statMsg);
				snooze(1000000 * retrycount * retrycount); // wait 1, 4, 9, 16 ... seconds
			}

			if (sMsgrE->SendMessage(M_INC_RECONNECT) != B_OK)
				throw failToLock();
			statString = B_TRANSLATE("[@] Attempting to ");
			if (retrycount != 0)
				statString += B_TRANSLATE("re");
			statString += B_TRANSLATE("connect (attempt ");
			statString << retrycount + 1;
			statString += B_TRANSLATE(" of ");
			statString << reply.FindInt32("max_retries");
			statString += ")\n";
			ClientAgent::PackDisplay(&statMsg, statString.String(), C_ERROR);
			sMsgrE->SendMessage(&statMsg);
		} else
			throw failToLock();

		statString = B_TRANSLATE("[@] Attempting a connection to ");
		statString << connectId;
		statString += ":";
		statString << connectPort;
		statString += B_UTF8_ELLIPSIS "\n";
		ClientAgent::PackDisplay(&statMsg, statString.String(), C_ERROR);
		sMsgrE->SendMessage(&statMsg);

		BNetworkAddress remoteAddr(AF_INET, connectId.String(), connectPort);
		if (remoteAddr.InitCheck() != B_OK) {
			ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Could not create connection to address and port. Make sure your internet connection is operational.\n"), C_ERROR);
			sMsgrE->SendMessage(&statMsg);
			sMsgrE->SendMessage(M_NOT_CONNECTING);
			sMsgrE->SendMessage(M_SERVER_DISCONNECT);
			throw failToLock();
		}

		remoteIP = remoteAddr.ToString(false);
		vision_app->AddIdent(remoteIP.String(), ident.String());

		if ((serverSock = new BSocket) == NULL) {
			ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Could not create connection to address and port. Make sure your internet connection is operational.\n"), C_ERROR);
			sMsgrE->SendMessage(&statMsg);
			sMsgrE->SendMessage(M_NOT_CONNECTING);
			sMsgrE->SendMessage(M_SERVER_DISCONNECT);
			throw failToLock();
		}

		// just see if he's still hanging around before
		// we got blocked for a minute
		ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Connection open, waiting for reply from server\n"), C_ERROR);
		sMsgrE->SendMessage(&statMsg);
		sMsgrE->SendMessage(M_LAG_CHANGED);

		if (serverSock->Connect(remoteAddr) == B_OK) {
			BString ip(serverSock->Local().ToString(false));

			// store local ip address for future use (dcc, etc)
			if (ip.IsEmpty()) {
				ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Error getting local IP\n"), C_ERROR);
				sMsgrE->SendMessage(&statMsg);
				BMessage setIP(M_SET_IP);
				setIP.AddString("ip", "127.0.0.1");
				setIP.AddBool("private", PrivateIPCheck("127.0.0.1"));
				sMsgrE->SendMessage(&setIP);
			} else {
				BMessage setIP(M_SET_IP);
				if (vision_app->GetBool("dccPrivateCheck")) {
					setIP.AddBool("private", PrivateIPCheck(ip.String()));
				} else {
					setIP.AddBool("private", false);
				}
				setIP.AddString("ip", ip.String());
				sMsgrE->SendMessage(&setIP);
				statString = B_TRANSLATE("[@] Local IP: ");
				statString += ip.String();
				statString += "\n";
				ClientAgent::PackDisplay(&statMsg, statString.String(), C_ERROR);
				sMsgrE->SendMessage(&statMsg);
			}

			if (vision_app->GetBool("dccPrivateCheck") && PrivateIPCheck(ip.String())) {
				ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] (It looks like you are behind an internet gateway. Vision will query the IRC server upon successful connection for your gateway's internet address. This will be used for DCC communication.)\n"), C_ERROR);
				sMsgrE->SendMessage(&statMsg);
			}

			ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Handshaking\n"), C_ERROR);
			sMsgrE->SendMessage(&statMsg);

			BString string;
			BMessage dataSend(M_SERVER_SEND);
			dataSend.AddString("data", "blah");

			BMessage endpointMsg(M_SET_ENDPOINT);
			endpointMsg.AddPointer("socket", serverSock);
			if (sMsgrE->SendMessage(&endpointMsg, &reply) != B_OK) throw failToLock();

			if (strlen(serverData->password) > 0) {
				ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Sending password\n"), C_ERROR);
				sMsgrE->SendMessage(&statMsg);
				string = "PASS ";
				string += serverData->password;
				dataSend.ReplaceString("data", string.String());
				sMsgrE->SendMessage(&dataSend);
			}

			string = "NICK ";
			string.Append(connectNick);

			dataSend.ReplaceString("data", string.String());
			if (sMsgrE->SendMessage(&dataSend) != B_OK) throw failToLock();

			string = "USER ";
			string.Append(ident);
			string.Append(" localhost ");
			string.Append(connectId);
			string.Append(" :");
			string.Append(name);

			dataSend.ReplaceString("data", string.String());
			if (sMsgrE->SendMessage(&dataSend) != B_OK) throw failToLock();

			// resume normal business matters.

			ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Established\n"), C_ERROR);
			sMsgrE->SendMessage(&statMsg);
		} else // No endpoint->connect
		{
			ClientAgent::PackDisplay(&statMsg, B_TRANSLATE("[@] Could not establish a connection to the server. Sorry.\n"), C_ERROR);
			sMsgrE->SendMessage(&statMsg);
			sMsgrE->SendMessage(M_NOT_CONNECTING);
			sMsgrE->SendMessage(M_SERVER_DISCONNECT);
			throw failToLock();
		}
	} catch (failToLock) {
		vision_app->RemoveIdent(remoteIP.String());
		return B_ERROR;
	}

	BString buffer;
	while (sMsgrE->IsValid()) {
		char indata[1024];
		int32 length(0);

		memset(indata, 0, 1024);
		if (serverSock != NULL && serverSock->IsConnected()) {
			if ((length = serverSock->Read(indata, 1023)) > 0) {
				BString temp;
				int32 index;

				temp.SetTo(indata, strlen(indata));
				buffer += temp;

				while ((index = buffer.FindFirst('\n')) != B_ERROR) {
					temp.SetTo(buffer, index);
					buffer.Remove(0, index + 1);

					temp.RemoveLast("\r");

					if (vision_app->fDebugRecv) {
						printf("RECEIVED: (%03" B_PRId32 ":%03" B_PRId32 ") \"", serverSid,
							   temp.Length());
						for (int32 i = 0; i < temp.Length(); ++i) {
							if (isprint(temp[i]))
								printf("%c", temp.String()[i]);
							else
								printf("[0x%02x]", temp.String()[i]);
						}
						printf("\"\n");
					}

					// We ship it off this way because
					// we want this thread to loop relatively
					// quickly.  Let ServerWindow's main thread
					// handle the processing of incoming data!
					BMessage msg(M_PARSE_LINE);
					msg.AddString("line", temp.String());
					sMsgrE->SendMessage(&msg);
				}
			}
			if (!serverSock->IsConnected()) {
				// we got disconnected :(

				if (vision_app->fDebugRecv) {
					// print interesting info
					printf("Negative from endpoint receive! (%" B_PRId32 ")\n", length);
				}
				// tell the user all about it
				sMsgrE->SendMessage(M_NOT_CONNECTING);
				sMsgrE->SendMessage(M_SERVER_DISCONNECT);
				break;
			}
		}

		// take a nap, so the ServerAgent can do things
		snooze(20000);
	}
	vision_app->RemoveIdent(remoteIP.String());

	return B_OK;
}

int ServerAgent::SortNotifyItems(const NotifyListItem* item1, const NotifyListItem* item2)
{
	if (!item1 || !item2) return 0;

	if (item1->GetState() && !item2->GetState()) return -1;
	if (!item1->GetState() && item2->GetState()) return 1;

	BString name(item1->Text());

	return name.ICompare(item2->Text());
}

void ServerAgent::AsyncSendData(const char* cData)
{
	int32 length(0);
	if (!cData) return;

	BString data(cData);

	data.Append("\r\n");
	length = data.Length();

	memset(fSend_buffer, 0, sizeof(fSend_buffer));

	int32 dest_length(sizeof(fSend_buffer)), state(0);
	int32 encoding = vision_app->GetInt32("encoding");
	if (encoding != B_UNICODE_CONVERSION) {
		convert_from_utf8(encoding, data.String(), &length, fSend_buffer, &dest_length, &state);
	} else {
		strlcpy(fSend_buffer, data.String(), dest_length);
		dest_length = strlen(fSend_buffer);
	}
	if (fSocket == NULL || !fSocket->IsConnected()) {
		// doh, we aren't even connected.
		return;
	}

	if ((length = fSocket->Write(fSend_buffer, dest_length) < 0)) {
		if (!fReconnecting && !fIsConnecting)
			fMsgr.SendMessage(M_SERVER_DISCONNECT);
		// doh, we aren't even connected.
	}

	if (vision_app->fDebugSend) {
		data.RemoveAll("\n");
		data.RemoveAll("\r");
		printf("    SENT: (%03" B_PRId32 ") \"%s\"\n", length, data.String());
	}
}

void ServerAgent::SendData(const char* cData)
{
	// this function simply queues up the data into the requests buffer, then releases
	// the sender thread to take care of it
	BString* data(new BString(cData));
	fSendLock->Lock();
	fPendingSends->AddItem(data);
	fSendLock->Unlock();
	release_sem(fSendSyncSem);
}

void ServerAgent::ParseLine(const char* cData)
{
	BString data = FilterCrap(cData);
	int32 length(data.Length()), destLength(2048), state(0);

	memset(fParse_buffer, 0, sizeof(fParse_buffer));
	int32 encoding = vision_app->GetInt32("encoding");
	if (encoding != B_UNICODE_CONVERSION) {
		convert_to_utf8(encoding, data.String(), &length, fParse_buffer, &destLength, &state);
	} else {
		if (IsValidUTF8(data.String(), destLength)) {
			strlcpy(fParse_buffer, data.String(), destLength);
			destLength = strlen(fParse_buffer);
		} else {
			convert_to_utf8(B_ISO1_CONVERSION, data.String(), &length, fParse_buffer, &destLength,
							&state);
		}
	}
	if (vision_app->fNumBench) {
		vision_app->fBench1 = system_time();
		if (ParseEvents(fParse_buffer)) {
			vision_app->fBench2 = system_time();
			BString bencht(GetWord(data.String(), 2));
			vision_app->BenchOut(bencht.String());
			return;
		}
	} else {
		if (ParseEvents(fParse_buffer)) return;
	}

	data.Append("\n");
	Display(data.String(), 0);
}

ClientAgent* ServerAgent::Client(const char* cName)
{
	ClientAgent* client(0);

	for (int32 i = 0; i < fClients.CountItems(); ++i) {
		ClientAgent* item(fClients.ItemAt(i));
		if (strcasecmp(cName, item->Id().String()) == 0) {

			client = item;
			break;
		}
	}
	return client;
}

ClientAgent* ServerAgent::ActiveClient(void)
{
	ClientAgent* client(NULL);
	//  printf("finding active client\n");

	for (int32 i = 0; i < fClients.CountItems(); i++) {
		//    printf("checking against client: %d, %s\n", i, fClients.ItemAt(i)->fId.String());
		if (!fClients.ItemAt(i)->IsHidden()) {
			//      printf("not hidden, break\n");
			client = fClients.ItemAt(i);
			break;
		}
	}
	return client;
}

void ServerAgent::Broadcast(BMessage* msg, bool sendToServer)
{
	for (int32 i = 0; i < fClients.CountItems(); ++i) {
		ClientAgent* client(fClients.ItemAt(i));

		if (client != this) client->fMsgr.SendMessage(msg);
	}
	if (sendToServer) {
		fSMsgr.SendMessage(msg);
	}
}

void ServerAgent::RepliedBroadcast(BMessage*)
{
	//  TODO: implement this
	//  BMessage cMsg (*msg);
	//  BAutolock lock (this);
	//
	//  for (int32 i = 0; i < fClients.CountItems(); ++i)
	//  {
	//    ClientAgent *client ((ClientAgent *)fClients.ItemAt (i));
	//
	//    if (client != this)
	//    {
	//      BMessenger fMsgr (client);
	//      BMessage reply;
	//      fMsgr.SendMessage (&cMsg, &reply);
	//    }
	//  }
}

void ServerAgent::DisplayAll(const char* buffer, const uint32 fore, const uint32 back,
							 const uint32 font)
{
	for (int32 i = 0; i < fClients.CountItems(); ++i) {
		ClientAgent* client(fClients.ItemAt(i));

		BMessage msg(M_DISPLAY);
		PackDisplay(&msg, buffer, fore, back, font);
		client->fMsgr.SendMessage(&msg);
	}

	return;
}

void ServerAgent::PostActive(BMessage* msg)
{
	BAutolock activeLock(Window());
	//  printf("postActive\n");
	ClientAgent* client(ActiveClient());
	//  printf("posting to: %p\n", client);
	if (client != NULL)
		client->fMsgr.SendMessage(msg);
	else
		fSMsgr.SendMessage(msg);
}

void ServerAgent::HandleReconnect(void)
{
	/*
	 * Function purpose: Setup the environment and attempt a new connection
	 * to the server
	 */

	if (fIsConnected) {
		// what's going on here?!
		printf(
			":ERROR: HandleReconnect() called when we're already connected! Whats up with that?!");
		return;
	}

	delete_sem(fSendSyncSem);

	CreateSenderThread();

	if (fRetry < fRetryLimit) {
		// we are go for main engine start
		fReconnecting = true;
		fIsConnecting = true;
		fNickAttempt = 0;
		fEstablishHasLock = false;
		CreateEstablishThread();
	} else {
		// we've hit our fRetry limit. throw in the towel
		fReconnecting = false;
		fRetry = 0;
		const char* soSorry;
		soSorry = B_TRANSLATE("[@] Retry limit reached; giving up. Type /reconnect if you want to give it another go.\n");
		Display(soSorry, C_ERROR);
		ClientAgent* agent(ActiveClient());
		if (agent && (agent != this)) agent->Display(soSorry, C_ERROR, C_BACKGROUND, F_SERVER);
	}
}

const ServerData* ServerAgent::GetNextServer()
{
	type_code type;
	int32 count;
	ssize_t size;
	fNetworkData.GetInfo("server", &type, &count);
	uint32 state = (fReconnecting) ? SERVER_SECONDARY : SERVER_PRIMARY;

	for (;;) {
		if (fServerIndex >= count) {
			fServerIndex = 0;
			state = SERVER_PRIMARY;
		}

		const ServerData* server(NULL);
		for (; fNetworkData.FindData("server", B_RAW_TYPE, fServerIndex,
									 reinterpret_cast<const void**>(&server), &size) == B_OK;
			 fServerIndex++)
			if (server->state == state) {
				memset(&fCurrentServer, 0, sizeof(ServerData));
				memcpy(&fCurrentServer, server, size);
				return &fCurrentServer;
			}
	}
}

const char* ServerAgent::GetNextNick()
{
	type_code type;
	int32 count;
	fNetworkData.GetInfo("nick", &type, &count);
	if (fNickIndex < count)
		return fNetworkData.FindString("nick", fNickIndex++);
	else {
		fNickIndex = 0;
		return "";
	}
}

bool ServerAgent::PrivateIPCheck(BString ip)
{
	/*
	 * Function purpose: Compare against fLocalip to see if it is a private address
	 *                   if so, set fLocalip_private to true;
	 *
	 * Private ranges: 10.0.0.0    - 10.255.255.255
	 *                 172.16.0.0  - 172.31.255.255
	 *                 192.168.0.0 - 192.168.255.255
	 *                 (as defined in RFC 1918)
	 */

	if (ip == NULL || ip == "") {
		// it is obviously a mistake we got called.
		// setup some sane values and print an assertion
		printf(":ERROR: PrivateIPCheck() called when there is no valid data to check!\n");
		return true;
	}

	if (ip == "127.0.0.1") return true;

	// catch 10.0.0.0 - 10.255.255.255 and 192.168.0.0 - 192.168.255.255
	if ((strncmp(ip, "10.", 3) == 0) || (strncmp(ip, "192.168.", 8) == 0)) return true;

	// catch 172.16.0.0 - 172.31.255.255
	if (strncmp(ip, "172.", 4) == 0) {
		// check to see if characters 5-6 are (or are between) 16 and 31
		{
			char temp172s[3];
			temp172s[0] = ip[4];
			temp172s[1] = ip[5];
			temp172s[2] = '\0';
			int temp172n(atoi(temp172s));

			if (temp172n >= 16 || temp172n <= 31) return true;
		}
		return false;
	}

	// if we got this far, its a public IP address
	return false;
}

void ServerAgent::AddResumeData(BMessage* msg)
{
	ResumeData* data;

	data = new ResumeData;

	data->expire = system_time() + 50000000LL;
	data->nick = msg->FindString("vision:nick");
	data->file = msg->FindString("vision:file");
	data->size = msg->FindString("vision:size");
	data->ip = msg->FindString("vision:ip");
	data->port = msg->FindString("vision:port");
	data->path = msg->FindString("path");
	data->pos = msg->FindInt64("pos");

	// PRINT(("%s %s %s %s %s", data->nick.String(), data->file.String(),
	//	data->size.String(), data->ip.String(), data->port.String()));
	fResumes.AddItem(data);

	BString buffer;

	buffer << "PRIVMSG " << data->nick << " :\1DCC RESUME " << data->file << " " << data->port
		   << " " << data->pos << "\1";

	SendData(buffer.String());
}

void ServerAgent::ParseAutoexecChans(const BString& origLine)
{
	BString line(origLine);
	int32 chanIndex(0);
	if ((chanIndex = line.IFindFirst("/JOIN")) != B_ERROR) {
		chanIndex += 6;
		line.Remove(0, chanIndex);
	} else if ((chanIndex = line.IFindFirst("/J")) != B_ERROR) {
		chanIndex += 3;
		line.Remove(0, chanIndex);
	} else
		return;
	// parse out all autoexec channels to ensure we don't try to focus those
	// on join
	chanIndex = 0;
	BString* newChan(NULL);
	for (;;) {
		if ((chanIndex = line.FindFirst(',')) == B_ERROR) break;
		newChan = new BString();
		line.CopyInto(*newChan, 0, chanIndex);
		if ((*newChan)[0] != '#') newChan->Prepend("#");
		fStartupChannels.AddItem(newChan);
		line.Remove(0, chanIndex + 1);
	}
	newChan = new BString();
	// catch last channel (or only channel if no comma separations)
	if ((chanIndex = line.FindFirst(' ')) != B_ERROR)
		line.CopyInto(*newChan, 0, chanIndex);
	else
		*newChan = line;
	if ((*newChan)[0] != '#') newChan->Prepend("#");
	fStartupChannels.AddItem(newChan);
}

void ServerAgent::RemoveAutoexecChan(const BString& chan)
{
	int32 chanCount(fStartupChannels.CountItems());
	for (int32 i = 0; i < chanCount; i++)
		if (fStartupChannels.ItemAt(i)->ICompare(chan) == 0) {
			delete fStartupChannels.RemoveItemAt(i);
			return;
		}
}

void ServerAgent::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_PARSE_LINE: {
		const char* buffer(NULL);
		msg->FindString("line", &buffer);
		if (buffer) ParseLine(buffer);
	} break;

	case M_STATE_CHANGE: {
		Broadcast(msg);
		ClientAgent::MessageReceived(msg);
	} break;

	case M_SEND_RAW: {
		const char* buffer;
		msg->FindString("line", &buffer);
		if (buffer) SendData(buffer);
	} break;

	case M_DISPLAY_ALL: {
		BString data;
		msg->FindString("data", &data);
		DisplayAll(data.String(), msg->FindInt32("fore"), msg->FindInt32("back"),
				   msg->FindInt32("font"));
	} break;

	case M_GET_ESTABLISH_DATA: {
		BMessage reply(B_REPLY);
		reply.AddData("server", B_RAW_TYPE, GetNextServer(), sizeof(ServerData));
		reply.AddString("ident", fLident.String());
		reply.AddString("name", fLname.String());
		reply.AddString("nick", fMyNick.String());
		msg->SendReply(&reply);
		fEstablishHasLock = true;

	} break;

	case M_GET_SENDER_DATA: {
		BMessage reply;
		reply.AddInt32("sendSyncLock", fSendSyncSem);
		reply.AddPointer("sendDataLock", fSendLock);
		reply.AddPointer("pendingSends", fPendingSends);
		msg->SendReply(&reply);
	} break;

	case M_SET_ENDPOINT:
		msg->FindPointer("socket", (void**)&fSocket);
		break;

	case M_NOT_CONNECTING:
		fIsConnecting = false;
		fReconnecting = false;
		break;

	case M_CONNECTED:
		fIsConnected = true;
		fIsConnecting = false;
		break;

	case M_INC_RECONNECT:
		++fRetry;
		break;

	case M_SET_IP: {
		static BString ip;
		msg->FindString("ip", &ip);
		fLocalip = ip.String();
		fLocalip_private = msg->FindBool("private");
		fGetLocalIP = fLocalip_private;
	} break;

	case M_GET_IP: {
		BMessage reply;
		reply.AddBool("private", fLocalip_private);
		reply.AddString("ip", fLocalip);
		msg->SendReply(&reply);
	} break;

	case M_GET_RECONNECT_STATUS: {
		BMessage reply(B_REPLY);
		reply.AddInt32("retries", fRetry);
		reply.AddInt32("max_retries", fRetryLimit);
		msg->SendReply(&reply);
	} break;

	case M_SERVER_SEND: {
		BString buffer;
		int32 i;

		for (i = 0; msg->HasString("data", i); ++i) {
			const char* str;

			msg->FindString("data", i, &str);
			buffer << str;
		}

		SendData(buffer.String());
		if (msg->IsSourceWaiting()) msg->SendReply(B_REPLY);
	} break;

	case M_DCC_ACCEPT: {
		bool cont(false);
		const char* nick, *size, *ip, *port;
		BPath path;

		msg->FindString("vision:nick", &nick);
		msg->FindString("vision:size", &size);
		msg->FindString("vision:ip", &ip);
		msg->FindString("vision:port", &port);

		if (msg->HasString("path"))
			path.SetTo(msg->FindString("path"));
		else {
			const char* file;
			entry_ref ref;

			msg->FindRef("directory", &ref);
			msg->FindString("name", &file);

			BDirectory dir(&ref);
			path.SetTo(&dir, file);
		}

		if (msg->HasBool("continue")) msg->FindBool("continue", &cont);

		DCCReceive* view;
		view = new DCCReceive(nick, path.Path(), size, ip, port, fSMsgr, cont);

		BMessage aMsg(M_DCC_FILE_WIN);
		aMsg.AddPointer("view", view);
		be_app->PostMessage(&aMsg);
	} break;

	case M_CHOSE_FILE: // DCC send
	{
		const char* nick(NULL);
		entry_ref ref;
		off_t size;
		msg->FindString("nick", &nick);
		msg->FindRef("refs", &ref); // get file

		BEntry entry(&ref);

		/* // TODO: resolve if symlink

				if (entry.IsSymLink())
				{
				  BSymLink link (&entry);
				}
		*/
		BPath path(&entry);
		// PRINT(("file path: %s\n", path.Path()));
		entry.GetSize(&size);

		BString ssize;
		ssize << size;

		DCCSend* view;

		view = new DCCSend(nick, path.Path(), ssize.String(), fSMsgr);
		BMessage message(M_DCC_FILE_WIN);
		message.AddPointer("view", view);
		vision_app->PostMessage(&message);

	} break;

	case M_ADD_RESUME_DATA: {
		AddResumeData(msg);
	} break;

	case B_CANCEL:
		if (msg->HasPointer("source")) {
			BFilePanel* fPanel;
			msg->FindPointer("source", reinterpret_cast<void**>(&fPanel));
			delete fPanel;
		}
		break;

	case M_CHAT_ACCEPT: {
		int32 acceptDeny;
		BString theNick;
		const char* theIP, *thePort;
		msg->FindInt32("which", &acceptDeny);
		if (acceptDeny) return;
		msg->FindString("nick", &theNick);
		msg->FindString("ip", &theIP);
		msg->FindString("port", &thePort);

		theNick.Append(" [DCC]");
		MessageAgent* newAgent(new MessageAgent(*vision_app->pClientWin()->AgentRect(),
												theNick.String(), fId.String(), fSMsgr,
												fMyNick.String(), "", true, false, theIP, thePort));
		vision_app->pClientWin()->pWindowList()->AddAgent(newAgent, theNick.String(),
														  WIN_MESSAGE_TYPE, true);

		fClients.AddItem(newAgent);
	} break;

	case M_CHAT_ACTION: // dcc chat
	{
		ClientAgent* client;
		const char* theNick;
		BString thePort;
		BString theId;

		msg->FindString("nick", &theNick);
		msg->FindString("port", &thePort);
		theId << theNick << " [DCC]";

		if ((client = Client(theId.String())) == 0) {
			MessageAgent* newAgent(new MessageAgent(
				*vision_app->pClientWin()->AgentRect(), theId.String(), fId.String(), fSMsgr,
				fMyNick.String(), "", true, true, "", thePort != "" ? thePort.String() : ""));
			vision_app->pClientWin()->pWindowList()->AddAgent(newAgent, theId.String(),
															  WIN_MESSAGE_TYPE, true);

			fClients.AddItem(newAgent);
		}
	} break;

	case M_SLASH_RECONNECT:
		if (!fIsConnected && !fIsConnecting) fMsgr.SendMessage(M_SERVER_DISCONNECT);
		break;

	case M_SERVER_DISCONNECT: {
		if (fIsQuitting)
			break;

		// store current nick for reconnect use (might be an away nick, etc)
		if (fReacquiredNick) {
			fReacquiredNick = false;
			fReconNick = fMyNick;
		}
		// let the user know
		if (fIsConnected) {
			fIsConnected = false;
			BString sAnnounce;
			sAnnounce += B_TRANSLATE("[@] Disconnected from ");
			sAnnounce += fServerName;
			sAnnounce += "\n";
			Display(sAnnounce.String(), C_ERROR);
			ClientAgent* agent(ActiveClient());
			if (agent && (agent != this))
				agent->Display(sAnnounce.String(), C_ERROR, C_BACKGROUND, F_SERVER);
		}

		// let other agents know about it
		Broadcast(msg);

		fMyLag = B_TRANSLATE("Disconnected");
		fMsgr.SendMessage(M_LAG_CHANGED);
		fCheckingLag = false;
		if (fSocket != NULL)
			delete fSocket;
		fSocket = NULL;

		// attempt a reconnect
		if (!fIsConnecting) HandleReconnect();
	} break;

	case M_STATUS_ADDITEMS: {
		vision_app->pClientWin()->pStatusView()->AddItem(new StatusItem(0, ""), true);

		vision_app->pClientWin()->pStatusView()->AddItem(
			new StatusItem("Lag: ", "", STATUS_ALIGN_LEFT), true);

		vision_app->pClientWin()->pStatusView()->AddItem(new StatusItem(0, "", STATUS_ALIGN_LEFT),
														 true);

		// The false bool for SetItemValue() tells the StatusView not to Invalidate() the view.
		// We send true on the last SetItemValue().
		vision_app->pClientWin()->pStatusView()->SetItemValue(STATUS_SERVER, fId.String(), false);
		vision_app->pClientWin()->pStatusView()->SetItemValue(STATUS_LAG, fMyLag.String(), false);
		vision_app->pClientWin()->pStatusView()->SetItemValue(STATUS_NICK, fMyNick.String(), true);
	} break;

	case M_LAG_CHECK: {
		if (fIsConnected) {
			if (fNetworkData.FindBool("lagCheck")) {
				BMessage lagSend(M_SERVER_SEND);
				AddSend(&lagSend, "VISION_LAG_CHECK");
				AddSend(&lagSend, endl);
				if (!fCheckingLag) {
					fLagCheck = system_time();
					fLagCount = 1;
					fCheckingLag = true;
				} else {
					if (fLagCount > 4) {
						// we've waited 50 seconds
						// connection problems?
						fMyLag = B_TRANSLATE("CONNECTION PROBLEM");
						fMsgr.SendMessage(M_LAG_CHANGED);
					} else {
						// wait some more
						char lag[15] = "";
						sprintf(lag, "%" B_PRId32 "0.000+",
								fLagCount); // assuming a 10 second runner
						fMyLag = lag;
						++fLagCount;
						fMsgr.SendMessage(M_LAG_CHANGED);
					}
				}
			}
			if (fNotifyNicks.CountItems() > 0) {
				BString cmd("ISON ");
				for (int32 i = 0; i < fNotifyNicks.CountItems(); i++) {
					cmd += " ";
					cmd += fNotifyNicks.ItemAt(i)->Text();
				}
				BMessage dataSend(M_SERVER_SEND);
				dataSend.AddString("data", cmd.String());
				fSMsgr.SendMessage(&dataSend);
			}
		}
	} break;

	case M_LAG_CHANGED: {
		if (!IsHidden())
			vision_app->pClientWin()->pStatusView()->SetItemValue(STATUS_LAG, fMyLag.String(),
																  true);

		BMessage newmsg(M_LAG_CHANGED);
		newmsg.AddString("lag", fMyLag);
		Broadcast(&newmsg);
	} break;

	case M_REJOIN_ALL: {
		for (int32 i = 0; i < fClients.CountItems(); ++i) {
			ClientAgent* client(fClients.ItemAt(i));

			if (dynamic_cast<ChannelAgent*>(client)) {
				BMessage rejoinMsg(M_REJOIN);
				rejoinMsg.AddString("nickname", fMyNick.String());
				client->fMsgr.SendMessage(&rejoinMsg);
			}
		}
	} break;

	case M_OPEN_MSGAGENT: {
		ClientAgent* client;
		const char* theNick;

		msg->FindString("nick", &theNick);

		if (!(client = Client(theNick))) {
			MessageAgent* newAgent(new MessageAgent(*vision_app->pClientWin()->AgentRect(), theNick,
													fId.String(), fSMsgr, fMyNick.String(), ""));
			vision_app->pClientWin()->pWindowList()->AddAgent(newAgent, theNick, WIN_MESSAGE_TYPE,
															  true);

			client = newAgent;

			fClients.AddItem(newAgent);
		} else
			client->fAgentWinItem->ActivateItem();

		if (msg->HasMessage("msg")) {
			BMessage buffer;

			msg->FindMessage("msg", &buffer);
			client->fMsgr.SendMessage(&buffer);
		}
	} break;

	case M_CLIENT_QUIT: {
		bool shutingdown(false);

		if (msg->HasBool("vision:shutdown")) msg->FindBool("vision:shutdown", &shutingdown);

		if (msg->HasString("vision:quit")) {
			const char* quitstr;
			msg->FindString("vision:quit", &quitstr);
			fQuitMsg = quitstr;
		}

		if (!fIsQuitting && fIsConnected && fSocket != NULL) {
			if (fQuitMsg.Length() == 0) {
				const char* expansions[1];
				BString version;
				vision_app->VisionVersion(VERSION_VERSION, version);
				expansions[0] = version.String();
				fQuitMsg << "QUIT :" << ExpandKeyed(vision_app->GetCommand(CMD_QUIT).String(), "V",
													expansions);
			}
			SendData(fQuitMsg.String());
		}

		fIsQuitting = true;

		if (fClients.CountItems() > 0) {
			Broadcast(msg);
			BMessenger listMsgr(fListAgent);
			listMsgr.SendMessage(M_CLIENT_QUIT);
		} else
			ClientAgent::MessageReceived(msg);
	} break;

	case M_CLIENT_SHUTDOWN: {
		ClientAgent* deadagent;

		if (msg->FindPointer("agent", reinterpret_cast<void**>(&deadagent)) != B_OK) {
			printf(":ERROR: error getting valid pointer from M_CLIENT_SHUTDOWN -- bailing\n");
			break;
		}

		fClients.RemoveItem(deadagent);

		BMessage deathchant(M_OBITUARY);
		deathchant.AddPointer("agent", deadagent);
		deathchant.AddPointer("item", deadagent->fAgentWinItem);
		vision_app->pClientWin()->PostMessage(&deathchant);

		if (fIsQuitting && (fClients.CountItems() == 0) &&
			(dynamic_cast<ServerAgent*>(deadagent) != this)) {
			fSMsgr.SendMessage(M_CLIENT_QUIT);
		}

	} break;

	case M_LIST_COMMAND: {
		if (fListAgent) break;
		vision_app->pClientWin()->pWindowList()->AddAgent(
			(fListAgent = new ListAgent(*vision_app->pClientWin()->AgentRect(),
										fServerHostName.String(), new BMessenger(this))),
			"Channels", WIN_LIST_TYPE, true);
		// kind of a hack since Agent() returns a pointer of type ClientAgent, of which
		// ListAgent is not a subclass...
		BMessenger listMsgr(fListAgent);
		listMsgr.SendMessage(msg);
	} break;

	case M_LIST_SHUTDOWN:
		fListAgent = NULL;
		break;

	case M_REGISTER_LOGGER: {
		const char* logName;
		msg->FindString("name", &logName);
		fLogger->RegisterLogger(logName);
	} break;

	case M_UNREGISTER_LOGGER: {
		const char* logName;
		msg->FindString("name", &logName);
		fLogger->UnregisterLogger(logName);
	} break;

	case M_CLIENT_LOG: {
		const char* logName;
		const char* data;
		msg->FindString("name", &logName);
		msg->FindString("data", &data);
		fLogger->Log(logName, data);
	} break;

	case M_IGNORE_ADD: {
		BString cmd(msg->FindString("cmd"));
		BString curNick;
		int32 idx(-1);
		int32 i(0);

		type_code type;
		int32 attrCount;

		// make sure this nick hasn't already been added
		fNetworkData.GetInfo("ignore", &type, &attrCount);

		// TODO: print notification message to user
		while ((idx = cmd.IFindFirst(" ")) > 0) {
			cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
			// remove leading space
			cmd.Remove(0, 1);
			for (i = 0; i < attrCount; i++)
				if (curNick.ICompare(fNetworkData.FindString("ignore", i)) == 0) break;
			// no dupes found, add it
			if (i == attrCount) {
				vision_app->AddIgnoreNick(fNetworkData.FindString("name"), curNick.String());
				fNetworkData.AddString("ignore", curNick.String());
			}
			curNick = "";
		}
		// catch last one
		if (cmd.Length() > 0) {
			for (i = 0; i < attrCount; i++)
				if (cmd.ICompare(fNetworkData.FindString("ignore", i)) == 0) break;
			// no dupes found, add it
			if (i == fNotifyNicks.CountItems()) {
				vision_app->AddIgnoreNick(fNetworkData.FindString("name"), curNick.String());
				fNetworkData.AddString("ignore", curNick.String());
			}
		}
	} break;

	case M_IGNORE_REMOVE: {
	} break;

	case M_EXCLUDE_ADD: {
	} break;

	case M_EXCLUDE_REMOVE: {
	} break;

	case M_NOTIFYLIST_ADD: {
		BString cmd(msg->FindString("cmd"));
		BString curNick;
		int32 idx(-1);
		int32 i(0);

		// TODO: print notification message to user
		while ((idx = cmd.IFindFirst(" ")) > 0) {
			cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
			// remove leading space
			cmd.Remove(0, 1);
			for (i = 0; i < fNotifyNicks.CountItems(); i++)
				if (curNick.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0) break;
			// no dupes found, add it
			if (i == fNotifyNicks.CountItems()) {
				vision_app->AddNotifyNick(fNetworkData.FindString("name"), curNick.String());
				NotifyListItem* item(new NotifyListItem(curNick.String(), false));
				fNotifyNicks.AddItem(item);
			}
			curNick = "";
		}
		// catch last one
		if (cmd.Length() > 0) {
			for (i = 0; i < fNotifyNicks.CountItems(); i++)
				if (cmd == fNotifyNicks.ItemAt(i)->Text()) break;
			// no dupes found, add it
			if (i == fNotifyNicks.CountItems()) {
				NotifyListItem* item(new NotifyListItem(cmd.String(), false));
				vision_app->AddNotifyNick(fNetworkData.FindString("name"), cmd.String());
				fNotifyNicks.AddItem(item);
			}
		}

		fNotifyNicks.SortItems(SortNotifyItems);
		BMessage updMsg(M_NOTIFYLIST_UPDATE);
		updMsg.AddPointer("list", &fNotifyNicks);
		updMsg.AddPointer("source", this);
		Window()->PostMessage(&updMsg);
	} break;

	case M_NOTIFYLIST_REMOVE: {
		BString cmd(msg->FindString("cmd"));
		BString curNick;
		int32 idx(-1);

		// TODO: print notification message to user
		while ((idx = cmd.IFindFirst(" ")) > 0) {
			cmd.MoveInto(curNick, 0, cmd.IFindFirst(" "));
			// remove leading space
			cmd.Remove(0, 1);
			vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), curNick.String());
			for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
				if (curNick.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0) {
					delete fNotifyNicks.RemoveItemAt(i);
				}
			curNick = "";
		}
		// catch last one
		if (cmd.Length() > 0) {
			vision_app->RemoveNotifyNick(fNetworkData.FindString("name"), cmd.String());
			for (int32 i = 0; i < fNotifyNicks.CountItems(); i++)
				if (cmd.ICompare(fNotifyNicks.ItemAt(i)->Text()) == 0) {
					delete fNotifyNicks.RemoveItemAt(i);
				}
		}
		BMessage updMsg(M_NOTIFYLIST_UPDATE);
		updMsg.AddPointer("list", &fNotifyNicks);
		updMsg.AddPointer("source", this);
		Window()->PostMessage(&updMsg);

	} break;

	case M_NOTIFYLIST_UPDATE: {
		// force agent to update (used for winlist switches)
		BMessage newMsg(M_NOTIFYLIST_UPDATE);
		newMsg.AddPointer("list", &fNotifyNicks);
		newMsg.AddPointer("source", this);
		Window()->PostMessage(&newMsg);
	} break;
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
		ClientAgent::MessageReceived(msg);
	}
}
