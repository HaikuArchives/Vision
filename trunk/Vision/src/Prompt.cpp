
#include <Application.h>
#include <Handler.h>
#include <Button.h>
#include <Alert.h>

#include "VTextControl.h"
#include "Prompt.h"

const uint32 M_DONE							= 'done';
const uint32 M_CANCEL						= 'cncl';

PromptValidate::PromptValidate (void)
{
}

PromptValidate::~PromptValidate (void)
{
}


PromptWindow::PromptWindow (
	BPoint point,
	const char *label,
	const char *title,
	const char *text,
	BHandler *handler_,
	BMessage *msg,
	PromptValidate *validate_,
	bool blanks_)

	: BWindow (
		BRect (
			point.x,
			point.y,
			point.x + 10,		// To be resized
			point.y + 10),
		title,
		B_FLOATING_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS),

		handler (handler_),
		invoked (msg),
		validate (validate_),
		blanks (blanks_)
{
	BRect frame (Bounds());
	float width;

	BView *bgView (new BView (
		frame,
		"Background",
		B_FOLLOW_ALL_SIDES,
		B_WILL_DRAW));

	bgView->SetViewColor (222, 222, 222, 255);
	AddChild (bgView);

	field = new VTextControl (
		BRect (
			10,
			10,
			(width = be_plain_font->StringWidth (label ? label : "")) + 135,
			25),
		"field",
		label ? label : "",
		text ? text : "",
		0,
		B_FOLLOW_LEFT | B_FOLLOW_TOP);

	field->SetDivider (width + 5);
	bgView->AddChild (field);

	field->TextView()->AddFilter (new EscapeFilter (this));

	done = new BButton (
		BRect (
			frame.right - 20,
			field->Frame().bottom + 10,
			frame.right - 10,
			field->Frame().bottom + 30),
		"Done",
		"Done",
		new BMessage (M_DONE),
		B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	done->ResizeToPreferred();
	done->MoveTo (
		frame.right - 10 - done->Frame().Width(),
		done->Frame().top);
	bgView->AddChild (done);

	cancel = new BButton (
		BRect (
			done->Frame().left - 20,
			field->Frame().bottom + 10,
			done->Frame().left - 10,
			field->Frame().bottom + 30),
		"Cancel",
		"Cancel",
		new BMessage (M_CANCEL),
		B_FOLLOW_RIGHT | B_FOLLOW_TOP);
	cancel->ResizeToPreferred();
	cancel->MoveTo (
		done->Frame().left - 10 - cancel->Frame().Width(),
		cancel->Frame().top);
	bgView->AddChild (cancel);

	ResizeTo (width + 145, cancel->Frame().bottom + 10);
	done->MakeDefault (true);
	field->MakeFocus (true);
}

PromptWindow::~PromptWindow (void)
{
	delete invoked;

	if (validate) delete validate;
}

void
PromptWindow::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_DONE:

			if (field->TextView()->TextLength() || blanks)
			{
				if (!validate
				||   validate->Validate (field->TextView()->Text()))
				{
					BMessenger msgr (handler);

					invoked->AddString ("text", field->TextView()->Text());
					msgr.SendMessage (invoked);
				}
				else break;
			}

		case M_CANCEL:

			PostMessage (B_QUIT_REQUESTED);
			break;

		default:
			BWindow::MessageReceived (msg);
	}
}

		

EscapeFilter::EscapeFilter (BWindow *window_)
	: BMessageFilter (B_PROGRAMMED_DELIVERY, B_LOCAL_SOURCE, B_KEY_DOWN),
	  window (window_)
{
}

EscapeFilter::~EscapeFilter (void)
{
}

filter_result
EscapeFilter::Filter (BMessage *msg, BHandler **)
{
	const char *bytes;
	uint32 keyModifiers;

	msg->FindString ("bytes", &bytes);
	msg->FindInt32 ("modifiers", (int32 *)&keyModifiers);

	if (bytes[0] == B_ESCAPE
	&& (keyModifiers & B_SHIFT_KEY)   == 0
	&& (keyModifiers & B_CONTROL_KEY) == 0
	&& (keyModifiers & B_OPTION_KEY)  == 0
	&& (keyModifiers & B_COMMAND_KEY) == 0)
	{
		window->PostMessage (B_QUIT_REQUESTED);
		return B_SKIP_MESSAGE;
	}

	return B_DISPATCH_MESSAGE;
}

RegExValidate::RegExValidate (const char *title_)
	: compiled (false),
	  title (title_)
{
	memset (&re, 0, sizeof (re));
}

RegExValidate::~RegExValidate (void)
{
	if (compiled) regfree (&re);
}

bool
RegExValidate::Validate (const char *text)
{
	if (compiled)
	{
		regfree (&re);
		memset (&re, 0, sizeof (re));
	}

	compiled = true;

	int errcode (regcomp (
		&re,
		text,
		REG_EXTENDED | REG_ICASE | REG_NOSUB));

	if (errcode)
	{
		char *buffer;
		size_t size;

		size = regerror (errcode, &re, 0, 0);
		buffer = new char [size];
		regerror (errcode, &re, buffer, size);

		BString aText;

		aText << title << " regular expression could not compile.\n\n";
		aText << buffer;
		delete [] buffer;

		(new BAlert (
			"Cannot compile",
			aText.String(),
			"Whoops",
			0,
			0,
			B_WIDTH_AS_USUAL,
			B_STOP_ALERT))->Go();

		return false;
	}

	return true;
}
