// BApplication-like class and supporting code
//
// see License at the end of file

#ifndef FAPPLICATION__
#define FAPPLICATION__

#include "PlatformDefines.h"

#include "CString.h"
#include "Looper.h"
#include "Entry.h"
#include "IOChannel.h"
#include "ObjectList.h"

struct AppInfo {
	EntryRef ref;
};

class FApplication;
class FWindow;

class ArgvIOChannelHandler : public IOChannelReader {
public:
	ArgvIOChannelHandler(FApplication *application)
		:	IOChannelReader('argv'),
			fApplication(application)
		{}

	virtual bool HandleReadData(const char *, int32);

	static void SendArgv(FIOClientChannel *, int32 argc, const char *const *argv);
private:
	FApplication *fApplication;
	
};

class ArgvIOServerChannel : public FIOServerChannel {
public:
	ArgvIOServerChannel(FApplication *, const char *signature);
};

bool StartAsClient(const char *signature, int argc, const char *const *argv);


class FApplication : public FLooper {
public:
	FApplication(const char *sig, int argc, const char *const * argv);
	virtual ~FApplication();

	virtual void ArgvReceived(int32, char **);

	void Run();
	void Quit();

	void GetAppInfo(AppInfo *) const;
	
	int32 CountWindows() const;
	FWindow *WindowAt(int32 index) const;

	// used by FWindow only
	void WindowAdded(FWindow *);
	void WindowRemoved(FWindow *);

protected:
	virtual void MessageReceived(BMessage *);
	virtual bool QuitRequested();
	virtual void RefsReceived(BMessage *);

	virtual void DispatchMessage(FMessage *, FHandler *);
	
private:
	String fAppSignature;
	EntryRef fAppPath;
	ArgvIOServerChannel *fServerChannel;
	ObjectList<FWindow> fWindows;
};

extern FApplication *be_app;

#endif

/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler

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
