
#include <Window.h>

#include "PlayButton.h"

PauseButton::PauseButton (BPoint left_top, BMessage *msg)
	: BButton (
		BRect (left_top, left_top + BPoint (21, 17)),
		"pause", "", msg),

	  isPaused (false),
	  isActive (false)
{
}

PauseButton::~PauseButton (void)
{
}

void
PauseButton::AttachedToWindow (void)
{
	BButton::AttachedToWindow();
	ResizeTo (22, 18);
}

void
PauseButton::Draw (BRect frame)
{
	rgb_color black = {0, 0, 0, 255};
	rgb_color white = {255, 255, 255, 255};

	BButton::Draw (frame);

	frame = Bounds();
	frame.InsetBy (9, 6);
	SetHighColor (Value() ? white : black);

	if (isPaused)
	{
		FillTriangle (
			frame.LeftTop(),
			frame.LeftBottom(),
			frame.RightTop() + BPoint (0, 3));
	}
	else
	{
		BRect rect (frame);
		rect.right -= 3;

		frame.left += 3;
		frame.OffsetBy (1, 0);

		FillRect (frame);
		FillRect (rect);
	}
}

void
PauseButton::MouseDown (BPoint point)
{
	isActive =
		 Window()->IsActive()
	|| (Window()->Flags() & B_WILL_ACCEPT_FIRST_CLICK) != 0;
	BButton::MouseDown (point);
}

void
PauseButton::MouseUp (BPoint point)
{
	if (isActive && Bounds().Contains (point))
		isPaused = !isPaused;

	BButton::MouseUp (point);
}

bool
PauseButton::IsPaused (void) const
{
	return isPaused;
}

StopButton::StopButton (BPoint left_top, BMessage *msg)
	: BButton (
		BRect (left_top, left_top + BPoint (21, 17)),
		"stop", "", msg)
{
}

StopButton::~StopButton (void)
{
}

void
StopButton::AttachedToWindow (void)
{
	BButton::AttachedToWindow();
	ResizeTo (22, 18);
}

void
StopButton::Draw (BRect frame)
{
	rgb_color black = {0, 0, 0, 255};
	rgb_color white = {255, 255, 255, 255};

	BButton::Draw (frame);

	frame = Bounds();
	frame.InsetBy (9, 7);
	SetHighColor (Value() ? white : black);
	FillRect (frame);
}

