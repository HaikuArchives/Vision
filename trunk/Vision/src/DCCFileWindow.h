
#ifndef DCCFILEWINDOW_H_
#define DCCFILEWINDOW_H_

#include "ObserverWindow.h"

class DCCConnect;
class WindowSettings;

class DCCFileWindow : public ObserverWindow
{
	WindowSettings				*settings;

	public:

									DCCFileWindow (DCCConnect *, bool);
	virtual						~DCCFileWindow (void);

	virtual bool				QuitRequested (void);
	virtual void				MessageReceived (BMessage *);
	virtual void				Hide (void);
	virtual void				Show (void);
};

#endif
