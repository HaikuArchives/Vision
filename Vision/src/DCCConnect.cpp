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
 * Contributor(s): Rene Gollent
 *								 Wade Majors
 *								 Todd Lair
 */

#include <Catalog.h>
#include <Mime.h>
#include <Path.h>
#include <StatusBar.h>
#include <StringView.h>
#include <Window.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "DCCConnect.h"
#include "NetworkManager.h"
#include "PlayButton.h"
#include "Vision.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "DCCMessages"

static const uint32 M_SEND_NEXT_BLOCK = 'msnb';

DCCConnect::DCCConnect (
	const char *n,
	const char *fn,
	const char *sz,
	const char *i,
	const char *p,
	const BMessenger &c)
		: BView (
				BRect (0.0, 0.0, 275.0, 150.0),
				"dcc connect",
				B_FOLLOW_LEFT | B_FOLLOW_TOP,
				B_WILL_DRAW),
				fCaller (c),
				fNick (n),
				fFileName (fn),
				fSize (sz),
				fIp (i),
				fPort (p),
				fTotalTransferred (0),
				fFinalRateAverage (0.0),
				fIsStopped (false)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));

	char trail[128];
	sprintf (trail, " / %.1fk", atol (fSize.String()) / 1024.0);

	BString statusBarString = B_TRANSLATE("KB/sec");
	statusBarString += ": ";

	fBar = new BStatusBar (
		BRect (10, 10, Bounds().right - 30, Bounds().bottom - 10),
		"progress",
		statusBarString.String(),
		trail);
	fBar->SetMaxValue (atol (fSize.String()));
	fBar->SetBarHeight (8.0);
	AddChild (fBar);

	fLabel = new BStringView (
		BRect (10, 10, Bounds().right - 10, Bounds().bottom - 10),
		"label",
		"",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	AddChild (fLabel);

	fStop = new StopButton (
		BPoint (fBar->Frame().right + 15, fBar->Frame().bottom - 18),
		new BMessage (M_DCC_STOP_BUTTON));
	
	AddChild (fStop);
}

DCCConnect::~DCCConnect (void)
{
	if (fFile.InitCheck() == B_OK)
	{
		fFile.Unset();
	}
}

void
DCCConnect::AttachedToWindow (void)
{
	fStop->SetTarget (this);
}

void
DCCConnect::AllAttached (void)
{
	fStop->MoveTo (
		fBar->Frame().right + 15, fBar->Frame().bottom - 18);
	fLabel->MoveTo (
		fLabel->Frame().left,
		fBar->Frame().bottom + 1);

	float width, height;
	fLabel->GetPreferredSize (&width, &height);
	fLabel->ResizeTo (fLabel->Frame().Width(), height);
	ResizeTo (fStop->Frame().right + 5, fLabel->Frame().bottom + 5.0);
}

void
DCCConnect::DetachedFromWindow (void)
{
}

void
DCCConnect::Draw (BRect)
{
	BView *top (Window()->ChildAt (0));

	if (this != top)
	{
		BeginLineArray (2);
		AddLine (
			Bounds().LeftTop(),
			Bounds().RightTop(),
			tint_color (ViewColor(), B_DARKEN_2_TINT));

		AddLine (
			Bounds().LeftTop() + BPoint (0.0, 1.0),
			Bounds().RightTop() + BPoint (0.0, 1.0),
			tint_color (ViewColor(), B_LIGHTEN_MAX_TINT));
		EndLineArray();
	}
}

void
DCCConnect::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_DCC_STOP_BUTTON:
			{
				Stopped();
			}
			break;

		default:
			BView::MessageReceived (msg);
	}
}

void
DCCConnect::Stopped (void)
{
	BMessage destroyMsg(M_DESTROY_CONNECTION);
	destroyMsg.AddInt32("connection", fSocketID);
	BMessenger(network_manager).SendMessage(&destroyMsg);
	if (fTotalTransferred > 0)
	{
		BMessage xfermsg (M_DCC_COMPLETE);
		xfermsg.AddString("nick", fNick.String());
		xfermsg.AddString("file", fFileName.String());
		xfermsg.AddString("size", fSize.String());
		xfermsg.AddInt32("transferred", fTotalTransferred);
		xfermsg.AddInt32("transferRate", fFinalRateAverage);

		DCCReceive *recview = dynamic_cast<DCCReceive *>(this);
		if (recview)
		{
			xfermsg.AddString("type", "RECV");
		}
		else
		{
			xfermsg.AddString("type", "SEND");
		}
		fCaller.SendMessage(&xfermsg);
	}

	BMessage msg (M_DCC_FINISH);

	msg.AddPointer ("source", this);
	msg.AddBool ("stopped", true);
	Window()->PostMessage (&msg);
}

void
DCCConnect::Lock (void)
{
}

void
DCCConnect::Unlock (void)
{
}

void
DCCConnect::UpdateBar (int readSize, bool update)
{
	char text[128];
	char trailing_text[128];
	if (update)
	{
		sprintf (trailing_text, "%.1f", fTotalTransferred / 1024.0);
		sprintf (text, "%.2f", (fTotalTransferred / 1024.0) / ((system_time() - fTransferStartTime) / 1000000));
	}
	if (update)
		fBar->Update(readSize, text ,trailing_text);
	else
		fBar->Update(readSize, NULL, NULL);
}

void
DCCConnect::UpdateStatus (const char *text)
{
	fLabel->SetText (text);
}

DCCReceive::DCCReceive (
	const char *n,
	const char *fn,
	const char *sz,
	const char *i,
	const char *p,
	const BMessenger &c,
	bool cont)
		: DCCConnect (n, fn, sz, i, p, c),
			fResume (cont)
{
	fFile.SetTo(fn, B_READ_WRITE);
}

DCCReceive::~DCCReceive (void)
{
}

void
DCCReceive::AttachedToWindow (void)
{
	DCCConnect::AttachedToWindow();

	BMessage msg(M_CREATE_CONNECTION);
	msg.AddMessenger("target", BMessenger(this));
	msg.AddString("port", fPort);
	msg.AddString("hostname", fIp.String());
	BMessenger(network_manager).SendMessage(&msg);
}

void
DCCReceive::MessageReceived(BMessage *msg)
{
	switch (msg->what)
	{
		case M_CONNECTION_CREATED:
		{
			int32 status = msg->FindInt32("status");
			if (status != B_OK)
			{
				UpdateStatus(B_TRANSLATE("Unable to establish connection."));
			}
			
			msg->FindInt32("connection", &fSocketID);
			if (fResume)
			{
				fFile.SetTo(fFileName, B_WRITE_ONLY | B_OPEN_AT_END);
			}
			else
			{
				fFile.SetTo(fFileName, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
			}
			
			if (fFile.InitCheck() == B_OK)
			{
				off_t fileSize = 0;
				status = fFile.GetSize(&fileSize);
				fTotalTransferred = (uint32) fileSize;
				if (status == B_OK)
				{
					UpdateBar(fTotalTransferred, true);
				}
			}
			else
			{
				UpdateStatus(B_TRANSLATE("Error creating / opening data file."));
			}
		}
		break;
		
		case M_CONNECTION_DATA_RECEIVED:
		{
			ssize_t size = 0;
			const void *data = NULL;
			if (msg->FindData("data", B_RAW_TYPE, reinterpret_cast<const void **>(&data), &size) == B_OK)
			{
				if (fFile.Write(data, size) == B_OK)
				{
					fTotalTransferred += size;
					UpdateBar(fTotalTransferred, true);
					BMessage message(M_SEND_CONNECTION_DATA);
					message.AddData("data", B_RAW_TYPE, &fTotalTransferred, sizeof(uint32));
					message.AddInt32("connection", fSocketID);
					BMessenger(network_manager).SendMessage(&message);
					if (fTotalTransferred == atoi(fSize.String()))
					{
						Stopped();
					}
				}
				else
				{
					UpdateStatus(B_TRANSLATE("Error writing data to disk."));
					BMessage message(M_DESTROY_CONNECTION);
					message.AddInt32("connection", fSocketID);
					BMessenger(network_manager).SendMessage(&message);
				}
			}
		}
		break;
		
		default:
		{
			DCCConnect::MessageReceived(msg);
		}
		break;
	}
}

DCCSend::DCCSend (
	const char *n,
	const char *fn,
	const char *sz,
	const BMessenger &c)
		: DCCConnect (n, fn, sz, "", "", c),
			fPos ((int32)0L)
{
	int32 dccPort (atoi (vision_app->GetString ("dccMinPort")));
	int32 diff (atoi (vision_app->GetString ("dccMaxPort")) - dccPort);
	if (diff > 0)
		dccPort += rand() % diff;
			
	fPort << dccPort;
	fFile.SetTo(fn, B_READ_ONLY);
}

DCCSend::~DCCSend (void)
{
}

void
DCCSend::AttachedToWindow (void)
{
	DCCConnect::AttachedToWindow();
	
	BMessage msg(M_CREATE_LISTENER);
	msg.AddMessenger("target", BMessenger(this));
	msg.AddString("port", fPort);
	BMessenger(network_manager).SendMessage(&msg);
}

void
DCCSend::MessageReceived(BMessage *msg)
{
	BString status;
	switch (msg->what)
	{
		case M_LISTENER_CREATED:
		{
			status_t result = B_OK;
			if (msg->FindInt32("status", &result) == B_OK && result == B_OK)
			{
				msg->FindInt32("connection", &fServerID);
				BMessage getIpMsg(M_GET_IP), reply;
				fCaller.SendMessage(&getIpMsg, &reply);
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
				status = "PRIVMSG ";
				status << fNick 
					<< " :\1DCC SEND "
					<< fFileName
					<< " "
					<< ipAddr
					<< " "
					<< fPort
					<< " "
					<< fSize
					<< "\1";

				BMessage sendMsg(M_SERVER_SEND);
				sendMsg.AddString("data", status.String());
				if (fCaller.IsValid())
					fCaller.SendMessage (&sendMsg);
				UpdateStatus(B_TRANSLATE("Waiting for connection" B_UTF8_ELLIPSIS));
			}
			else
			{
				UpdateStatus(B_TRANSLATE("Unable to set up listener."));
			}
		}
		break;
		
		case M_CONNECTION_ACCEPTED:
		{
			BMessage destroyMsg(M_DESTROY_CONNECTION);
			destroyMsg.AddInt32("connection", fServerID);
			BMessenger(network_manager).SendMessage(&destroyMsg);
			msg->FindInt32("client", &fSocketID);
			status = B_TRANSLATE("Sending %1 to %2.");
			BPath path(fFileName);
			status.ReplaceFirst("%1", path.Leaf());
			status.ReplaceFirst("%2", fNick);
			UpdateStatus (status.String());
			fFile.Seek(fPos, SEEK_SET);
			fTotalTransferred = fPos;
			SendNextBlock();
		}
		break;
		
		case M_SEND_NEXT_BLOCK:
		{
			SendNextBlock();
		}
		break;

		case M_CONNECTION_DISCONNECT:
		{
			if (msg->FindInt32("connection") == fSocketID)
			{
				UpdateStatus (B_TRANSLATE("Error writing data."));
			}
		}
		break;
		
		case M_CONNECTION_DATA_RECEIVED:
		{
			int32 recvsize = -1;
			ssize_t size = 0;
			const uint32 *data = NULL;
			if (msg->FindData("data", B_RAW_TYPE, reinterpret_cast<const void **>(&data), &size) == B_OK)
			{
				for (ssize_t i = 0; static_cast<ssize_t>(i * sizeof(uint32)) < size; i++)
				{
					recvsize = ntohl(data[i]);
				}
			}
			if (recvsize == fTotalTransferred)
			{
				if (SendNextBlock() == 0)
				{
					Stopped();
				}
			}
		}
		break;
		
		default:
		{
			DCCConnect::MessageReceived(msg);
		}
		break;
	}
}

ssize_t
DCCSend::SendNextBlock(void)
{
	const int32 DCC_BLOCK_SIZE (atoi(vision_app->GetString ("dccBlockSize")));
	char buffer[DCC_BLOCK_SIZE];

	int32 count = fFile.Read (buffer, DCC_BLOCK_SIZE);
	if (count > 0)
	{
		fTotalTransferred += count;
		UpdateBar(count, true);
		BMessage msg(M_SEND_CONNECTION_DATA);
		msg.AddInt32("connection", fSocketID);
		msg.AddData("data", B_RAW_TYPE, buffer, count);
		BMessenger(network_manager).SendMessage(&msg);
	}
	
	return count;
}

bool
DCCSend::IsMatch (const char *n, const char *p) const
{
	return fNick == n && fPort == p;
}

void
DCCSend::SetResume (off_t p)
{
	fPos = p;
}
