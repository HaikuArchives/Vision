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
 */

#include <MenuItem.h>
#include <Messenger.h>

#include "NetworkMenu.h"
#include "Vision.h"

NetworkMenu::NetworkMenu (const char *title, uint32 what, BMessenger msgTarget)
	: BMenu (title),
		fMsgConst (what),
		fTarget (msgTarget)
{
}

NetworkMenu::~NetworkMenu (void)
{
}

void
NetworkMenu::AttachedToWindow (void)
{
	if (CountItems())
	{
		BMenuItem *item (NULL);
		while ((item = RemoveItem((int32)0)) != NULL)
			delete item;
	}

	BMessage msg;
	BMessage *invoke (NULL);
	for (int32 i = 0; (msg = vision_app->GetNetwork (i)), !msg.HasBool ("error"); i++)
	{
		const char *name = msg.FindString ("name");
		invoke = new BMessage (fMsgConst);
		invoke->AddString ("network", name);
		BMenuItem *item (new BMenuItem(name, invoke));
		AddItem (item);
		if (!vision_app->CheckNetworkValid (name))
			item->SetEnabled (false);
	}
	SetTargetForItems (fTarget.Target(NULL));
	BMenu::AttachedToWindow();
}

