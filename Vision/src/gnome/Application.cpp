// BApplication-like class and supporting code
//
// see License at the end of file

#include "PlatformDefines.h"

#include "Application.h"
#include "Message.h"
#include "IOChannel.h"
#include "Font.h"
#include "Window.h"

#include <Debug.h>

#include <stdio.h>
// #include <libgnomeui/gnome-init.h>
// -- gnome-init.h have broken extern "C" { linkage specifiers
//    and won't compile with C++
#include <gtk/gtkmain.h>
#include <malloc.h>

bool 
ArgvIOChannelHandler::HandleReadData(const char *data, int32 size)
{
	char **argv = NULL;
	int32 argc = 0;

	const char *nextArg = data;
	for (;;) {
		if (size <= 0)
			break;
		
		argc++;
		argv = (char **)realloc(argv, (argc + 1) * sizeof(char *));
		argv[argc - 1] = strdup(nextArg);

		int32 nextArgSize = strlen(nextArg);
		size -= nextArgSize + 1;
		nextArg += nextArgSize + 1;
	}

	if (argc) {
		argv[argc] = NULL;
		fApplication->ArgvReceived(argc, argv);
		for (int32 index = 0; index < argc; index++)
			free(argv[index]);
		free(argv);
	}
	return IOChannelReader::HandleReadData(data, size);
}

void 
ArgvIOChannelHandler::SendArgv(FIOClientChannel *channel, int32 argc, 
	const char *const *argv)
{
	int32 size = 0;
	for (int32 index = 0; index < argc; index++)
		size += strlen(argv[index]) + 1;

	char *data = (char *)malloc(size);
	char *next = data;
	for (int32 index = 0; index < argc; index++) {
		strcpy(next, argv[index]);
		next += strlen(argv[index]) + 1;
	}

	channel->Send('argv', data, size);
	free(data);
}

ArgvIOServerChannel::ArgvIOServerChannel(FApplication *app, const char *signature)
	:	FIOServerChannel(signature)
{
	AddReader(new ArgvIOChannelHandler(app));
}

bool
StartAsClient(const char *signature, int argc, const char *const *argv)
{
	try {
		if (FIOClientChannel::ServerExists(signature)) {
			// app already running, send it the arg parameters.
			FIOClientChannel clientChannel(signature);
			ArgvIOChannelHandler::SendArgv(&clientChannel, argc, argv);
			return true;
		}
	} catch (...) {
	}
	
	return false;
}

FApplication *be_app = NULL;

// having trouble including gnome-init.h
// -- gnome-init.h have broken extern "C" { linkage specifiers
//    and won't compile with C++
extern "C" void gnome_init(const char *, const char *, int, char **);

FApplication::FApplication(const char *signature, int argc, 
	const char *const *argv)
	:	fAppSignature(signature),
		fAppPath(*argv),
		fServerChannel(NULL)
{
	try {
		fServerChannel = new ArgvIOServerChannel(this, signature);
	} catch (status_t error) {
		printf("Failed to set up a server channel (%s)\n", strerror(error));
		/* continue anyhow */
	}
	
	gnome_init("Eddie", "1.0", argc, (char **)argv);
	gtk_init(&argc, (char ***)&argv);
	FFont::InitFonts();
	ASSERT(be_app == NULL);
	be_app = this;
}

FApplication::~FApplication()
{
	delete fServerChannel;
	gtk_exit(0);
}

void 
FApplication::ArgvReceived(int32, char **)
{
}

void
FApplication::Quit()
{
	if (!QuitRequested())
		return;

	gtk_main_quit();
}

void 
FApplication::Run()
{
	gtk_main();
}

void 
FApplication::GetAppInfo(AppInfo *info) const
{
	FEntry entry("/proc/self/exe");
	if (entry.Exists())
		entry.GetRef(&info->ref);
	else {
		PRINT(("failed to get app path from /proc/self/exe\n"));
		info->ref = fAppPath;
	}
}

void
FApplication::MessageReceived(FMessage *message)
{
	switch (message->what) {
		case B_QUIT_REQUESTED:
			Quit();
			break;

		default:
			FLooper::MessageReceived(message);
			break;
	}
}

bool 
FApplication::QuitRequested()
{
	// ask if everyone is OK to quit
	bool okToQuit = true;
	int32 count = fWindows.CountItems();
	for (int32 index = 0; index < count; index++) {
		FWindow *window = fWindows.ItemAt(index);
		ASSERT(FLooper::IsValid(window));
		if (FLooper::IsValid(window) && !window->QuitRequested()) 
			okToQuit = false;
	}
	
	if (okToQuit)
		// everyone is OK to quit, quit every window
		// go in reverse so that we don't trip over deleted
		// items
		for (int32 index = count - 1; index >= 0; index--) {
			FWindow *window = fWindows.ItemAt(index);
			ASSERT(FLooper::IsValid(window));
			if (FLooper::IsValid(window))
				window->Quit();
		}
	
	return okToQuit;
}

void 
FApplication::DispatchMessage(FMessage *message, FHandler *handler)
{
	if (message->what == B_REFS_RECEIVED)
		RefsReceived(message);
	else
		FLooper::DispatchMessage(message, handler);
}


void 
FApplication::WindowAdded(FWindow *window)
{
	ASSERT(FLooper::IsValid(window));
	fWindows.AddItem(window);
}

void 
FApplication::WindowRemoved(FWindow *window)
{
	fWindows.RemoveItem(window);
}

void 
FApplication::RefsReceived(BMessage *)
{
}

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
