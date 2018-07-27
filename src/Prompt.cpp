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
#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Handler.h>
#include <TextControl.h>

#include "Prompt.h"
#include "VisionBase.h"

PromptValidate::PromptValidate()
{
}

PromptValidate::~PromptValidate()
{
}

PromptWindow::PromptWindow(BPoint point, const char* label, const char* title, const char* text,
						   BHandler* handler_, BMessage* msg, PromptValidate* validate_,
						   bool blanks_)

	: BWindow(BRect(point.x, point.y,
					point.x + 10, // To be resized
					point.y + 10),
			  title, B_FLOATING_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS
			  | B_AUTO_UPDATE_SIZE_LIMITS),

	  handler(handler_),
	  invoked(msg),
	  validate(validate_),
	  blanks(blanks_)
{
	float width;
	field = new BTextControl("field", label ? label : "", text ? text : "", 0);
	field->TextView()->AddFilter(new EscapeFilter(this));

	done = new BButton("Done", "Done", new BMessage(M_PROMPT_DONE));

	cancel = new BButton("Cancel", "Cancel", new BMessage(M_PROMPT_CANCEL));

//	ResizeTo(width + 145, cancel->Frame().bottom + 10);
	BLayoutBuilder::Grid<>(this)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(field->CreateLabelLayoutItem(), 0, 0)
			.Add(field->CreateTextViewLayoutItem(), 1, 0, 2)
			.Add(cancel, 1, 1)
			.Add(done, 2, 1)
			.End();

	done->MakeDefault(true);
	field->MakeFocus(true);
}

PromptWindow::~PromptWindow()
{
	delete invoked;

	if (validate) delete validate;
}

void PromptWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_PROMPT_DONE:

		if (field->TextView()->TextLength() || blanks) {
			if (!validate || validate->Validate(field->TextView()->Text())) {
				BMessenger msgr(handler);

				invoked->AddString("text", field->TextView()->Text());
				msgr.SendMessage(invoked);
			} else
				break;
		}

	case M_PROMPT_CANCEL:

		PostMessage(B_QUIT_REQUESTED);
		break;

	default:
		BWindow::MessageReceived(msg);
	}
}

EscapeFilter::EscapeFilter(BWindow* window_)
	: BMessageFilter(B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE, B_KEY_DOWN), window(window_)
{
}

EscapeFilter::~EscapeFilter()
{
}

filter_result EscapeFilter::Filter(BMessage* msg, BHandler**)
{
	const char* bytes;
	uint32 keyModifiers;

	msg->FindString("bytes", &bytes);
	msg->FindInt32("modifiers", (int32*)&keyModifiers);

	if (bytes[0] == B_ESCAPE && (keyModifiers & B_SHIFT_KEY) == 0 &&
		(keyModifiers & B_CONTROL_KEY) == 0 && (keyModifiers & B_OPTION_KEY) == 0 &&
		(keyModifiers & B_COMMAND_KEY) == 0) {
		window->PostMessage(B_QUIT_REQUESTED);
		return B_SKIP_MESSAGE;
	}

	return B_DISPATCH_MESSAGE;
}

RegExValidate::RegExValidate(const char* title_) : compiled(false), title(title_)
{
	memset(&re, 0, sizeof(re));
}

RegExValidate::~RegExValidate()
{
	if (compiled)
		regfree(&re);
}

bool RegExValidate::Validate(const char* text)
{
	if (compiled) {
		regfree(&re);
		memset(&re, 0, sizeof(re));
	}

	compiled = true;

	int errcode(regcomp(&re, text, REG_EXTENDED | REG_ICASE | REG_NOSUB));

	if (errcode) {
		char* buffer;
		size_t size;

		size = regerror(errcode, &re, 0, 0);
		buffer = new char[size];
		regerror(errcode, &re, buffer, size);

		BString aText;

		aText << title << " regular expression could not compile.\n\n";
		aText << buffer;
		delete[] buffer;

		(new BAlert("Cannot compile", aText.String(), "Whoops", 0, 0, B_WIDTH_AS_USUAL,
					B_STOP_ALERT))->Go();

		return false;
	}

	return true;
}
