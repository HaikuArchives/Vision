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
 * Contributor(s): Wade Majors <guru@startrek.com>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Jamie Wilkinson
 */
 
 
#include "MessageAgent.h"
#include "Vision.h"


MessageAgent::MessageAgent (
  BRect &frame_,
  const char *id_,
  int32 sid_,
  const char *serverName_,
  const BMessenger &sMsgr_,
  const char *nick,
  const char *addyString,
  bool chat,
  bool initiate,
  const char *IP,
  const char *port)

  : ClientAgent (
    id_,
    sid_,
    serverName_,
    nick,
    sMsgr_,
    frame_),

  chatAddy (addyString ? addyString : ""),
  chatee (id_),
  dIP (IP),
  dPort (port),
  dChat (chat),
  dInitiate (initiate),
  dConnected (false)
{
 //
}


MessageAgent::~MessageAgent (void)
{
  //
}


void
MessageAgent::MessageReceived (BMessage *msg)
{
  ClientAgent::MessageReceived (msg);
}

void
MessageAgent::Show (void)
{
  ClientAgent::Show();
}

void
MessageAgent::Parser (const char *)
{
  //
}

void
MessageAgent::DroppedFile (BMessage *drop)
{
  //
}

void
MessageAgent::TabExpansion (void)
{
 //
}