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

#include "Vision.h"
#include "DCCConnect.h"
#include "DCCFileWindow.h"

DCCFileWindow::DCCFileWindow (DCCConnect *view)
	: BWindow (
			BRect (50, 50, 100, 100),
			"DCC Transfers",
			B_TITLED_WINDOW,
			B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_NOT_MINIMIZABLE |
			B_NOT_CLOSABLE | B_ASYNCHRONOUS_CONTROLS)
{
	AddChild (view);
	ResizeTo (
		view->Bounds().Width(),
		view->Bounds().Height());
}

DCCFileWindow::~DCCFileWindow (void)
{
}

bool
DCCFileWindow::QuitRequested (void)
{
	be_app->PostMessage (M_DCC_FILE_WIN_DONE);
	return true;
}

void
DCCFileWindow::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_DCC_FILE_WIN:
		{
			BRect bounds (Bounds());
			BPoint point (0.0, 0.0);
			DCCConnect *view;

			if (ChildAt (0))
			{
				point.y = bounds.bottom + 1;
			}
						
			msg->FindPointer ("view", reinterpret_cast<void **>(&view));
			view->MoveTo (point);
			AddChild (view);
			ResizeTo (bounds.Width(), view->Frame().bottom);
			if (IsHidden())
			{
				Show();
			}
		}
		break;

		case M_DCC_FINISH:
		{
			bool success, stopped;

			msg->FindBool ("success", &success);
			msg->FindBool ("stopped", &stopped);

			if (success || stopped)
			{
				BView *view;
				
				msg->FindPointer ("source", reinterpret_cast<void **>(&view));
				
				for (int32 i = 0; i < CountChildren(); ++i)
				{
					BView *child (ChildAt (i));
					
					if (child == view)
					{
						float height (view->Frame().Height());
						RemoveChild (view);
						delete view;
						for (int32 j = i; j < CountChildren(); ++j)
						{
							child = ChildAt (j);
							child->MoveBy (0.0, -(height + 1));
						}
						
						if (CountChildren() == 0)
						{
							BMessenger(this).SendMessage(B_QUIT_REQUESTED);
						}
						else
						{
							ResizeBy (0.0, -(height + 1));
						}
						break;
					}
				}
			}
		}
		break;

		case M_ADD_RESUME_DATA:
		{
			const char *nick (NULL);
			const char *file (NULL);
			const char *port (NULL);
			bool hit (false);
			off_t pos (0);
			
			msg->FindString ("vision:nick", &nick);
			msg->FindString ("vision:file", &file);
			msg->FindString ("vision:port", &port);
			msg->FindInt64	("vision:pos", &pos);
			
			for (int32 i = 0; i < CountChildren(); ++i)
			{
				DCCSend *view (dynamic_cast<DCCSend *>(ChildAt (i)));
				if (view && view->IsMatch (nick, port))
				{
					view->SetResume (pos);
					hit = true;
					break;
				}
			}
			
			BMessage reply (B_REPLY);
			reply.AddBool ("hit", hit);
			msg->SendReply (&reply);
		}
		break;

		default:
			BWindow::MessageReceived (msg);
	}
}

void
DCCFileWindow::Hide (void)
{
	// we do this to keep it out of the deskbar
	SetFlags (Flags() | B_AVOID_FOCUS);
	BWindow::Hide();
}

void
DCCFileWindow::Show (void)
{
	SetFlags (Flags() & ~(B_AVOID_FOCUS));
	SetWorkspaces (B_CURRENT_WORKSPACE);
	BWindow::Show();
}
