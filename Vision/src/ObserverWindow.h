
#ifndef OBSERVERWINDOW_H_
#define OBSERVERWINDOW_H_

#include <Window.h>

class Settings;

class ObserverWindow : public BWindow
{
	public:

									ObserverWindow (
										BRect,
										const char *,
										window_look = B_DOCUMENT_WINDOW_LOOK,
										window_feel = B_NORMAL_WINDOW_FEEL);
									ObserverWindow (
										BRect,
										const char *,
										bool,
										window_look = B_DOCUMENT_WINDOW_LOOK,
										window_feel = B_NORMAL_WINDOW_FEEL);

	virtual						~ObserverWindow (void);

	virtual bool				QuitRequested (void);
	virtual void				DispatchMessage (BMessage *, BHandler *);
	virtual void				NoticeReceived (BMessage *, uint32);
	virtual void				StateChange (BMessage *);
	virtual void				SetWindowFollows (bool);
};

#endif
