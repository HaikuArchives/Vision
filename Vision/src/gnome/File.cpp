// (c) 2000 Pavel Cisler

#include "PlatformDefines.h"

#include "Directory.h"
#include "File.h"
#include "Path.h"

#include <errno.h>
#include <unistd.h>

FFile::FFile()
{
}

FFile::FFile(const FFile &file)
	:	FNode(), FPositionIO()
{
	this->operator=(file);
}

FFile::FFile(const EntryRef *ref, uint32 mode)
{
	SetTo(ref, mode);
}

FFile::FFile(const FEntry *entry, uint32 mode)
{
	SetTo(entry, mode);
}

FFile::FFile(const char *path, uint32 mode)
{
	SetTo(path, mode);
}


FFile::FFile(const FDirectory *directory, const char *name, uint32 mode)
{
	SetTo(directory, name, mode);
}

FFile::~FFile()
{
	SetFD(-1);
}


status_t 
FFile::SetTo(const char *path, uint32 mode)
{
	int fd = open(path, mode, 0666);
	SetFD(fd);

	if (fd >= 0) {
		// set up the attribute support
		struct stat statBuffer;
		if (lstat(path, &statBuffer) == 0) {
			fInode = statBuffer.st_ino;
			fOriginalFileName = path;
		}	
	}

	return fStatus;
}

status_t 
FFile::SetTo(const EntryRef *ref, uint32 mode)
{
	return SetTo(ref->Path(), mode);
}

status_t 
FFile::SetTo(const FEntry *entry, uint32 mode)
{
	EntryRef ref;
	entry->GetRef(&ref);
	return SetTo(&ref, mode);
}

status_t 
FFile::SetTo(const FDirectory *directory, const char *name, uint32 mode)
{
	FPath path;
	fStatus = directory->GetPath(&path);
	if (fStatus != B_OK)
		return fStatus;
	path.Append(name);
	fStatus = SetTo(path.Path(), mode);
	
	return fStatus;
}

FFile &
FFile::operator=(const FFile &file)
{
	if (&file == this)
		return *this;

	fStatus = file.fStatus;
	fFd = file.fFd;

	if (fFd >= 0) {
		fInode = file.fInode;
		fOriginalFileName = file.fOriginalFileName;
	}

	return *this;
}


ssize_t 
FFile::Read(void *buffer, size_t size)
{
	return read(fFd, buffer, size);
}

ssize_t 
FFile::Write(const void *buffer, size_t size)
{
	return write(fFd, buffer, size);
}

off_t 
FFile::Seek(off_t offset, uint32 whence)
{
	return lseek(fFd, offset, whence);
}

status_t 
FFile::SetSize(off_t size)
{
	return ftruncate(fFd, size);
}

ssize_t 
FFile::ReadAt(off_t pos, void *buffer, size_t size)
{
	if (Seek(pos, SEEK_SET) < 0)
		return (ssize_t)errno;
	return Read(buffer, size);
}

ssize_t 
FFile::WriteAt(off_t pos, const void *buffer, size_t size)
{
	if (Seek(pos, SEEK_SET) < 0)
		return (ssize_t)errno;
	return Write(buffer, size);
}

off_t 
FFile::Position() const
{
	return lseek(fFd, 0, SEEK_CUR);
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
