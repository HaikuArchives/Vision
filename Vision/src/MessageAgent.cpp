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

#include <Beep.h>
#include <Catalog.h>
#include <Entry.h>
#include <MenuItem.h>
#ifdef __HAIKU__
#include <Notification.h>
#endif
#include <PopUpMenu.h>
#include <Roster.h>
#include <UTF8.h>

#include "ClientWindow.h"
#include "MessageAgent.h"
#include "NetworkManager.h"
#include "StatusView.h"
#include "Utilities.h"
#include "Vision.h"
#include "VTextControl.h"
#include "WindowList.h"

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>


#ifdef USE_INFOPOPPER
#include <infopopper/InfoPopper.h>
#endif

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DCCMessages"

MessageAgent::MessageAgent (
	BRect &frame_,
	const char *id_,
	const char *fServerName_,
	const BMessenger &fSMsgr_,
	const char *nick,
	const char *addyString,
	bool chat,
	bool initiate,
	const char *IP,
	const char *port)

	: ClientAgent (
		id_,
		fServerName_,
		nick,
		fSMsgr_,
		frame_),

	fChatAddy (addyString ? addyString : ""),
	fChatee (id_),
	fDIP (IP),
	fDPort (port),
	fDChat (chat),
	fDInitiate (initiate),
	fDConnected (false),
	fConnectionID (-1)
{
	Init();
}

MessageAgent::~MessageAgent (void)
{
	/*
	fDConnected = false;
	if (fDChat)
	{
		if (fMySocket)
			close (fMySocket);
		if (fAcceptSocket)
			close (fAcceptSocket);
	}
	*/
}

void
MessageAgent::AllAttached (void)
{
	if (fDChat)
	{
		if (fDInitiate)
		{
			BMessage msg(M_CREATE_LISTENER);
			msg.AddMessenger("target", this);
			BMessenger(network_manager).SendMessage(&msg);
			msg.AddString("port", fDPort);
		}
		else
		{
			BMessage msg(M_CREATE_CONNECTION);
			msg.AddMessenger("target", this);
			msg.AddString("port", fDPort);

			BMessenger(network_manager).SendMessage(&msg);		}
	}
	ClientAgent::AllAttached();
}

void
MessageAgent::AddMenuItems (BPopUpMenu *pMenu)
{
	BMenuItem *item (NULL);
	item = new BMenuItem("Whois", new BMessage (M_MSG_WHOIS));
	item->SetTarget (this);
	if (Id().FindFirst (" [DCC]") >= 0)	// dont enable for dcc sessions
			item->SetEnabled (false);
	pMenu->AddItem (item);
	BMessage *msg (new BMessage (M_SUBMIT));
	BString command ("/dcc send ");
	command += fId;
	msg->AddString ("input", command.String());
	item = new BMenuItem("DCC Send", msg);
	item->SetTarget (this);
	if (Id().FindFirst (" [DCC]") >= 0)	// dont enable for dcc sessions
			item->SetEnabled (false);
	pMenu->AddItem (item);
	pMenu->AddSeparatorItem();
}

void
MessageAgent::Init (void)
{
	if (fDChat && fDInitiate)
	{
		int32 dccPort (atoi (vision_app->GetString ("dccMinPort")));
		int32 diff (atoi (vision_app->GetString ("dccMaxPort")) - dccPort);
		if (diff > 0)
			dccPort += rand() % diff;

		fDPort << dccPort;
	}
}

void
MessageAgent::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case B_SIMPLE_DATA:
			{
				if (msg->HasRef("refs"))
				{
					// TODO: get this to work properly for multiple refs
					// and update DCC send logic to figure that out too
					entry_ref ref;
					msg->FindRef("refs", &ref);

					BMessage dccmsg(M_CHOSE_FILE);
					dccmsg.AddString("nick", fChatee.String());
					dccmsg.AddRef("refs", &ref);
					fSMsgr.SendMessage(&dccmsg);
				}
			}
			break;

		case M_CONNECTION_DISCONNECT:
			{
				BMessage termMsg (M_DISPLAY);
				fDConnected = false;
				BString temp = B_TRANSLATE("DCC Chat Terminated.");
				temp += "\n";
				ClientAgent::PackDisplay (&termMsg, temp.String());
				fMsgr.SendMessage (&termMsg);
			}
			break;

		case M_CONNECTION_DATA_RECEIVED:
			{
				const char *data = NULL;
				ssize_t size = -1;
				if (msg->FindData("data", B_RAW_TYPE, reinterpret_cast<const void **>(&data), &size) == B_OK)
				{
					fRecvBuffer.Append(data, size);
					char convBuffer[2048];
					int32 length (-1),
						destLength (sizeof(convBuffer)),
						state (0);

					BMessage dispMsg (M_CHANNEL_MSG);
					while ((length = fRecvBuffer.FindFirst('\n')) != B_ERROR)
					{
						length -= 1;
						memset (convBuffer, 0, sizeof(convBuffer));
						destLength = sizeof(convBuffer);
						state = 0;
						convert_to_utf8 (vision_app->GetInt32("encoding"),
							fRecvBuffer,
							&length,
							convBuffer,
							&destLength,
							&state);

						dispMsg.MakeEmpty();
						dispMsg.AddString ("msgz", convBuffer);
						fMsgr.SendMessage(&dispMsg);
						fRecvBuffer.Remove(0, length + 2);
					}
				}
			}
			break;

		case M_CONNECTION_CREATED:
			{
				int32 status = -1;
				BString temp;
				if (msg->FindInt32("status", &status) == B_OK && status == B_OK)
				{
					fDConnected = true;
					msg->FindInt32("connection", &fConnectionID);
					temp = B_TRANSLATE("Connected!");
				}
				else
				{
					fDConnected = false;
					temp = B_TRANSLATE("DCC Chat Terminated.");
				}
				temp += "\n";
				BMessage dispMsg(M_DISPLAY);
				ClientAgent::PackDisplay (&dispMsg, temp.String());
				fMsgr.SendMessage (&dispMsg);
			}
			break;

		case M_LISTENER_CREATED:
			{
				int32 status = -1;
				if (msg->FindInt32("status", &status) == B_OK && status == B_OK)
				{
					msg->FindInt32("connection", &fConnectionID);
					BMessage getIpMsg(M_GET_IP), reply;
					fSMsgr.SendMessage(&getIpMsg, &reply);
					BString ipAddr;
					reply.FindString("ip", &ipAddr);
					if (ipAddr.FindFirst(":") == B_ERROR)
					{
						addrinfo hints;
						hints.ai_flags = AI_NUMERICHOST | AI_ADDRCONFIG;
						addrinfo *res = NULL;
						memset(&hints, 0, sizeof(hints));
						if (getaddrinfo(ipAddr.String(), NULL, &hints, &res) == 0)
						{
							ipAddr = "";
							ipAddr << htonl(((sockaddr_in *)(res->ai_addr))->sin_addr.s_addr);
							freeaddrinfo(res);
						}
					}
					BString outNick (fChatee);
					outNick.RemoveFirst (" [DCC]");

					BString request = "PRIVMSG ";
					request << outNick
						<< " :\1DCC CHAT "
						<< ipAddr
						<< " "
						<< fDPort
						<< "\1";

					BMessage sendMsg(M_SERVER_SEND);
					sendMsg.AddString("data", request.String());
					fSMsgr.SendMessage (&sendMsg);
				}
				else
				{
					BMessage termMsg (M_DISPLAY);
					fDConnected = false;
					BString temp = B_TRANSLATE("DCC Chat Terminated.");
					temp += "\n";
					ClientAgent::PackDisplay (&termMsg, temp.String());
					fMsgr.SendMessage (&termMsg);
				}
			}
			break;

		case M_CONNECTION_ACCEPTED:
			{
				fDConnected = true;
				BMessage destroyMsg(M_DESTROY_CONNECTION);
				destroyMsg.AddInt32("connection", fConnectionID);
				BMessenger(network_manager).SendMessage(&destroyMsg);
				msg->FindInt32("client", &fConnectionID);
				BString temp = B_TRANSLATE("Connection accepted from %1.");
				temp.ReplaceFirst("%1", msg->FindString("address"));
				temp += "\n";
				BMessage dispMsg(M_DISPLAY);
				ClientAgent::PackDisplay (&dispMsg, temp.String());
				fMsgr.SendMessage (&dispMsg);
			}
			break;

		case M_CHANNEL_MSG:
			{
				const char *nick (NULL);

				if (msg->HasString ("nick"))
				{
					msg->FindString ("nick", &nick);
					BString outNick (nick);
					outNick.RemoveFirst (" [DCC]");
					if (fMyNick.ICompare (outNick) != 0 && !fDChat)
						fAgentWinItem->SetName (outNick.String());
					msg->ReplaceString ("nick", outNick.String());
				}
				else
				{
					BString outNick (fChatee.String());
					outNick.RemoveFirst (" [DCC]");
					msg->AddString ("nick", outNick.String());
				}

				BWindow *window (Window());

				if (IsHidden())
					UpdateStatus (WIN_NICK_BIT);

				if (IsHidden() || (window && !window->IsActive()))
				{
#ifdef __HAIKU__
					BNotification notification(B_INFORMATION_NOTIFICATION);
					notification.SetGroup(BString("Vision"));
					entry_ref ref = vision_app->AppRef();
					notification.SetOnClickFile(&ref);
					notification.SetTitle(fServerName.String());
					BString tempString(msg->FindString("msgz"));
					if (tempString[0] == '\1')
					{
						tempString.RemoveFirst("\1ACTION ");
						tempString.RemoveLast ("\1");
					}

					BString content;
					content << nick << " said: " << tempString.String();
					notification.SetContent(content);

					notification.Send();
#endif
#ifdef USE_INFOPOPPER
							if (be_roster->IsRunning(InfoPopperAppSig) == true) {
				entry_ref ref = vision_app->AppRef();
								BMessage infoMsg(InfoPopper::AddMessage);
								infoMsg.AddString("appTitle", S_INFOPOPPER_TITLE);
								infoMsg.AddString("title", fServerName.String());
								infoMsg.AddInt8("type", (int8)InfoPopper::Important);

								infoMsg.AddInt32("iconType", InfoPopper::Attribute);
								infoMsg.AddRef("iconRef", &ref);

				BString tempString(msg->FindString("msgz"));
								if (tempString[0] == '\1')
								{
									tempString.RemoveFirst("\1ACTION ");
									tempString.RemoveLast ("\1");
								}

								BString content;
								content << nick << " said: " << tempString.String();
								infoMsg.AddString("content", content);

								BMessenger(InfoPopperAppSig).SendMessage(&infoMsg);
							};
#endif
				}

				if (window != NULL && !window->IsActive())
					system_beep(kSoundEventNames[(uint32)seNickMentioned]);

				// Send the rest of processing up the chain
				ClientAgent::MessageReceived (msg);
			}
			break;

		case M_MSG_WHOIS:
			{
				BMessage dataSend (M_SERVER_SEND);

				AddSend (&dataSend, "WHOIS ");
				AddSend (&dataSend, fChatee.String());
				AddSend (&dataSend, " ");
				AddSend (&dataSend, fChatee.String());
				AddSend (&dataSend, endl);
			}

		case M_CHANGE_NICK:
			{
				const char *oldNick, *newNick;

				msg->FindString ("oldnick", &oldNick);
				msg->FindString ("newnick", &newNick);

				if (fChatee.ICompare (oldNick) == 0)
				{
					const char *address;
					const char *ident;

					msg->FindString ("address", &address);
					msg->FindString ("ident", &ident);

					BString oldId (fId);
					fChatee = fId = newNick;

					if (fDChat)
						fId.Append(" [DCC]");

					// set up new logging file for new nick
					BMessage logMsg (M_UNREGISTER_LOGGER);
					logMsg.AddString("name", oldId.String());
					fSMsgr.SendMessage(&logMsg);
					logMsg.MakeEmpty();
					logMsg.what = M_REGISTER_LOGGER;
					logMsg.AddString("name", fId.String());
					fSMsgr.SendMessage(&logMsg);

					fAgentWinItem->SetName (fId.String());


					ClientAgent::MessageReceived (msg);
				}

				else if (fMyNick.ICompare (oldNick) == 0)
				{
					if (!IsHidden())
						vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, newNick);
					ClientAgent::MessageReceived (msg);
				}
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
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_SERVER, fServerName.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_LAG, fMyLag.String(), false);
				vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK, fMyNick.String(), true);
			}
			break;

		default:
			ClientAgent::MessageReceived (msg);
	}
}

void
MessageAgent::ActionMessage (const char *msg, const char *nick)
{
	if (!fDChat)
		ClientAgent::ActionMessage (msg, nick);
	else if (fDConnected)
	{
		BString outTemp;
		outTemp = "\1ACTION ";
		outTemp += msg;
		outTemp += "\1";
		outTemp += "\n";

		char convBuffer[2048];
		memset (convBuffer, 0, sizeof(convBuffer));

		int32 length (outTemp.Length()),
					destLength (sizeof(convBuffer)),
					state (0);

		convert_from_utf8 (
			vision_app->GetInt32("encoding"),
			outTemp.String(),
			&length,
			convBuffer,
			&destLength,
			&state);

		BMessage message (M_SEND_CONNECTION_DATA);
		message.AddInt32("connection", fConnectionID);
		message.AddData("data", B_RAW_TYPE, convBuffer, destLength);
		BMessenger(network_manager).SendMessage(&message);

		outTemp.RemoveLast ("\n");
		ChannelMessage (outTemp.String(), nick);
	}

}
void
MessageAgent::Parser (const char *buffer)
{
	if (!fDChat)
	{
		BMessage dataSend (M_SERVER_SEND);

		AddSend (&dataSend, "PRIVMSG ");
		AddSend (&dataSend, fChatee);
		AddSend (&dataSend, " :");
		AddSend (&dataSend, buffer);
		AddSend (&dataSend, endl);
	}
	else if (fDConnected)
	{
		BString outTemp (buffer);

		outTemp << "\n";

		char convBuffer[2048];
		memset (convBuffer, 0, sizeof(convBuffer));

		int32 length (outTemp.Length()),
					destLength (sizeof(convBuffer)),
					state (0);

		convert_from_utf8 (
			vision_app->GetInt32("encoding"),
			outTemp.String(),
			&length,
			convBuffer,
			&destLength,
			&state);

		BMessage msg (M_SEND_CONNECTION_DATA);
		msg.AddInt32("connection", fConnectionID);
		msg.AddData("data", B_RAW_TYPE, convBuffer, destLength);
		BMessenger(network_manager).SendMessage(&msg);
	}
	else
		return;

	Display ("<", C_MYNICK);
	Display (fMyNick.String(), C_NICKDISPLAY);
	Display ("> ", C_MYNICK);

	BString sBuffer (buffer);
	Display (sBuffer.String());


	Display ("\n");
}

void
MessageAgent::DroppedFile (BMessage *)
{
	// TODO: implement this when DCC's ready
}

void
MessageAgent::TabExpansion (void)
{
	int32 start,
				finish;

	fInput->TextView()->GetSelection (&start, &finish);

	if (fInput->TextView()->TextLength()
	&&	start == finish
	&&	start == fInput->TextView()->TextLength())
	{
		const char *fInputText (
									fInput->TextView()->Text()
									+ fInput->TextView()->TextLength());
		const char *place (fInputText);


		while (place > fInput->TextView()->Text())
		{
			if (*(place - 1) == '\x20')
				break;
			--place;
		}

		BString insertion;

		if (!fId.ICompare (place, strlen (place)))
		{
			insertion = fId;
			insertion.RemoveLast(" [DCC]");
		}
		else if (!fMyNick.ICompare (place, strlen (place)))
			insertion = fMyNick;

		if (insertion.Length())
		{
			fInput->TextView()->Delete (
				place - fInput->TextView()->Text(),
				fInput->TextView()->TextLength());

			fInput->TextView()->Insert (insertion.String());
			fInput->TextView()->Select (
				fInput->TextView()->TextLength(),
				fInput->TextView()->TextLength());
		}
	}
}
