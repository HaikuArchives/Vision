// Socket-based class used by client-server app setup.
//
// see License at the end of file

#include "PlatformDefines.h"
#include "IOChannel.h"
#include "FileErrors.h"

#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>
#include <algorithm>

IOChannelReader::IOChannelReader(uint32 signature)
	:	fSignature(signature),
		fExpectedSize(0),
		fReadSize(-1)
	{}

void 
IOChannelReader::StartReading(int32 size)
{
	fExpectedSize = size;
	fReadSize = 0;
}

bool 
IOChannelReader::HandleReadData(const char *, int32 size)
{
	ASSERT(fReadSize >= 0);
	fReadSize -= size;
	return fReadSize <= 0;
}

const status_t kSocketNotFound = -2;

void 
FIOServerChannel::MakeSocketPath(const char *signature, String &result)
{
	result << g_get_tmp_dir() << "/.";
	for (;;) {
		if (!*signature)
			break;

		// don't allow any slashes
		if (*signature == '/')
			result += '-';
		else
			result += *signature;

		signature++;
	}
}

bool 
FIOServerChannel::Exists(const char *path)
{
	struct stat statBuffer;
	return stat(path, &statBuffer) == 0 && S_ISSOCK(statBuffer.st_mode);
}

FIOServerChannel::FIOServerChannel(const char *signature)
	:	fFd(-1),
		fIOWatchID(0),
		fReaders(3, true)
{
	MakeSocketPath(signature, fChannelPath);
	OpenListener(fChannelPath.CStr());
}

FIOServerChannel::~FIOServerChannel()
{
	if (fIOWatchID != 0)
		g_source_remove(fIOWatchID);

	close(fFd);
	unlink(fChannelPath.CStr());
}

void 
FIOServerChannel::OpenListener(const char *path)
{

	try {
		unlink(path);
		fFd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fFd < 0)
			throw (status_t)fFd;

		// bind our new socket to a physical socket in the file system
		struct sockaddr_un socketAddress;
		memset(&socketAddress, 0, sizeof(socketAddress));
		socketAddress.sun_family = AF_UNIX;
		strncpy(socketAddress.sun_path, path, sizeof(socketAddress.sun_path));

		int32 size = strlen(socketAddress.sun_path) + sizeof(socketAddress.sun_family);
		ThrowErrno( bind(fFd, (struct sockaddr *)&socketAddress, size) );

		// listen for input
		ThrowErrno( listen(fFd, 5) );
		// serve up to 5 connections

		// for now just call the reading call directly
		// this would not work normally, we'd need a thread or
		// the GIOChannel callbacks
		GIOChannel *channel = g_io_channel_unix_new (fFd);
		fIOWatchID = g_io_add_watch(channel, 
			(GIOCondition)(G_IO_IN | G_IO_HUP), 
			FIOServerChannel::ChannelCallbackBinder, this);
		g_io_channel_unref (channel);

	} catch (...) {
		if (fFd >= 0)
			close(fFd);
		if (fIOWatchID != 0)
			g_source_remove(fIOWatchID);
		unlink(path);
		throw;
	}
}

gboolean 
FIOServerChannel::ChannelCallbackBinder(GIOChannel *channel, GIOCondition condition, 
	gpointer castToThis)
{
	return ((FIOServerChannel *)castToThis)->ChannelCallback(channel, condition);
}

class MatchBySignature : public UnaryPredicate<IOChannelReader> {
public:
	MatchBySignature(uint32 signature)
		:	fSignature(signature)
		{}
	virtual int operator()(const IOChannelReader *reader) const
		{ return reader->Matches(fSignature) ? 0 : -1; }

	uint32 fSignature;
};

bool 
FIOServerChannel::ChannelCallback(GIOChannel *, GIOCondition)
{
	int listenFD = accept (fFd, NULL, 0);
	try {
		size_t size;
		IOChannelDataHeader header;

		size = ThrowIfNotSize( read(listenFD, (char *)&header, sizeof(header)) );

		if (header.fSignature == 'strt') {
			// startup ack, open call must follow
			// ToDo: clean this up
			size = ThrowIfNotSize( read(listenFD, (char *)&header, sizeof(header)) );
		}
		IOChannelReader *reader = fReaders.Search(MatchBySignature(header.fSignature));

		// how much data?
		reader->StartReading(header.fSize);

		char tmpBuffer[1024];
		ssize_t remaining = header.fSize;

		for (;;) {
			size = ThrowIfNotSize( read(listenFD, (char *)&tmpBuffer, max(1024, remaining)));
			remaining -= size;

			// send the read data off to the handler
			if (reader->HandleReadData(tmpBuffer, size)) {
				// done reading
				break;
			}
		}

	} catch (status_t error) {
		// could send back the error here.
	}

	return true;
}


FIOClientChannel::FIOClientChannel(const char *signature)
	:	fFd(-1)
{
	String channelPath;

	FIOServerChannel::MakeSocketPath(signature, channelPath);
	if (!FIOServerChannel::Exists(channelPath.CStr()))
		throw kSocketNotFound;

	OpenWriter(channelPath.CStr());
}

FIOClientChannel::~FIOClientChannel()
{
	if (fFd >= 0)
		close(fFd);
}

void 
FIOClientChannel::SendData(const void *data, int32 size)
{
	if (fFd < 0)
		Throw((status_t)fFd);

	try {
		const char *buffer = (const char *)data;
		while (size > 0) {
			int32 bytesSent = write(fFd, buffer, size);
			// Use send() to avoid EPIPE errors
			//int32 bytesSent = send(fFd, buffer, size, MSG_NOSIGNAL);
			if (bytesSent < 0) {
				if (bytesSent != EINTR)
					Throw((status_t)bytesSent);
			} else if (bytesSent == 0)
				throw (status_t)B_ERROR;
			else {
				size -= bytesSent;
				buffer += bytesSent;
			}
		}
	} catch (...) {
		close(fFd);
		fFd = -1;
		throw;
	}
}

void 
FIOClientChannel::Send(uint32 signature, const char *data, int32 size)
{
	IOChannelDataHeader header;
	header.fSignature = signature;
	header.fSize = size;
	SendData(&header, sizeof(header));
	SendData(data, size);
}

bool 
FIOClientChannel::ServerExists(const char *signature)
{
	String channelPath;
	FIOServerChannel::MakeSocketPath(signature, channelPath);

	return FIOServerChannel::Exists(channelPath.CStr());
}

void 
FIOClientChannel::OpenWriter(const char *path)
{
	fFd = ThrowErrno( socket( AF_UNIX, SOCK_STREAM, 0) );

	struct sockaddr_un socketAddress;
	memset(&socketAddress, 0, sizeof(socketAddress));
	socketAddress.sun_family = AF_UNIX;
	strncpy(socketAddress.sun_path, path, sizeof(socketAddress.sun_path));

	int32 size = strlen(socketAddress.sun_path) + sizeof(socketAddress.sun_family);
	ThrowErrno( connect(fFd, (struct sockaddr *)&socketAddress, size) );

	ThrowErrno( fcntl(fFd, F_SETFD, FD_CLOEXEC) );

	// try writing a startup header
	IOChannelDataHeader startupHeader;
	startupHeader.fSignature = 'strt';
	startupHeader.fSize = 0;
	ThrowErrno( write(fFd, &startupHeader, sizeof(startupHeader)) );
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
