// Socket-based class used by client-server app setup.
//
// see License at the end of file

#ifndef IO_CHANNEL__
#define IO_CHANNEL__

#include "PlatformDefines.h"

#include "CString.h"
#include "ObjectList.h"

class IOChannelReader {
public:
	IOChannelReader(uint32 signature);

	virtual void StartReading(int32 size);
	virtual bool HandleReadData(const char *, int32);
		// returns true when done

	bool Matches(uint32 signature) const
		{ return signature == fSignature; }

private:
	uint32 fSignature;
	int32 fExpectedSize;
	int32 fReadSize;
};

struct IOChannelDataHeader {
	uint32 fSignature;
	int32 fSize;
};

class FIOServerChannel {
public:
	FIOServerChannel(const char *signature);
	virtual ~FIOServerChannel();
	static void MakeSocketPath(const char *signature, String &result);
	static bool Exists(const char *path);

	void AddReader(IOChannelReader *reader)
		{ fReaders.AddItem(reader); }
	//void Respond(const char *replyData, int32 replySize);

private:
	bool ChannelCallback(GIOChannel *, GIOCondition);

	void OpenListener(const char *path);
	static gboolean ChannelCallbackBinder(GIOChannel *, GIOCondition, gpointer);

	String fChannelPath;
	int fFd;
	uint32 fIOWatchID;

	ObjectList<IOChannelReader> fReaders;
};

class FIOClientChannel {
public:
	FIOClientChannel(const char *signature);
	virtual ~FIOClientChannel();

	void Send(uint32 signature, const char *data, int32 size);
	static bool ServerExists(const char *signature);

private:
	void OpenWriter(const char *path);
	void SendData(const void *, int32 size);

	int fFd;
	
};

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
