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
 *                 Wade Majors
 *                 Todd Lair
 */

#include <InterfaceDefs.h>
#include <Message.h>
#include <ctype.h>
#include "NumericFilter.h"

NumericFilter::NumericFilter (void)
    : BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE)
{
}

NumericFilter::~NumericFilter (void)
{
}

filter_result 
NumericFilter::Filter (BMessage *msg, BHandler **)
{
  filter_result result = B_DISPATCH_MESSAGE;
  switch (msg->what)
  {
    case B_KEY_DOWN:
    {
      uint32 modifier (msg->FindInt32 ("modifiers"));
      const char *bytes (msg->FindString ("bytes"));
      
      if (!modifier)
      {
        if (bytes[0] != B_ENTER
        &&  bytes[0] != B_BACKSPACE
        &&  !isdigit (bytes[0]))
          result = B_SKIP_MESSAGE;
      }
      else if (modifier & B_SHIFT_KEY)
      {
        if (bytes[0] == B_LEFT_ARROW
        ||  bytes[0] == B_RIGHT_ARROW
        ||  bytes[0] == B_HOME
        ||  bytes[0] == B_END)
          break;
        else
          result = B_SKIP_MESSAGE;
      }
    }
    break;
  }
  return result;
}
