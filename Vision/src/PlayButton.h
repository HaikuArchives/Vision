
#ifndef PLAYBUTTON_H_
#define PLAYBUTTON_H_

#include <Button.h>

class PauseButton : public BButton
{
	bool					isPaused;
	bool					isActive;

	public:

							PauseButton (BPoint, BMessage *);
	virtual				~PauseButton (void);

	virtual void		Draw (BRect);
	virtual void		AttachedToWindow (void);
	virtual void		MouseDown (BPoint);
	virtual void		MouseUp (BPoint);
	bool					IsPaused (void) const;
};

class StopButton : public BButton
{
	public:

							StopButton (BPoint, BMessage *);
	virtual				~StopButton (void);

	virtual void		Draw (BRect);
	virtual void		AttachedToWindow (void);
};

#endif
