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
 */

#ifdef GNOME_BUILD
#  include "gnome/AppFileInfo.h"
#  include "gnome/Alert.h"
#  include "gnome/Invoker.h"
#elif BEOS_BUILD
#  include <AppFileInfo.h>
#  include <Alert.h>
#  include <Invoker.h>
#endif

#include <stdlib.h>

#include "Vision.h"
#include "StringManip.h"
#include "ServerAgent.h"

void
ServerAgent::DCCChatDialog(BString theNick, BString theIP, BString thePort)
{
	BString theText(theNick);
	theText << " wants to begin a DCC chat with you.";
	BAlert *myAlert = new BAlert("DCC Request", theText.String(), "Accept",
		"Refuse");
	BMessage *myMessage = new BMessage(M_CHAT_ACCEPT);
	myMessage->AddString("nick", theNick.String());
	myMessage->AddString("ip", theIP.String());
	myMessage->AddString("port", thePort.String());
	BInvoker *myInvoker = new BInvoker(myMessage, this);
	myAlert->Go(myInvoker);
}
