// find_directory
//
// see License at the end of file

#include "PlatformDefines.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "Directory.h"
#include "FindDirectory.h"
#include "Entry.h"
#include "Path.h"

struct WellKnownDirectory {
	uint32 selector;
	const char *path;
};

#define HOME "$(HOME)"
const WellKnownDirectory directories [] = {
	{ B_USER_DIRECTORY, HOME },
	{ B_USER_SETTINGS_DIRECTORY, HOME "/.config/settings" },
	{ B_USER_ADDONS_DIRECTORY, HOME "/.config/addons" },
	{ B_COMMON_TEMP_DIRECTORY, "/tmp" },
	{ 0, NULL }
};

FindDirectory::FindDirectory()
	:	fHome(getenv("HOME"))
{
}

status_t 
FindDirectory::Find(uint32 selector, FPath *result, bool createIfNeeded)
{
	for (int32 index = 0; ; index++) {
		if (directories[index].selector == 0)
			break;
		if (directories[index].selector == selector) {
			String path(directories[index].path);
			findDirectory.SubstituteHome(path);
			if (createIfNeeded) {
				status_t error = MakeDirectory(path.CStr());
				if (error != B_OK) 
					return ENOENT;
			}
			FEntry entry(path.CStr());
			if (!entry.Exists())
				return ENOENT;

			result->SetTo(path.CStr());
			return B_OK;
		}
	}
	return ENOENT;
}

status_t 
FindDirectory::MakeDirectory(const char *path)
{
	struct stat statBuffer;
	if (stat(path, &statBuffer) == 0)
		return B_OK;
	return FDirectory::CreateDirectory(path);
}

void 
FindDirectory::SubstituteHome(String &path) const
{
	if (path.FindFirst(HOME) != 0)
		return;

	path.ReplaceFirst(HOME, fHome.CStr());
}

status_t 
find_directory(uint32 selector, FPath *path, bool createIfNeeded)
{
	return FindDirectory::Find(selector, path, createIfNeeded);
}

FindDirectory FindDirectory::findDirectory;

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
