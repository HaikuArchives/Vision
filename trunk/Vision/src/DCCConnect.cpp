
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
		running (false),
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
	running = true;
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

		default:
			BView::MessageReceived (msg);
	}
}

void
DCCConnect::Stopped (void)
{
	running = false;

	if (s > 0)
	{
#ifdef BONE_BUILD
		close (s);
#elif NETSERVER_BUILD
		closesocket (s);
#endif
	}

	Unlock();
	if (file.InitCheck() == B_OK)
	{
		file.Unset();
	}

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

	msg.AddBool ("success", true);
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
DCCConnect::UpdateBar (int read, int cps, uint32 size, bool update)
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
	Looper()->PostMessage (&msg, bar);
}

void
DCCConnect::UpdateStatus (const char *text)
{
	BMessage msg (M_UPDATE_STATUS);
	BMessenger msgr (this, Window());

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
	running = false;
	status_t result;
	wait_for_thread (tid, &result);
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
	DCCReceive *view ((DCCReceive *)arg);
	struct sockaddr_in sin;
	//unsigned long ip;
	
	if ((view->s = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		view->UpdateStatus ("Unable to establish connection.");
		view->running = false;
		return 0;
	}
	
	memset (&sin, 0, sizeof(sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port   = htons (atoi (view->port.String()));
	sin.sin_addr.s_addr = htonl(strtoul (view->ip.String(), 0, 10));

//	printf ("***** CONNECTION TO %s\n", view->ip.String());

	view->UpdateStatus ("Connecting to sender.");
	if (connect (view->s, (sockaddr *)&sin, sizeof (sin)) < 0)
	{
		view->UpdateStatus ("Unable to establish connection.");
		view->running = false;
		return 0;
	}
	
//	printf("connected\n");

	BPath path (view->file_name.String());
	BString buffer;
	off_t file_size (0);
	
	buffer << "Receiving \""
		<< path.Leaf()
		<< "\" from "
		<< view->nick
		<< ".";

	view->UpdateStatus (buffer.String());
	
	if (view->resume && view->running)
	{
		if (view->file.SetTo (
			view->file_name.String(),
			B_WRITE_ONLY | B_OPEN_AT_END) == B_NO_ERROR
		&&  view->file.GetSize (&file_size) == B_NO_ERROR
		&&  file_size > 0LL)
		{
			view->UpdateBar (file_size, 0, 0, true);
		}
		else
		{
			view->resume = false;
			file_size = 0LL;
		}
	}
	else if (view->running)
	{
		view->file.SetTo (
			view->file_name.String(),
			B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	}
	
	int32 bytes_received (file_size);
	int32 size (atol (view->size.String()));
	int32 cps (0);
	
	if (view->file.InitCheck() == B_NO_ERROR)
	{
		bigtime_t last (system_time()), now;
		int period (0);
		char buffer[8196];		
		bigtime_t start = system_time();


		while (view->running && bytes_received < size)
		{
			int read;
			if ((read = recv (view->s, buffer, 8196, 0)) < 0)
			{
				break;
			}
			
			view->file.Write (buffer, read);
			bytes_received += read;
			view->totalTransferred = bytes_received;

			uint32 feed_back (htonl (bytes_received));
			send (view->s, &feed_back, sizeof (uint32), 0);

			now = system_time();
			period += read;
			bool hit (false);

			if (now - last > 500000)
			{
				cps = (int)ceil ((bytes_received - file_size) / ((now - start) / 1000000.0));
				view->finalRateAverage = cps;
				last = now;
				period = 0;
				hit = true;
			}

			view->UpdateBar (read, cps, bytes_received, hit);
		}
	}

	if (view->running)
	{
		view->success = bytes_received == size;
		update_mime_info(path.Path(), false, false, true);
		view->file.Unset();	
		view->Stopped ();
	}
	
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
	BMessage reply;
	be_app_messenger.SendMessage (M_DCC_PORT, &reply);
	port << (40000 + (rand() % 5000)); // baxter's way of getting a port
	reply.FindInt32 ("sid", &sid);
}

DCCSend::~DCCSend (void)
{
	running = false;
	status_t result;
	wait_for_thread (tid, &result);
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
	DCCSend *view ((DCCSend *)arg);
	BPath path (view->file_name.String());
	BString file_name, status;
	struct sockaddr_in sin;
	int sd;

	file_name.Append (path.Leaf());
	file_name.ReplaceAll (" ", "_");

	view->Lock();
	if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
	{
		view->UpdateStatus ("Unable to establish connection.");
		view->Unlock();
		view->Stopped();
		return 0;
	}

	memset (&sin, 0, sizeof (struct sockaddr_in));
	sin.sin_family      = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port        = htons (atoi (view->port.String()));

	int sin_size = (sizeof (struct sockaddr_in));
		
	if (!view->running || bind (sd, (sockaddr *)&sin, sin_size) < 0)
	{
		view->UpdateStatus ("Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
		view->Unlock();
		view->Stopped();
		return 0;
	}
	view->UpdateStatus ("Waiting for acceptance.");

	if (view->running)
	{
		status = "PRIVMSG ";
		status << view->nick
			<< " :\1DCC SEND "
			<< file_name
			<< " "
			<< htonl (view->addr.s_addr)
			<< " "
			<< view->port
			<< " "
			<< view->size
			<< "\1";

		BMessage msg (M_SERVER_SEND);
		msg.AddString ("data", status.String());
		view->caller.SendMessage (&msg);
		view->UpdateStatus ("Doing listen call.");
		if (listen (sd, 1) < 0)
		{
			view->UpdateStatus ("Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
			view->Unlock();
			view->Stopped();
			return 0;
		}
	}

	struct timeval t;
	t.tv_sec  = 2;
	t.tv_usec = 0;

	uint32 try_count (0);

	while (view->running)	
	{
		fd_set rset;

		FD_ZERO (&rset);
		FD_SET (sd, &rset);

		if (select (sd + 1, &rset, 0, 0, &t) < 0)
		{
			view->UpdateStatus ("Unable to establish connection.");
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
			view->Unlock();
			view->Stopped();
			return 0;
		}

		if (FD_ISSET (sd, &rset))
		{
			view->s = accept (sd, (sockaddr *)&sin, &sin_size);
			view->UpdateStatus ("Established connection.");
			break;
		}

		++try_count;
		status = "Waiting for connection ";
		status << try_count << ".";
		view->UpdateStatus (status.String());
	};
	char set[4];
	memset(set, 1, sizeof(set));
#ifdef BONE_BUILD
		close (sd);
#elif NETSERVER_BUILD
		closesocket (sd);
#endif
	view->Unlock();
	
	view->file.SetTo(view->file_name.String(), B_READ_ONLY);
	int32 bytes_sent (0L);

	if (view->pos)
	{
		view->file.Seek (view->pos, SEEK_SET);
		view->UpdateBar (view->pos, 0, 0, true);
		bytes_sent = view->pos;
	}

	status = "Sending \"";
	status << path.Leaf() 
		<< "\" to "
		<< view->nick
		<< ".";
	view->UpdateStatus (status.String());

	int cps (0);
	
	if (view->file.InitCheck() == B_NO_ERROR)
	{
		bigtime_t last (system_time()), now;
		char buffer[DCC_BLOCK_SIZE];
		int period (0);
		ssize_t count;
		bigtime_t start = system_time();

		while (view->running
		&&    (count = view->file.Read (buffer, DCC_BLOCK_SIZE - 1)) > 0)
		{
			int sent;

			if ((sent = send (view->s, buffer, count, 0)) < count)
			{
				view->UpdateStatus ("Error writing data.");
				break;
			}

			bytes_sent += sent;
			view->totalTransferred = bytes_sent;
	
			uint32 confirm;
			fd_set rset;
			FD_ZERO (&rset);
			FD_SET (view->s, &rset);
			t.tv_sec = 0;
			t.tv_usec = 10;
			
			while (select (view->s + 1, &rset, NULL, NULL, &t) > 0 && FD_ISSET (view->s, &rset))
			{
  			  recv (view->s, &confirm, sizeof (confirm), 0);
  			  FD_ZERO (&rset);
  			  FD_SET (view->s, &rset);
  			}

			now = system_time();
			period += sent;

			bool hit (false);

			if (now - last > 500000)
			{
				cps = (int) ceil ((bytes_sent - view->pos) / ((now - start) / 1000000.0));
				view->finalRateAverage = cps;
				last = now;
				period = 0;
				hit = true;
			}

			view->UpdateBar (sent, cps, bytes_sent, hit);
		}

		off_t size;
		view->file.GetSize (&size);
		view->success = bytes_sent == size;
	}
	
	view->file.Unset();	
	if (view->running)
	{
		view->Stopped();
	}
	return 0;
}

void
DCCSend::Lock (void)
{
	acquire_sem (sid);
}

void
DCCSend::Unlock (void)
{
	release_sem (sid);
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
