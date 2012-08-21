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
 */

#include "ChannelAgent.h"
#include "ChannelOptions.h"
#include "VisionBase.h"

#include <Catalog.h>
#include <StringView.h>

#include <stdio.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChannelOptions"

ChannelOptions::ChannelOptions (const char *chan_name_, ChannelAgent *parent_)
	: BWindow (
			BRect (188.0, 88.0, 600.0, 390.0),
			"",
			B_TITLED_WINDOW,
			B_ASYNCHRONOUS_CONTROLS | B_NOT_RESIZABLE | B_NOT_ZOOMABLE),
	parent (parent_),
	chan_name (chan_name_)
{
	Init();
}


ChannelOptions::~ChannelOptions (void)
{
	//
}

bool
ChannelOptions::QuitRequested (void)
{
	parent->fMsgr.SendMessage (M_CHANNEL_OPTIONS_CLOSE);
	return true;	
}

void
ChannelOptions::Init (void)
{
	BString temp = B_TRANSLATE("%1 options");
	temp.ReplaceFirst("%1", chan_name);
	SetTitle (temp.String());
	
	bgView = new BView (Bounds(),
											"Background",
											B_FOLLOW_ALL_SIDES,
											B_WILL_DRAW);

	bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	AddChild (bgView);

	BStringView *tempStringView = new BStringView (Bounds(),
																								 "temp",
																								 "AEIOUglqj",
																								 0,
																								 0);
	tempStringView->ResizeToPreferred();
	float stringHeight = tempStringView->Frame().bottom;
	
	delete tempStringView;

	
	privilegesView = new BView (BRect (bgView->Frame().left + 2,
																		 bgView->Frame().top + 2,
																		 bgView->Frame().right - 2,
																		 stringHeight+2),
														 "privilege message",
														 0,
														 B_WILL_DRAW);
	privilegesView->SetViewColor (0, 100, 0);
	bgView->AddChild (privilegesView);

	
	BString privString;	// this will become dynamic based on the current mode
	privString += B_TRANSLATE("You are currently a channel operator.");
	privString += " ";
	privString += B_TRANSLATE("You may change any of these options.");
	
	BStringView *privMsgView = new BStringView (BRect (privilegesView->Bounds().left,
																										 privilegesView->Bounds().top,
																										 privilegesView->Bounds().right,
																										 stringHeight),
																							"privMsgView",
																							privString.String(),
																							0,
																							B_WILL_DRAW);
	privMsgView->SetHighColor (255,255,255);
	privMsgView->SetAlignment (B_ALIGN_CENTER);
	privilegesView->ResizeToPreferred();
	privilegesView->AddChild (privMsgView);
	
	
	
}

