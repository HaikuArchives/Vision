
#ifndef DCCFILEWINDOW_H_
#define DCCFILEWINDOW_H_

#include <Window.h>

class DCCConnect;
class WindowSettings;

class DCCFileWindow : public BWindow
{
	public:

								DCCFileWindow (DCCConnect *);
	virtual						~DCCFileWindow (void);

	virtual bool				QuitRequested (void);
	virtual void				MessageReceived (BMessage *);
	virtual void				Hide (void);
	virtual void				Show (void);
};

#endif
