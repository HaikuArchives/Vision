
#include <StatusBar.h>
#include <StringView.h>
#include <Path.h>
#include <Mime.h>
#include <Window.h>

#include <stdlib.h>
#include <stdio.h>

#include "Vision.h"
#include "DCCConnect.h"
#include "PlayButton.h"

const uint32 M_STOP_BUTTON				= 'stop';
const uint32 M_UPDATE_STATUS			= 'stat';
const uint32 M_GET_CONNECT_DATA         = 'dccd';
const uint32 M_GET_RESUME_POS           = 'dcrp';
const uint32 M_SUCCESS                  = 'dccs';
const uint32 M_UPDATE_TRANSFERRED       = 'dcut';
const uint32 M_UPDATE_AVERAGE           = 'dcua';


#ifdef BONE_BUILD
#define DCC_BLOCK_SIZE 8192
#elif NETSERVER_BUILD
#define DCC_BLOCK_SIZE 1400
#endif

DCCConnect::DCCConnect (
	const char *n,
	const char *fn,
	const char *sz,
	const char *i,
	const char *p)

	: BView (
		BRect (0.0, 0.0, 275.0, 150.0),
		"dcc connect",
		B_FOLLOW_LEFT | B_FOLLOW_TOP,
		B_WILL_DRAW),

		nick (n),
		file_name (fn),
		size (sz),
		ip (i),
		port (p),
		finalRateAverage (0),
		tid (-1),
		success (false),
		isStopped (false)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));

	char trail[128];
	sprintf (trail, " / %.1fk", atol (size.String()) / 1024.0);

	bar = new BStatusBar (
		BRect (10, 10, Bounds().right - 30, Bounds().bottom - 10),
		"progress",
		"bps: ",
		trail);
	bar->SetMaxValue (atol (size.String()));
	bar->SetBarHeight (8.0);
	AddChild (bar);

	label = new BStringView (
		BRect (10, 10, Bounds().right - 10, Bounds().bottom - 10),
		"label",
		"",
		B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);
	AddChild (label);

	stop = new StopButton (
		BPoint (bar->Frame().right + 15, bar->Frame().bottom - 18),
		new BMessage (M_STOP_BUTTON));
	AddChild (stop);
}

DCCConnect::~DCCConnect (void)
{
}

void
DCCConnect::AttachedToWindow (void)
{
	stop->SetTarget (this);
}

void
DCCConnect::AllAttached (void)
{
	stop->MoveTo (
		bar->Frame().right + 15, bar->Frame().bottom - 18);
	label->MoveTo (
		label->Frame().left,
		bar->Frame().bottom + 1);

	float width, height;
	label->GetPreferredSize (&width, &height);
	label->ResizeTo (label->Frame().Width(), height);
	ResizeTo (stop->Frame().right + 5, label->Frame().bottom + 5.0);
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
		case M_STOP_BUTTON:
		{
			Stopped();
			break;
		}

		case M_UPDATE_STATUS:
		{
			label->SetText (msg->FindString ("text"));
			break;
		}
		
		case M_GET_CONNECT_DATA:
		{
		  BMessage reply;
		  reply.AddString ("port", port.String());
		  reply.AddString ("ip", ip.String());
		  reply.AddString ("name", file_name.String());
		  reply.AddString ("nick", nick.String());
		  reply.AddString ("size", size.String());
		  DCCReceive *recview (dynamic_cast<DCCReceive *>(this));
		  if (recview != NULL)
		    reply.AddBool ("resume", recview->resume);
		  DCCSend *sendview (dynamic_cast<DCCSend *>(this));
		  if (sendview != NULL)
		  {
		    reply.AddData ("addr", B_RAW_TYPE, &sendview->addr, sizeof(in_addr));
		    reply.AddMessenger ("caller", sendview->caller);
		  }
		  msg->SendReply(&reply);
		}
		break;
		
		case M_GET_RESUME_POS:
		{
		  BMessage reply;
		  DCCSend *sendview (dynamic_cast<DCCSend *>(this));
		  if (sendview != NULL)
		    reply.AddInt32 ("pos", sendview->pos);
		  msg->SendReply(&reply);
		}
		break;
		
		case M_UPDATE_TRANSFERRED:
		{
		  totalTransferred = msg->FindInt32 ("transferred");
		}
		break;
		
		case M_UPDATE_AVERAGE:
		{
		  finalRateAverage = msg->FindInt32 ("average");
		}
		break;
		
		case M_SUCCESS:
		{
		   success = true;
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
 	if (totalTransferred > 0)
 	{
 	
 		BMessage xfermsg (M_DCC_COMPLETE);
 		xfermsg.AddString("nick", nick.String());
 		xfermsg.AddString("file", file_name.String());
 		xfermsg.AddString("size", size.String());
 		xfermsg.AddInt32("transferred", totalTransferred);
 		xfermsg.AddInt32("transferRate", finalRateAverage);
 	
 		DCCReceive *recview = dynamic_cast<DCCReceive *>(this);
 		if (recview)
 		{
 			xfermsg.AddString("type", "RECV");
 		}
 		else
 		{
 			xfermsg.AddString("type", "SEND");	
 		}
 	
 		vision_app->PostMessage(&xfermsg);
 	}
 	

	BMessage msg (M_DCC_FINISH);

	msg.AddBool ("success", success);
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
DCCConnect::UpdateBar (const BMessenger &msgr, int read, int cps, uint32 size, bool update)
{
	BMessage msg (B_UPDATE_STATUS_BAR);

	if (update)
	{
		char text[128];

		sprintf (text, "%.1f", size / 1024.0);
		msg.AddString ("trailing_text", text);

		sprintf (text, "%d", cps);
		msg.AddString ("text", text);
	}

	msg.AddFloat ("delta", read);
	BLooper *looper (NULL);
	DCCConnect *transferView ((DCCConnect *)msgr.Target(&looper));
	if ((looper != NULL) && (transferView != NULL))
	  looper->PostMessage (&msg, transferView->bar);
}

void
DCCConnect::UpdateStatus (const BMessenger &msgr, const char *text)
{
	BMessage msg (M_UPDATE_STATUS);

	msg.AddString ("text", text);
	msgr.SendMessage (&msg);
}

DCCReceive::DCCReceive (
	const char *n,
	const char *fn,
	const char *sz,
	const char *i,
	const char *p,
	bool cont)

	: DCCConnect (n, fn, sz, i, p),
	  resume (cont)
{
}

DCCReceive::~DCCReceive (void)
{
}

void
DCCReceive::AttachedToWindow (void)
{
	DCCConnect::AttachedToWindow();

	tid = spawn_thread (
		Transfer,
		"DCC Receive",
		B_NORMAL_PRIORITY,
		this);
	resume_thread (tid);
}

int32
DCCReceive::Transfer (void *arg)
{
	BMessenger msgr (reinterpret_cast<DCCReceive *>(arg));
	struct sockaddr_in sin;
	BLooper *looper (NULL);
	
	int32 dccSock (-1);
	
	if ((dccSock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		UpdateStatus (msgr, "Unable to establish connection.");
		return B_ERROR;
	}
	
	BMessage reply;
	
	if (msgr.SendMessage (M_GET_CONNECT_DATA, &reply) == B_ERROR)
	  return B_ERROR;
	
	
	
	memset (&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port   = htons (atoi (reply.FindString ("port")));
	sin.sin_addr.s_addr = htonl(strtoul (reply.FindString("ip"), 0, 10));

	UpdateStatus (msgr, "Connecting to sender.");
	if (connect (dccSock, (sockaddr *)&sin, sizeof (sin)) < 0)
	{
		UpdateStatus (msgr, "Unable to establish connection.");
#ifdef BONE_BUILD
		close (dccSock);
#elif NETSERVER_BUILD
		closesocket (dccSock);
#endif
		return B_ERROR;
	}
	
	BPath path (reply.FindString("name"));
	BString buffer;
	off_t file_size (0);
	
	buffer << "Receiving \""
		<< path.Leaf()
		<< "\" from "
		<< reply.FindString ("nick")
		<< ".";

	UpdateStatus (msgr, buffer.String());
	
	BFile file;
	
	if (msgr.IsValid())
	{
  		if (reply.FindBool ("resume"))
		{
			if (file.SetTo (
				reply.FindString("name"),
				B_WRITE_ONLY | B_OPEN_AT_END) == B_NO_ERROR
			&&  file.GetSize (&file_size) == B_NO_ERROR
			&&  file_size > 0LL)
				UpdateBar (msgr, file_size, 0, 0, true);
			else
				file_size = 0LL;
		}
		else
		{
			file.SetTo (
				reply.FindString("name"),
				B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
		}
	}
		
	int32 bytes_received (file_size);
	int32 size (atol (reply.FindString("size")));
	int32 cps (0);
	
	if (file.InitCheck() == B_NO_ERROR)
	{
		bigtime_t last (system_time()), now;
		int period (0);
		char buffer[8196];		
		bigtime_t start = system_time();


		while ((msgr.Target(&looper) != NULL) && bytes_received < size)
		{
			int read;
			if ((read = recv (dccSock, buffer, 8196, 0)) < 0)
				break;
			
			file.Write (buffer, read);
			bytes_received += read;
			BMessage msg (M_UPDATE_TRANSFERRED);
			msg.AddInt32 ("transferred", bytes_received);
			msgr.SendMessage (&msg);

			uint32 feed_back (htonl (bytes_received));
			send (dccSock, &feed_back, sizeof (uint32), 0);

			now = system_time();
			period += read;
			bool hit (false);

			if (now - last > 500000)
			{
				cps = (int)ceil ((bytes_received - file_size) / ((now - start) / 1000000.0));
				BMessage msg (M_UPDATE_AVERAGE);
				msg.AddInt32 ("transferred", cps);
				msgr.SendMessage (&msg);
				last = now;
				period = 0;
				hit = true;
			}

			DCCConnect::UpdateBar (msgr, read, cps, bytes_received, hit);
		}
	}

	if (msgr.IsValid())
	{
	    BMessage msg (M_SUCCESS);
	    msg.AddBool ("success", bytes_received == size);
	    msgr.SendMessage (&msg);
	}

	if (dccSock > 0)
	{
#ifdef BONE_BUILD
	close (dccSock);
#elif NETSERVER_BUILD
	closesocket (dccSock);
#endif
	}

	if (file.InitCheck() == B_OK)
		file.Unset();

	return 0;
}

DCCSend::DCCSend (
	const char *n,
	const char *fn,
	const char *sz,
	const BMessenger &c,
	struct in_addr a)

	: DCCConnect (n, fn, sz, "", ""),
	  caller (c),
	  pos (0LL),
	  addr (a)
{
	port << (40000 + (rand() % 5000)); // baxter's way of getting a port
}

DCCSend::~DCCSend (void)
{
}

void
DCCSend::AttachedToWindow (void)
{
	DCCConnect::AttachedToWindow();

	tid = spawn_thread (
		Transfer,
		"DCC Send",
		B_NORMAL_PRIORITY,
		this);
	resume_thread (tid);
}

int32
DCCSend::Transfer (void *arg)
{
	BMessenger msgr (reinterpret_cast<DCCSend *>(arg));
	BMessage reply;
	BLooper *looper (NULL);

	if (msgr.IsValid())
	  msgr.SendMessage (M_GET_CONNECT_DATA, &reply);
	
	BPath path (reply.FindString ("name"));
	BString file_name, status;
	struct sockaddr_in sin;
	const struct in_addr *sendaddr;
	memset (&sendaddr, 0, sizeof (struct in_addr));
	int sd, dccSock (-1);

	file_name.Append (path.Leaf());
	file_name.ReplaceAll (" ", "_");

	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		UpdateStatus (msgr, "Unable to establish connection.");
		return 0;
	}

	memset (&sin, 0, sizeof (struct sockaddr_in));
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port        = htons (atoi (reply.FindString("port")));

	int sin_size;
	reply.FindData ("addr", B_RAW_TYPE, reinterpret_cast<const void **>(&sendaddr), (ssize_t *)&sin_size);
	sin_size = (sizeof (struct sockaddr_in));
		
	if (!msgr.IsValid() || bind (sd, (sockaddr *)&sin, sin_size) < 0)
	{
		UpdateStatus (msgr, "Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
		return 0;
	}
	UpdateStatus (msgr, "Waiting for acceptance.");

	if (msgr.IsValid())
	{
		status = "PRIVMSG ";
		status << reply.FindString ("nick") 
			<< " :\1DCC SEND "
			<< file_name
			<< " "
			<< htonl (sendaddr->s_addr)
			<< " "
			<< reply.FindString ("port")
			<< " "
			<< reply.FindString ("size")
			<< "\1";

		BMessage msg (M_SERVER_SEND);
		msg.AddString ("data", status.String());
		BMessenger callmsgr;
		reply.FindMessenger ("caller", &callmsgr);
		if (callmsgr.IsValid())
		  callmsgr.SendMessage (&msg);
		UpdateStatus (msgr, "Doing listen call.");
		if (listen (sd, 1) < 0)
		{
			UpdateStatus (msgr, "Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
			return 0;
		}
	}

	struct timeval t;
	t.tv_sec  = 2;
	t.tv_usec = 0;

	uint32 try_count (0);

	while (msgr.Target(&looper) != NULL)	
	{
		fd_set rset;

		FD_ZERO (&rset);
		FD_SET (sd, &rset);

		if (select (sd + 1, &rset, 0, 0, &t) < 0)
		{
			UpdateStatus (msgr, "Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
			return 0;
		}

		if (FD_ISSET (sd, &rset))
		{
			dccSock = accept (sd, (sockaddr *)&sin, &sin_size);
			UpdateStatus (msgr, "Established connection.");
			break;
		}

		++try_count;
		status = "Waiting for connection ";
		status << try_count << ".";
		UpdateStatus (msgr, status.String());
	};
	char set[4];
	memset(set, 1, sizeof(set));
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
	BFile file;

	file.SetTo(reply.FindString ("name"), B_READ_ONLY);
	int32 bytes_sent (0L),
	      seekpos (0L);
	
	BMessage resumeData;
	msgr.SendMessage (M_GET_RESUME_POS, &resumeData);
	
	if (resumeData.HasInt32 ("pos"))
	{
	 	resumeData.FindInt32 ("pos", &seekpos);
		file.Seek (seekpos, SEEK_SET);
		UpdateBar (msgr, seekpos, 0, 0, true);
		bytes_sent = seekpos;
	}

	status = "Sending \"";
	status << path.Leaf() 
		<< "\" to "
		<< reply.FindString ("nick")
		<< ".";
	UpdateStatus (msgr, status.String());

	int cps (0);
	
	
	if (file.InitCheck() == B_NO_ERROR)
	{
		bigtime_t last (system_time()), now;
		char buffer[DCC_BLOCK_SIZE];
		int period (0);
		ssize_t count;
		bigtime_t start = system_time();

		while ((msgr.Target(&looper) != NULL)
		&&    (count = file.Read (buffer, DCC_BLOCK_SIZE - 1)) > 0)
		{
			int sent;

			if ((sent = send (dccSock, buffer, count, 0)) < count)
			{
				UpdateStatus (msgr, "Error writing data.");
				break;
			}

			bytes_sent += sent;
			BMessage msg (M_UPDATE_TRANSFERRED);
			msg.AddInt32 ("transferred", bytes_sent);
			msgr.SendMessage (&msg);
	
			uint32 confirm;
			fd_set rset;
			FD_ZERO (&rset);
			FD_SET (dccSock, &rset);
			t.tv_sec = 0;
			t.tv_usec = 10;
			
			while (select (dccSock + 1, &rset, NULL, NULL, &t) > 0 && FD_ISSET (dccSock, &rset))
			{
  			  recv (dccSock, &confirm, sizeof (confirm), 0);
  			  FD_ZERO (&rset);
  			  FD_SET (dccSock, &rset);
  			}

			now = system_time();
			period += sent;

			bool hit (false);

			if (now - last > 500000)
			{
				cps = (int) ceil ((bytes_sent - seekpos) / ((now - start) / 1000000.0));
				BMessage msg (M_UPDATE_AVERAGE);
				msg.AddInt32 ("transferred", cps);
				msgr.SendMessage (&msg);
				last = now;
				period = 0;
				hit = true;
			}

			UpdateBar (msgr, sent, cps, bytes_sent, hit);
		}
	}

	off_t size;
	file.GetSize (&size);

	if (msgr.IsValid())
	{
	    BMessage msg (M_SUCCESS);
	    msg.AddBool ("success", bytes_sent == size);
	    msgr.SendMessage (&msg);
	}

	if (dccSock > 0)
	{
#ifdef BONE_BUILD
	close (dccSock);
#elif NETSERVER_BUILD
	closesocket (dccSock);
#endif
	}

	if (file.InitCheck() == B_OK)
		file.Unset();
	
	return 0;
}

bool
DCCSend::IsMatch (const char *n, const char *p) const
{
	return nick == n && port == p;
}

void
DCCSend::SetResume (off_t p)
{
	pos = p;
}
