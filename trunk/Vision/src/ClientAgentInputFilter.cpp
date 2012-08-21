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
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Jamie Wilkinson
 */

#include <Alert.h>
#include <Clipboard.h>
#include <NodeInfo.h>
#include <ScrollView.h>
#include <String.h>

#include <stdio.h>

#include "ClientAgent.h"
#include "ClientAgentInputFilter.h"
#include "ClientWindow.h"
#include "RunView.h"
#include "Vision.h"
#include "VisionBase.h"
#include "VTextControl.h"
#include "WindowList.h"

ClientAgentInputFilter::ClientAgentInputFilter (ClientAgent *agent)
		: BMessageFilter (B_ANY_DELIVERY, B_ANY_SOURCE),
		fWindow (agent),
		fHandledDrop (false)
{}


filter_result
ClientAgentInputFilter::Filter (BMessage *msg, BHandler **target)
{
	filter_result result (B_DISPATCH_MESSAGE);
	switch (msg->what)
	{
	case B_MOUSE_MOVED:
		break;

	case B_COPY:
		{
			int32 start, finish;
			fWindow->fInput->TextView()->GetSelection (&start, &finish);
			if (start == finish)
				*target = fWindow->fText;
		}
		break;

	case B_SELECT_ALL:
		{
			if (fWindow->fInput->TextView()->TextLength() == 0)
				*target = fWindow->fText;
		}
		break;

	case B_KEY_DOWN:
		{
			result = HandleKeys (msg);
		}
		break;

	case B_MOUSE_UP:
		{
			if (fHandledDrop)
			{
				fHandledDrop = false;
				result = B_SKIP_MESSAGE;
			}
		}
		break;

	case B_MIME_TYPE:
		{
			if (msg->HasData ("text/plain", B_MIME_TYPE))
			{
				const char *buffer;
				ssize_t size;

				msg->FindData (
						"text/plain",
						B_MIME_TYPE,
						0,
						reinterpret_cast<const void **>(&buffer),
						&size);

				// We copy it, because B_MIME_TYPE
				// might not be \0 terminated

				BString string;

				string.Append (buffer, size);
				HandleDrop (string.String());

				fHandledDrop = true;
				result = B_SKIP_MESSAGE;
			}
		}
		break;

	case B_SIMPLE_DATA:
		{
			if (msg->HasRef ("refs"))
			{
				for (int32 i = 0; msg->HasRef ("refs", i); ++i)
				{
					entry_ref ref;

					msg->FindRef ("refs", &ref);

					char mime[B_MIME_TYPE_LENGTH];
					BFile file (&ref, B_READ_ONLY);
					BNodeInfo info (&file);
					off_t size;

					if (file.InitCheck()							 == B_NO_ERROR
									&&	file.GetSize (&size)					 == B_NO_ERROR
									&&	info.InitCheck()							 == B_NO_ERROR
									&&	info.GetType (mime)						== B_NO_ERROR
									&&	strncasecmp (mime, "text/", 5) == 0)
					{
						char *buffer (new char [size + 1]);

						if (buffer)
						{
							// Oh baby!
							file.Read (buffer, size);
							buffer[size] = 0;

							HandleDrop (buffer);
							delete [] buffer;
							break;
						}
					}
				}

				// Give the fWindow a chance to handle non
				// text files.	If it's a message window, it'll
				// kick off a dcc send
				fWindow->DroppedFile (msg);
			}
		}
		break;

	case B_PASTE:
		{
			// we have our own pasting code so we can catch multiple lines
			BClipboard clipboard ("system");
			const char *fText;
			ssize_t textLen;
			BMessage *clip (NULL);

			if (clipboard.Lock())
			{
				if ((clip = clipboard.Data()))
					if (clip->FindData ("text/plain", B_MIME_TYPE,
									(const void **)&fText, &textLen) != B_OK)
					{
						clipboard.Unlock();
						break;
					}
			}

			clipboard.Unlock();
			BString data (fText, textLen);
			HandleDrop (data.String());
			result = B_SKIP_MESSAGE;
		}
		break;

	case B_MOUSE_WHEEL_CHANGED:
		{
			// pass this msg to IRCView
			fWindow->fText->MessageReceived (msg);
			result = B_SKIP_MESSAGE;
		}
		break;


	default:
		{
			//printf ("FILTER UNHANDLED: ");
			//msg->PrintToStream();
		}

		break;
	}

	return result;
}

filter_result
ClientAgentInputFilter::HandleKeys (BMessage *msg)
{
	filter_result result (B_DISPATCH_MESSAGE);
	const char *keyStroke;
	int32 keymodifiers;

	BMessenger msgr (fWindow);

		WindowList *winList (vision_app->pClientWin()->pWindowList());

	msg->FindString ("bytes", &keyStroke);
	msg->FindInt32 ("modifiers", &keymodifiers);

	if (keyStroke == NULL)
	{
		return result;
	}

	switch (keyStroke[0])
	{
		/////////////////
		/// catch all ///
		/////////////////

	case B_RETURN:
		{
			// we dont want Shift+B_RETURN to select all the text
			// treat keypress like we would a normal B_RETURN
			if (fWindow->fInput->TextView()->TextLength())
			{
				BMessage message (M_SUBMIT);
				message.AddString ("input", fWindow->fInput->TextView()->Text());
				msgr.SendMessage (&message);
			}
			result = B_SKIP_MESSAGE;
		}
		break;

	case B_TAB: // tab key
		{
						if ((keymodifiers & B_OPTION_KEY)	== 0
					&&	(keymodifiers & B_COMMAND_KEY) == 0
					&&	(keymodifiers & B_CONTROL_KEY) == 0
					&&	(keymodifiers & B_SHIFT_KEY) == 0)
					{
					// used for tabcompletion for nickname/channelname/etc
				fWindow->TabExpansion();
					BMessage logMessage (M_CLIENT_LOG);
				logMessage.AddString ("name", fWindow->fId.String());
					logMessage.AddString ("data", "DEBUG: Tab completion used\n");
					fWindow->fSMsgr.SendMessage (&logMessage);
			}
			if ((keymodifiers & B_SHIFT_KEY) || !(keymodifiers & B_CONTROL_KEY))
				result = B_SKIP_MESSAGE;
		}
		break;
	}

	if ((keymodifiers & B_OPTION_KEY)	== 0
					&&	(keymodifiers & B_COMMAND_KEY) != 0
					&&	(keymodifiers & B_CONTROL_KEY) == 0
					&&	(keymodifiers & B_SHIFT_KEY) != 0)
	{
		switch (keyStroke[0])
			{
				case '0':
				case B_INSERT:
					// switch to last active agent
					winList->SelectLast();
			result = B_SKIP_MESSAGE;
					break;

				case B_UP_ARROW:
				case B_LEFT_ARROW: // baxter muscle memory
				case ',': // bowser muscle memory
					winList->ContextSelectUp();
			result = B_SKIP_MESSAGE;
					break;

				case B_DOWN_ARROW: //
				case B_RIGHT_ARROW: // baxter muscle memory
				case '.': // bowser muscle memory
					winList->ContextSelectDown();
			result = B_SKIP_MESSAGE;
					break;

				case 'U':
					winList->MoveCurrentUp();
			result = B_SKIP_MESSAGE;
					break;

				case 'D':
					winList->MoveCurrentDown();
			result = B_SKIP_MESSAGE;
					break;
			}
		}

	else if ((keymodifiers & B_OPTION_KEY)	== 0
			 &&	(keymodifiers & B_COMMAND_KEY) != 0
			 &&	(keymodifiers & B_CONTROL_KEY) == 0
			 &&	(keymodifiers & B_SHIFT_KEY) == 0)
	{
		///////////////
		/// Command ///
		///////////////
		switch (keyStroke[0])
		{
			case B_UP_ARROW:
			case ',': // bowser muscle memory
				{
				// move up one agent
				winList->Select (winList->CurrentSelection() - 1);
				winList->ScrollToSelection();
			result = B_SKIP_MESSAGE;
			}
				break;

			case B_DOWN_ARROW:
			case '.': // bowser muscle memory
				{
				// move down one agent
				winList->Select (winList->CurrentSelection() + 1);
				winList->ScrollToSelection();
			result = B_SKIP_MESSAGE;
			}
				break;

			case B_LEFT_ARROW: // collapse current server (if expanded)
				{
				winList->CollapseCurrentServer();
			result = B_SKIP_MESSAGE;
			}
				break;

			case B_RIGHT_ARROW: // expand current server (if collapsed)
				{
				winList->ExpandCurrentServer();
			result = B_SKIP_MESSAGE;
			}
				break;

			case '/': // bowser muscle memory
				// move to the agents parent ServerAgent
				// XXX move to WindowList ?
				{
				winList->SelectServer();
			result = B_SKIP_MESSAGE;
			}
				break;
		}
	}

	if ((keymodifiers & B_OPTION_KEY)	== 0
					&&	(keymodifiers & B_COMMAND_KEY) == 0
					&&	(keymodifiers & B_CONTROL_KEY) == 0
					&&	(keymodifiers & B_SHIFT_KEY) == 0)
	{
		////////////////////
		/// no modifiers ///
		////////////////////
		switch (keyStroke[0])
		{
		case B_UP_ARROW:
			{
				// used for input history
				msgr.SendMessage (M_PREVIOUS_INPUT);
				result = B_SKIP_MESSAGE;
			}
			break;

		case B_DOWN_ARROW:
			{
				// used for input history
				msgr.SendMessage (M_NEXT_INPUT);
				result = B_SKIP_MESSAGE;
			}
			break;

		case B_PAGE_UP:
			{
				// scroll the IRCView
				BRect myrect (fWindow->fText->Bounds());
				float height (myrect.bottom - myrect.top - 10.0);

				if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() > height)
					fWindow->fText->ScrollBy (0.0, -1 * height);
				else
					fWindow->fText->ScrollTo (0.0, 0.0);

				result = B_SKIP_MESSAGE;
			}
			break;

		case B_PAGE_DOWN:
			{
				// scroll the IRCView
				BRect myrect (fWindow->fText->Bounds());
				float height (myrect.bottom - myrect.top - 10.0);

				float min, max;
				fWindow->fTextScroll->ScrollBar (B_VERTICAL)->GetRange (&min, &max);
				if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() != max)
					fWindow->fText->ScrollBy (0.0, height);
				result = B_SKIP_MESSAGE;
			}
			break;

		case B_ESCAPE:
				{
			fWindow->fCancelMLPaste = true;
			result = B_SKIP_MESSAGE;
			}
			break;
		}
	}

	else if ((keymodifiers & B_OPTION_KEY)	== 0
					&&	(keymodifiers & B_COMMAND_KEY) == 0
					&&	(keymodifiers & B_CONTROL_KEY) != 0
					&&	(keymodifiers & B_SHIFT_KEY) == 0)
	{
		////////////
		/// Ctrl ///
		////////////
		switch (keyStroke[0])
		{
		case B_UP_ARROW:
			{
				// scroll the IRCView up by 1 line
				if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() != 0)
				{
					fWindow->fText->ScrollBy (0.0, vision_app->GetClientFont(F_TEXT)->Size() * -1);
					result = B_SKIP_MESSAGE;
				}
			}
			break;

		case B_DOWN_ARROW:
			{
				// scroll the IRCView down by 1 line
				float min, max;
				fWindow->fTextScroll->ScrollBar (B_VERTICAL)->GetRange (&min, &max);
				if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() != max)
				{
					fWindow->fText->ScrollBy (0.0, vision_app->GetClientFont(F_TEXT)->Size());
					result = B_SKIP_MESSAGE;
				}
			}
			break;

		case B_HOME:
			{
				// scroll to the beginning of the IRCView
				fWindow->fText->ScrollTo (0.0, 0.0);
				result = B_SKIP_MESSAGE;
			}
			break;

		case B_END:
			{
				// scroll to the end of the IRCView
				float min, max;
				fWindow->fTextScroll->ScrollBar (B_VERTICAL)->GetRange (&min, &max);
				fWindow->fText->ScrollTo (0.0, max);
				result = B_SKIP_MESSAGE;
			}
			break;

		case B_PAGE_UP:
		case B_PAGE_DOWN:
			{
				// scroll the IRCView
				BRect myrect (fWindow->fText->Bounds());
				float height (myrect.bottom - myrect.top);

				if (keyStroke[0] == B_PAGE_UP)
				{
					if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() > height)
						fWindow->fText->ScrollBy (0.0, -1 * height);
					else
						fWindow->fText->ScrollTo (0.0, 0.0);
				}
				else // B_PAGE_DOWN
				{
					float min, max;
					fWindow->fTextScroll->ScrollBar (B_VERTICAL)->GetRange (&min, &max);
					if (fWindow->fTextScroll->ScrollBar (B_VERTICAL)->Value() != max)
						fWindow->fText->ScrollBy (0.0, height);
				}

				result = B_SKIP_MESSAGE;
			}
			break;

			// ctrl+u = special control char - ascii 21
		case 21:
			{
				if (fWindow->fInput->TextView()->TextLength())
				{
					int32 selstart, selfinish;
					fWindow->fInput->TextView()->GetSelection (&selstart, &selfinish);
					fWindow->fInput->TextView()->Delete (0,
									selfinish);
				}
				result = B_SKIP_MESSAGE;
			}
			break;

		}
	}
	return result;
}

void
ClientAgentInputFilter::HandleDrop (const char *buffer)
{
	BMessage msg (M_SUBMIT_INPUT);
	const char *place;
	int32 lines (0);

	BMessenger msgr (fWindow);

	while ((place = strchr (buffer, '\n')))
	{
		BString str;

		str.Append (buffer, place - buffer);
		msg.AddString ("data", str.String());
		++lines;
		buffer = place + 1;
	}

	if (*buffer)
	{
		msg.AddString ("data", buffer);
		++lines;
	}

	int32 start, finish;
	fWindow->fInput->TextView()->GetSelection (&start, &finish);
	msg.AddInt32 ("selstart", start);
	msg.AddInt32 ("selend", finish);

	if (lines > 1)
	{
		if (true == vision_app->GetBool("Newbie Spam Mode"))
		{
			BString str;

			str += "As if there isn't enough, you ";
			str += "are about to add ";
			str << lines;
			str += " more lines of spam to ";
			str += "the internet.	How ";
			str += "would you like to go about this?";

			BAlert *alert (new BAlert (
									"Spam",
									str.String(),
									"Cancel",
									"Spam!",
									"Single Line",
									B_WIDTH_FROM_WIDEST,
									B_OFFSET_SPACING,
									B_WARNING_ALERT));

			BMessage *invokeMsg (new BMessage (msg));
			BInvoker *invoker (new BInvoker (invokeMsg, msgr));
			invokeMsg->AddPointer ("invoker", invoker);
			alert->Go (invoker);
		}
		else
		{
			msg.AddInt32 ("which", PASTE_MULTI);
			msgr.SendMessage (&msg);
		}
	}

	if (lines == 1)
	{
		msg.AddInt32 ("which", PASTE_SINGLE);
		msgr.SendMessage (&msg);
		fWindow->fInput->MakeFocus(false);
		fWindow->fInput->MakeFocus(true);
	}
}

