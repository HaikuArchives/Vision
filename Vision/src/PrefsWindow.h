#ifndef _PREFSWINDOW_H_
#define _PREFSWINDOW_H_

#include <Window.h>

class PrefsWindow : public BWindow
{
  public:
    PrefsWindow(void);
    virtual ~PrefsWindow(void);
    virtual bool QuitRequested(void);
    virtual void MessageReceived(BMessage *);
};
#endif // _PREFS_WINDOW_H
