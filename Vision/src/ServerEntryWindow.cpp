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
 * Contributor(s): Rene Gollent
 */

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <MenuField.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Window.h>

#include "NumericFilter.h"
#include "ServerEntryWindow.h"
#include "Vision.h"
#include "VTextControl.h"

#include <stdio.h>
#include <stdlib.h>


ServerEntryWindow::ServerEntryWindow (BHandler *handler, BMessage *invoked, const ServerData *data, int32 size)
  : BWindow (
        BRect (50, 50, 350, 250), 
        S_SERVERWIN_TITLE, 
        B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS)
{
  AddChild (new ServerEntryView (Bounds(), handler, invoked, data, size));
}

ServerEntryWindow::~ServerEntryWindow (void)
{
}

ServerEntryView::ServerEntryView (BRect bounds, BHandler *handler, BMessage *invoked, const ServerData *data, int32 size)
  : BView (
      bounds,
      "entryView",
      B_FOLLOW_ALL_SIDES,
      B_WILL_DRAW),
        invocation (invoked),
        target (handler)
{
  ASSERT (handler != NULL);
  memset(&currentServer, 0, sizeof(ServerData));
  if (size != 0)
    memcpy(&currentServer, data, size);
  SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
  serverName = new VTextControl (BRect (0,0,0,0), "serverName", S_SERVERWIN_SERVER,
    (data) ? data->serverName : "", new BMessage (M_SERVER_NAME_CHANGED),
    B_FOLLOW_LEFT | B_FOLLOW_TOP);
  BString strPort ("");
  if (data) strPort << data->port;
  else strPort << 6667;
  port = new VTextControl (BRect (0,0,0,0), "portVal", S_SERVERWIN_PORT,
    strPort.String(), new BMessage (M_SERVER_PORT_CHANGED),
    B_FOLLOW_LEFT | B_FOLLOW_TOP);
  port->SetDivider (be_plain_font->StringWidth ("Port: ") + 5);

  BMenu *stateMenu = new BMenu (S_SERVERWIN_MENU1);
  stateMenu->AddItem (new BMenuItem (S_SERVERWIN_MENU_PRI, new BMessage (M_SERVER_STATE)));
  stateMenu->AddItem (new BMenuItem (S_SERVERWIN_MENU_SEC, new BMessage (M_SERVER_STATE)));
  stateMenu->AddItem (new BMenuItem (S_SERVERWIN_MENU_DIS , new BMessage (M_SERVER_STATE)));
  statusField = new BMenuField (BRect (0,0,0,0), "states", S_SERVERWIN_STATE, stateMenu,
    B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);

  okButton = new BButton (BRect (0,0,0,0), "serverOk", S_SERVERWIN_DONE_BUTTON,
    new BMessage (M_SERVER_DONE), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
    
  cancelButton = new BButton (BRect (0,0,0,0), "serverCancel", S_SERVERWIN_CANCEL_BUTTON,
    new BMessage (M_SERVER_CANCEL), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);

  BString password ("");
  if (strlen(currentServer.password) > 0)
    password = currentServer.password;


  usePassword = new BCheckBox (BRect (0,0,0,0), "usePass", S_SERVERWIN_PASS_CHECK,
    new BMessage (M_SERVER_USEPASS), B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
  
  passwordField = new VTextControl (BRect (0,0,0,0), "password", NULL,
    password.String(), NULL, B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW | B_NAVIGABLE);
      

  AddChild (statusField);
  AddChild (serverName);
  AddChild (port);
  AddChild (okButton);
  AddChild (cancelButton);
  AddChild (usePassword);
  AddChild (passwordField);
}


ServerEntryView::~ServerEntryView (void)
{
	delete invocation;
}

void
ServerEntryView::AttachedToWindow (void)
{
  BView::AttachedToWindow();

  serverName->SetDivider (be_plain_font->StringWidth (S_SERVERWIN_SERVER) + 5);
  serverName->ResizeToPreferred();
  serverName->ResizeTo (Bounds().Width() / 2, serverName->Bounds().Height());
  serverName->MoveTo (10,10);
  serverName->SetTarget (this);
  
  port->ResizeToPreferred();
  port->MoveTo (serverName->Frame().right + 15, serverName->Frame().top);
  port->SetTarget (this);
  port->TextView()->AddFilter (new NumericFilter());
  
  float diff (0);
  if ((diff = (port->Frame().right - Bounds().right)) > 0.0)
     ResizeBy (diff, 0.0);
  
  statusField->ResizeToPreferred();     
  statusField->Menu()->SetTargetForItems (this);

  usePassword->ResizeToPreferred();
  usePassword->MoveTo (serverName->Frame().left, serverName->Frame().bottom + 15);
  usePassword->SetTarget(this);
  usePassword->SetValue ((strlen(currentServer.password) > 0) ? B_CONTROL_ON : B_CONTROL_OFF);
  passwordField->SetEnabled(usePassword->Value() == B_CONTROL_ON);
  passwordField->ResizeToPreferred();
  passwordField->ResizeTo(port->Frame().right - usePassword->Frame().right - 5, passwordField->Bounds().Height());
  passwordField->MoveTo (usePassword->Frame().right + 5, usePassword->Frame().top);
  passwordField->TextView()->HideTyping(true);
#if B_BEOS_VERSION_DANO
  statusField->MoveTo ((Bounds().Width() - statusField->Bounds().Width()) / 2.0,
#else
  statusField->MoveTo ((Bounds().Width() - statusField->Bounds().Width()) / 4.0,
#endif
  	usePassword->Frame().bottom + 15);
  
  cancelButton->SetTarget (this);
  okButton->SetTarget (this);
  okButton->ResizeToPreferred();
  okButton->MoveTo (port->Frame().right - okButton->Bounds().Width(), statusField->Frame().bottom + 30);
  cancelButton->ResizeToPreferred();
  cancelButton->MoveTo (okButton->Frame().left - (cancelButton->Frame().Width() + 5), okButton->Frame().top);

  okButton->SetEnabled (false);
    
  Window()->ResizeTo (okButton->Frame().right + 5, okButton->Frame().bottom + 10);

  dynamic_cast<BInvoker *>(statusField->Menu()->ItemAt(currentServer.state))->Invoke();
}

void
ServerEntryView::AllAttached (void)
{
  BView::AllAttached();
  port->MakeFocus (false);
  port->MakeFocus (true);
  serverName->MakeFocus(false);
  serverName->MakeFocus(true);
}

void
ServerEntryView::CheckDoneState (void)
{
	if (serverName->TextView()->TextLength() != 0)
		if (port->TextView()->TextLength() != 0)
			if (statusField->Menu()->FindMarked() != NULL)
			{
				okButton->SetEnabled (true);
				return;
			}
	okButton->SetEnabled (false);
}

void
ServerEntryView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_SERVER_STATE:
			{
				BMenu *menu (statusField->Menu());
				BMenuItem *item (NULL);
				item = menu->FindMarked();
				if (item)
					item->SetMarked (false);
				msg->FindPointer ("source", reinterpret_cast<void **>(&item));
				item->SetMarked (true);
				statusField->MenuItem()->SetLabel (item->Label());
			}

		case M_SERVER_NAME_CHANGED:
		case M_SERVER_PORT_CHANGED:
			CheckDoneState();
			break;
	    
	    case M_SERVER_USEPASS:
	      {
	        passwordField->SetEnabled (usePassword->Value() == B_CONTROL_ON);
	      }
	      break;      
		
		case M_SERVER_DONE:
			{
				BMenu *menu (statusField->Menu());
				ServerData data;
				memset (&data, 0, sizeof (ServerData));
				strcpy (data.serverName, serverName->Text());
				data.port = atoi (port->Text());
				data.state = menu->IndexOf(menu->FindMarked());
				if (usePassword->Value() == B_CONTROL_ON)
				  strcpy(data.password, passwordField->TextView()->Text());
				BMessenger msgr (target);
				BMessage msg (*invocation);
				msg.AddData ("server", B_RAW_TYPE, &data, sizeof(data));
				msgr.SendMessage (&msg);
			}
		
		case M_SERVER_CANCEL:
		 	Window()->PostMessage (B_QUIT_REQUESTED);
		 	break;
		
		default:
			BView::MessageReceived (msg);
	}
}
