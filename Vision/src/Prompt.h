
#ifndef PROMPTWINDOW_H_
#define PROMPTWINDOW_H_

#include <Window.h>
#include <MessageFilter.h>
#include <String.h>

#include <regex.h>

class VTextControl;
class BButton;

class PromptValidate
{
	public:

									PromptValidate (void);
	virtual						~PromptValidate (void);
	virtual bool				Validate (const char *) = 0;
};

class RegExValidate : public PromptValidate
{
	regex_t				re;
	bool					compiled;
	BString				title;

	public:

							RegExValidate (const char *);
	virtual				~RegExValidate (void);
	virtual bool		Validate (const char *);
};

class PromptWindow : public BWindow
{
	BHandler						*handler;
	BMessage						*invoked;

	VTextControl				*field;
	BButton						*done, *cancel;
	PromptValidate				*validate;
	bool							blanks;

	public:

									PromptWindow (
										BPoint,
										const char *,
										const char *,
										const char *,
										BHandler *,
										BMessage *,
										PromptValidate * = 0,
										bool = false);

	virtual						~PromptWindow (void);
	virtual void				MessageReceived (BMessage *);
};

class EscapeFilter : public BMessageFilter
{
	BWindow						*window;

	public:

									EscapeFilter (BWindow *);
	virtual						~EscapeFilter (void);
	virtual filter_result	Filter (BMessage *, BHandler **);
};


#endif
