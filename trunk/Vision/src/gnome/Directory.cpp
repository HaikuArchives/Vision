// BDirectory-like class
//
// see License at the end of file

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <malloc.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Directory.h"
#include "File.h"
#include "Path.h"
#include "SymLink.h"

FDirectory::FDirectory()
	:	fStatus(B_NO_INIT),
		fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
}

FDirectory::FDirectory(const char *path)
	:	fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
	struct stat statBuffer;
	
	fStatus = lstat(path, &statBuffer);
	if (fStatus == B_OK && !S_ISDIR(statBuffer.st_mode))
		fStatus = ENOTDIR;

	if (fStatus == B_OK)
		fRef.SetTo(path);
}
	
FDirectory::FDirectory(const FDirectory &directory)
	:	fRef(directory.fRef),
		fStatus(directory.fStatus),
		fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
}

FDirectory::FDirectory(const FEntry *entry)
	:	fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
	SetTo(entry);
}

FDirectory::FDirectory(const EntryRef *ref)
	:	fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
	SetTo(ref);
}

FDirectory::FDirectory(const FDirectory *directory, const char *path)
	:	fDirectoryIterator(NULL),
		fDirentBuffer(NULL)
{
	SetTo(directory, path);
}

FDirectory::~FDirectory()
{
	Close();
	free(fDirentBuffer);
}

void
FDirectory::Close()
{
	if (fDirectoryIterator)
		closedir(fDirectoryIterator);
	fDirectoryIterator = NULL;
}

status_t 
FDirectory::GetRef(EntryRef *ref) const
{
	*ref = fRef;
	return B_OK;
}

status_t 
FDirectory::GetPath(FPath *path) const
{
	path->SetTo(fRef.Path());
	return B_OK;
}

status_t 
FDirectory::GetEntry(FEntry *entry) const
{
	entry->SetTo(&fRef);
	return B_OK;
}

bool 
FDirectory::operator==(const FDirectory &directory) const
{
	return directory.fRef == fRef;
}

FDirectory &
FDirectory::operator=(const FDirectory &directory)
{
	if (&directory == this)
		return *this;

	Close();
	fRef = directory.fRef;
	fStatus = directory.fStatus;

	return *this;
}

status_t 
FDirectory::SetTo(const char *path)
{
	Close();

	struct stat statBuffer;
	fStatus = lstat(path, &statBuffer);
	
	if (fStatus == B_OK && !S_ISDIR(statBuffer.st_mode))
		fStatus = ENOTDIR;

	if (fStatus == B_OK)
		fRef.SetTo(path);

	return fStatus;
}

status_t 
FDirectory::SetTo(const FEntry *entry)
{
	Close();

	FPath path;
	fStatus = entry->GetPath(&path);
	if (fStatus != B_OK)
		return fStatus;

	return SetTo(path.Path());
}

status_t 
FDirectory::SetTo(const EntryRef *ref)
{
	return SetTo(ref->Path());
}

status_t 
FDirectory::SetTo(const FDirectory *directory, const char *path)
{
	FPath newPath;
	fStatus = directory->GetPath(&newPath);
	if (fStatus != B_OK)
		return fStatus;

	fStatus = newPath.Append(path);
	if (fStatus != B_OK)
		return fStatus;

	return SetTo(newPath.Path());
}

bool 
FDirectory::Contains(const char *name) const
{
	FEntry entry(this, name);
	return entry.Exists();
}

status_t 
FDirectory::InitCheck() const
{
	return fStatus;
}

status_t 
FDirectory::CreateFile(const char *name, FFile *result, bool failIfExists)
{
	uint32 mode = O_RDWR | O_CREAT | O_TRUNC;
	if (failIfExists)
		mode |= O_EXCL;


	FPath path(fRef.Path());
	path.Append(name);

	return result->SetTo(path.Path(), mode);
}

status_t 
FDirectory::CreateDirectory(const char *path)
{
	for (int32 index = 0;;) {
		if (!path[index])
			break;

		const char *scanner = strchr(&path[index + 1], '/');
		if (!scanner) 
			// we are done
			return mkdir(path, 0777);

		index = scanner - path;
		String partialPath;
		partialPath.Assign(path, index);

		// ignore errors as we go
		mkdir(partialPath.CStr(), 0777);
	}
	return B_OK;
}

status_t 
FDirectory::CreateSymLink(const char *name, const char *linkTo, FSymLink *result)
{
	status_t error = result->SetTo(this, name);
	if (error != B_OK)
		return error;
	
	FPath path(this, name);
	return symlink(path.Path(), linkTo);
}


status_t 
FDirectory::FDirectory::Rewind()
{
	if (fDirectoryIterator)
		rewinddir(fDirectoryIterator);
		
	return B_OK;
}

status_t 
FDirectory::GetNextRef(EntryRef *ref)
{
	if (!fDirectoryIterator)
		// lazily open the directory
		fDirectoryIterator = opendir(fRef.Path());

	if (!fDirectoryIterator) {
		fStatus = errno;
		return fStatus;
	}

	if (!fDirentBuffer)
		// lazily allocate the dirent buffer
		fDirentBuffer = (struct dirent *)malloc (sizeof(dirent) + _POSIX_PATH_MAX + 1);

	FPath path(&fRef);

	dirent *resultDirent;
	for (;;) {
		// read the next entry
		fStatus = readdir_r(fDirectoryIterator, fDirentBuffer, &resultDirent);

		if (fStatus == B_OK && !resultDirent)
			fStatus = ENOENT;

		if (fStatus != B_OK)
			return fStatus;

		if (strcmp(resultDirent->d_name, ".") != 0
			&& strcmp(resultDirent->d_name, "..") != 0)
			// these aren't useful, get me the next
			break;
	}
	
	// append to the parent dir
	path.Append(resultDirent->d_name);
	ref->SetTo(path.Path());

	return B_OK;
}

status_t 
FDirectory::GetNextEntry(FEntry *entry, bool reslove)
{
	EntryRef ref;
	status_t result = GetNextRef(&ref);
	if (result != B_OK)
		return result;

	entry->SetTo(&ref, reslove);
	return B_OK;
}

status_t 
FDirectory::FindEntry(const char *name, BEntry *result, bool DEBUG_ONLY(traverse)) const
{
	ASSERT(!traverse || !"not yet implemented");

	BEntry temp(this, name);
	if (!temp.Exists())
		return ENOENT;

	*result = temp;
	return B_OK;	
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
