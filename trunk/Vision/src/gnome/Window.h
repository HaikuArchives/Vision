// BWindow-like class
//
// see License at the end of file

#ifndef FWINDOW__
#define FWINDOW__

#include "PlatformDefines.h"

#include <gtk/gtkwindow.h>

#include "Looper.h"
#include "Rect.h"
#include "ObjectGlue.h"

typedef int32 window_type;
typedef int32 window_look;
typedef int32 window_feel;

const int32 B_DOCUMENT_WINDOW = 1;
const int32 B_TITLED_WINDOW = 2;
const int32 B_MODAL_WINDOW = 3;

const int32 B_TITLED_WINDOW_LOOK = 1;
const int32 B_DOCUMENT_WINDOW_LOOK = 2;
const int32 B_BORDERED_WINDOW_LOOK = 3;
const int32 B_MODAL_WINDOW_LOOK = 4;

const int32 B_NORMAL_WINDOW_FEEL = 1;
const int32 B_MODAL_APP_WINDOW_FEEL = 2;
const int32 B_FLOATING_ALL_WINDOW_FEEL = 3;

const int32 B_NOT_RESIZABLE = 1;
const int32 B_NOT_ZOOMABLE = 2;

class FMenuBar;

class FWindow : public ObjectGlue<GtkWindow>, public FLooper {
public:
	FWindow(FRect frame, const char *title, window_type type, uint32 flags);
	FWindow(FRect frame, const char *title, window_look look, window_feel feel,
		uint32 flags);
	~FWindow();

	virtual void Quit();
	virtual void Show();
	virtual void Hide();
	virtual void Activate(bool);
	virtual void WindowActivated(bool);	

	virtual void Close();
	
	BRect Frame() const;
	BRect Bounds() const;

	bool IsActive() const;
	bool IsHidden() const;

	// for now, will change when FMenuBar inherits from FView
	void AddChild(FView *);
	void AddChild(FMenuBar *);
	
	FView *FindView(const char *) const;
	FView *ChildAt(int32) const;

	FMenuBar *KeyMenuBar() const;

	void MoveTo(float, float);
	void MoveTo(FPoint);
	void ResizeTo(float, float);
	
	const char *Title() const;	
	void SetTitle(const char *);
	
	GtkWidget *AsGtkWidget()
		{ return GTK_WIDGET(fObject); }
	const GtkWidget *AsGtkWidget() const
		{ return reinterpret_cast<const GtkWidget *>(fObject); }

	void SetFlags(uint32);

	virtual bool QuitRequested();
	virtual void MessageReceived(FMessage *);
	virtual void FrameResized(float, float);
	
	FPoint ConvertToScreen(FPoint) const;
	FPoint ConvertFromScreen(FPoint) const;

	void GetSizeLimits(float *, float *, float *, float *);
	void SetSizeLimits(float , float , float , float);
	void SetZoomLimits(float , float);
	
	void SetPulseRate(bigtime_t);
	
	void UpdateIfNeeded() {}
	
	// only FView should call this
	void TopLeveViewMoveTo(FView *view, FPoint to);
	void RemoveChild(FView *view);
	bigtime_t PulseRate() const;

protected:
	virtual void DispatchMessage(FMessage *, FHandler *);

	// non-BeOS only
	virtual int DeleteEvent(GdkEventAny *);
	virtual int FocusInEvent(GdkEventFocus *);
	virtual int FocusOutEvent(GdkEventFocus *);
	virtual void SizeAllocate(GtkAllocation *);
	
	virtual void MenusBeginning();


private:
	void FWindowCommon(const char *title, window_look look, window_feel feel,
		uint32 flags);

	static FWindow *GnoBeObject(GtkWidget *gtkObject)
		{	return ::GnoBeObject<FWindow, GtkWidget>(gtkObject); }

	// non-BeOS only default method overriding glue
	void InitializeClass();
	static int DeleteEventBinder(GtkWidget *, GdkEventAny *);
	static int FocusInEventBinder(GtkWidget *, GdkEventFocus *);
	static int FocusOutEventBinder(GtkWidget *, GdkEventFocus *);
	static void SizeAllocateBinder(GtkWidget *, GtkAllocation *);

	static void RealizeHook(GtkWidget *, FWindow *);	
	
	static gboolean PulseDispatcher(void *);

	// need this to be able to get frame even before window
	// is properly activated
	BRect fInitialFrame;

	// special background widget used to keep items in the right places without
	// gtk interferring. We could probably get rid of this. 
	GtkWidget *fFixed;
	FView *fTopLevelView;
	FMenuBar *fMenuBar;
	bool fActive;

	window_look fInitialLook;
	window_feel fInitialFeel;

	uint32 fIdleHandler;
	bool fInitialAllocate;
	
	float fMinX;
	float fMinY;
	float fMaxX;
	float fMaxY;
	float fMaxZoomWidth;
	float fMaxZoomHeight;

	bigtime_t fPulseRate;
	
	typedef FLooper inherited_;
};
#endif
/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler
Copyright (c) 1999-2001, Gene Ragan

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met: 

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE,
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE. 
*/
