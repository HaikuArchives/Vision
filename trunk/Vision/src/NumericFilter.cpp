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

#include <InterfaceDefs.h>
#include <Message.h>
#include <ctype.h>
#include "NumericFilter.h"
#include <TextView.h>
#include <Invoker.h>

#include <stdio.h>

NumericFilter::NumericFilter (void)
		: BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE)
{
}

NumericFilter::~NumericFilter (void)
{
}

filter_result 
NumericFilter::Filter (BMessage *msg, BHandler **handler)
{
	filter_result result = B_DISPATCH_MESSAGE;
	switch (msg->what)
	{
		case B_KEY_DOWN:
		{
			uint32 modifier (msg->FindInt32 ("modifiers"));
			const char *bytes (msg->FindString ("bytes"));

			if ((modifier & B_SHIFT_KEY) == 0
				&& (modifier & B_CONTROL_KEY) == 0
				&& (modifier & B_OPTION_KEY) == 0
				&& (modifier & B_COMMAND_KEY) == 0)
			{
				switch (bytes[0])
				{
					case B_BACKSPACE:
					case B_LEFT_ARROW:
					case B_RIGHT_ARROW:
					case B_HOME:
					case B_END:
					case B_TAB:
						break;

					case B_ENTER:
						// for some stupid reason BTextControl on R5 does not always Invoke() when
						// you hit enter, so forcing the issue here.
						if (dynamic_cast<BTextView *>(*handler))
						{
							dynamic_cast<BInvoker *>(((BView *)*handler)->Parent())->Invoke();
						}
						break;
						
					default:
						if (!isdigit(bytes[0]))
							result = B_SKIP_MESSAGE;
						break;
				}
			}
			else if ((modifier & B_SHIFT_KEY) != 0)
			{
				if (bytes[0] == B_LEFT_ARROW
				||	bytes[0] == B_RIGHT_ARROW
				||	bytes[0] == B_HOME
				||	bytes[0] == B_END
				||	bytes[0] == B_TAB)
					break;
				else
					result = B_SKIP_MESSAGE;
			}
		}
		break;
	}
	return result;
}
