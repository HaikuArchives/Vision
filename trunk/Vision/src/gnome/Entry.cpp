// entry_ref, Entry
//
// See License at the end fo this file

#include "Entry.h"

#include <Debug.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include "CString.h"
#include "Directory.h"
#include "Path.h"

EntryRef::EntryRef()
	:	name(NULL)
{
}

EntryRef::EntryRef(const EntryRef &ref)
	:	fPath(ref.fPath)
{
	SetUpName();
}

EntryRef::EntryRef(const char *path)
	:	fPath(path)
{
	// add path normailzing here
	SetUpName();
}

EntryRef::~EntryRef()
{
}

status_t 
EntryRef::SetTo(const char *name)
{
	fPath = name;
	SetUpName();
	return B_OK;
}

status_t 
EntryRef::SetName(const char *newName)
{
	int32 nameOffset = fPath.FindLast('/');
	if (nameOffset == -1) {
		TRESPASS();
		return -1;
	}
	fPath.Truncate(nameOffset + 1);
	fPath += newName;
	SetUpName();
	return B_OK;
}

bool 
EntryRef::operator==(const EntryRef &ref) const
{
	return fPath == ref.fPath;
}

EntryRef &
EntryRef::operator=(const EntryRef &ref)
{
	if (&ref == this)
		// self-assignment
		return *this;
	
	fPath = ref.fPath;

	SetUpName();
	return *this;
}

void 
EntryRef::SetUpName()
{
	if (!fPath.Length()) {
		name = NULL;
		return;
	}
	if (fPath[0] != '/') {
		// prepend directory if 
		char *parent = g_get_current_dir();
		FPath path(parent);
		g_free(parent);
		path.Append(fPath.CStr());
		fPath = path.Path();
	}

	FPath::Normalize(fPath.CStr(), fPath);

	name = (const char *)strrchr((char *)fPath.CStr(), '/');
	if (name)
		name++;
	else
		// no '/', just point to the start of the path
		name = fPath.CStr();
}

const char *
EntryRef::Path() const
{
	return fPath.CStr();
}

bool 
EntryRef::operator!=(const EntryRef &ref) const
{
	return !operator==(ref);
}

FEntry::FEntry()
	:	fStatus(B_NO_INIT)
{
}

FEntry::FEntry(const char *path, bool resolve)
{
	SetTo(path, resolve);
}

FEntry::FEntry(const EntryRef *ref, bool resolve)
{
	SetTo(ref, resolve);
}

FEntry::FEntry(const FDirectory *directory, const char *name)
{
	SetTo(directory, name);
}

status_t 
FEntry::SetTo(const FDirectory *directory, const char *name)
{
	EntryRef ref;
	fStatus = directory->GetRef(&ref);	
	if (fStatus == B_OK) {
		FPath path(ref.Path());
		if (name) {
			path.Append(name);
			ref.SetTo(path.Path());

		}
		if (fStatus == B_OK)
			fRef = ref;
	}
	return fStatus;
}

status_t 
FEntry::SetTo(const char *path, bool resolve)
{
	char linkTarget[2048];
	if (resolve) {
		// resolve if needed
		struct stat statStruct;
		if (lstat(path, &statStruct) == 0 
			&& S_ISLNK(statStruct.st_mode)) {
			int32 length = readlink(path, linkTarget, 2048);
			if (length > 0) {
				linkTarget[length] = '\0';
				path = &linkTarget[0];
			}
		}
	}
	fRef.SetTo(path);
	fStatus = B_OK;
	return B_OK;
}

status_t 
FEntry::SetTo(const EntryRef *ref, bool resolve)
{
	return SetTo(ref->Path(), resolve);
}

status_t 
FEntry::GetRef(EntryRef *ref) const
{
	*ref = fRef;
	return B_OK;
}

status_t 
FEntry::GetPath(FPath *path) const
{
	path->SetTo(fRef.Path());
	return B_OK;
}

status_t 
FEntry::GetName(char *result) const
{
	strcpy(result, fRef.name);
	return B_OK;
}

status_t 
FEntry::GetParent(FEntry *entry) const
{
	String parentPath(fRef.Path());
	int32 nameOffset = parentPath.FindLast('/');
	if (nameOffset == -1) 
		return ENOENT;

	parentPath.Truncate(nameOffset);
	entry->SetTo(parentPath.CStr());
	return B_OK;
}

status_t 
FEntry::GetParent(FDirectory *directory) const
{
	FEntry entry;
	status_t result = GetParent(&entry);
	if (result != B_OK)
		return result;

	directory->SetTo(&entry);
	return B_OK;
}

status_t 
FEntry::InitCheck() const
{
	return fStatus;
}

bool 
FEntry::Exists() const
{
	struct stat tmp;
	if (fStatus != B_OK)
		return false;
	return lstat(fRef.fPath.CStr(), &tmp) == 0;
}

bool 
FEntry::IsDirectory() const
{
	struct stat statStruct;
	if (lstat(fRef.fPath.CStr(), &statStruct) != 0)
		return false;

	return S_ISDIR(statStruct.st_mode);
}

bool 
FEntry::IsFile() const
{
	struct stat statStruct;
	if (lstat(fRef.fPath.CStr(), &statStruct) != 0)
		return false;

	return S_ISREG(statStruct.st_mode);
}

bool 
FEntry::IsSymLink() const
{
	struct stat statStruct;
	if (lstat(fRef.fPath.CStr(), &statStruct) != 0)
		return false;

	return S_ISLNK(statStruct.st_mode);
}

status_t 
FEntry::GetStat(struct stat *statBuffer)
{
	return lstat(fRef.Path(), statBuffer);
}

status_t 
FEntry::Remove()
{
	if (IsDirectory()) 
		return rmdir(fRef.Path());

	return unlink(fRef.Path());
}


#if 0

// This works close to the BeOS entry_ref but is useless because inodes cannot be
// opened on Linux.

EntryRef::EntryRef()
	:	device((dev_t)-1),
		directory((ino_t)-1),
		name(NULL)
{
}


EntryRef::EntryRef(const EntryRef &ref)
	:	device(ref.name ? ref.device : (dev_t)-1),
		directory(ref.name ? ref.directory : (ino_t)-1),
		name(g_strdup(ref.name))
{
}


EntryRef::EntryRef(dev_t dev, ino_t dir, const char *name)
	:	device(name ? dev : (dev_t)-1),
		directory(name ? dir : (ino_t)-1),
		name(g_strdup(name))
{
}


EntryRef::EntryRef(const char *)
{
}


EntryRef::~EntryRef()
{
	g_free(name);
}

status_t 
EntryRef::SetName(const char *newName)
{
	if (newName == name)
		return B_OK;

	g_free(name);
	name = g_strdup(name);

	return B_OK;
}

bool 
EntryRef::operator==(const EntryRef &ref) const
{
	if (ref.device != device || ref.directory != directory)
		return false;
	if (ref.name == NULL || name == NULL)
		return ref.name == name;
	
	ASSERT(ref.name != NULL && name != NULL);
	return strcmp(ref.name, name) == 0;
}

EntryRef &
EntryRef::operator=(const EntryRef &ref)
{
	if (&ref == this)
		// self-assignment
		return *this;
	
	device = ref.device;
	directory = ref.directory;
	g_free(name);
	name = g_strdup(ref.name);

	return *this;
}

void 
EntryRef::MakeEmpty()
{
	g_free(name);
	name = NULL;
	device = (dev_t)-1;
	directory = (ino_t)-1;
}

status_t 
EntryRef::SetTo(dev_t newDevice, ino_t newDirectory, const char *newName)
{
	if (!newName)
		MakeEmpty();
	else {
		device = newDevice;
		directory = newDirectory;
		g_free(name);
		name = g_strdup(newName);
	}
	return B_OK;
}

status_t 
EntryRef::SetTo(const char *path, bool resolve)
{
	if (path == NULL) {
		MakeEmpty();
		return 0;
	}
		
	int32 length = strlen(path);
	if (length == 0) {
		MakeEmpty();
		return 0;
	}

	if (length > 1024)
		return E2BIG;

	const char *lastSlash = strrchr(path, '/');
	int error;

	if (!resolve && lastSlash == NULL && strcmp(path, ".") != 0 
		&& strcmp(path, "..") != 0) {
		// simple case, name in the current directory
		struct stat statBuffer;
		error = stat(".", &statBuffer);
		if (error != 0)
			return error;
	
		return SetTo(statBuffer.st_dev, statBuffer.st_ino, path);
	}
	
	struct stat pathStatBuffer;
	if (resolve)
		error = lstat(path, &pathStatBuffer);
	else
		error = stat(path, &pathStatBuffer);
	if (error != 0)
		return error;

	struct stat parentStatBuffer;
	String parentPath(path);
	parentPath += "/..";
	if (resolve)
		error = lstat(parentPath.CStr(), &parentStatBuffer);
	else
		error = stat(parentPath.CStr(), &parentStatBuffer);
	if (error != 0)
		return error;
	
	if (pathStatBuffer.st_dev == parentStatBuffer.st_dev
		&& pathStatBuffer.st_ino == parentStatBuffer.st_ino)
		// must be the "/", parent directory and item inodes match
		return SetTo(pathStatBuffer.st_dev, pathStatBuffer.st_ino, ".");

	// traverse the parent directory, looking for the item
	DIR *dir = opendir(parentPath.CStr());
	if (dir == NULL)
		return errno;
	
	error = 0;

	char *direntBuffer = new char [sizeof(struct dirent) + PATH_MAX + 1];
	for (;;) {
		struct dirent *nextDirent;
		int count = readdir_r(dir, (struct dirent *)direntBuffer, &nextDirent);
		if (!count) {
			error = ENOENT;
			break;
		}
		if (nextDirent->d_ino == pathStatBuffer.st_ino) {
			if (pathStatBuffer.st_dev == parentStatBuffer.st_dev) {
				// found it - use d_name as the cannonical name of the item
				error = SetTo(pathStatBuffer.st_dev, pathStatBuffer.st_ino,
					nextDirent->d_name);
				break;
			} else {
				TRESPASS();
				// what does this mean?
			}			
		}
	}
	delete [] direntBuffer;
	closedir(dir);
	
	return error;
}

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
