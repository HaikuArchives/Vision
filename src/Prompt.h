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

#ifndef PROMPTWINDOW_H_
#define PROMPTWINDOW_H_

#include <MessageFilter.h>
#include <String.h>
#include <Window.h>

#include <regex.h>
class BTextControl;
class BButton;

class PromptValidate {
public:
	PromptValidate();
	virtual ~PromptValidate();
	virtual bool Validate(const char*) = 0;
};

class RegExValidate : public PromptValidate {
	regex_t re;
	bool compiled;
	BString title;

public:
	RegExValidate(const char*);
	virtual ~RegExValidate();
	virtual bool Validate(const char*);
};

class PromptWindow : public BWindow {
	BHandler* handler;
	BMessage* invoked;

	BTextControl* field;
	BButton *done, *cancel;
	PromptValidate* validate;
	bool blanks;

public:
	PromptWindow(BPoint, const char*, const char*, const char*, BHandler*, BMessage*,
		PromptValidate* = 0, bool = false);

	virtual ~PromptWindow();
	virtual void MessageReceived(BMessage*);
};

class EscapeFilter : public BMessageFilter {
	BWindow* window;

public:
	EscapeFilter(BWindow*);
	virtual ~EscapeFilter();
	virtual filter_result Filter(BMessage*, BHandler**);
};

#endif
