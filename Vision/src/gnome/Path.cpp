// BPath-like class
//
// see License at the end of file

#include "Path.h"
#include "Directory.h"

#include <errno.h>
#include <string.h>

FPath::FPath()
	:	fStatus(B_NO_INIT)
{
}

FPath::FPath(const char *path, const char */*leaf*/, bool /*normalize*/)
	:	fPath(path),
		fStatus(B_OK)
{
}

FPath::FPath(const EntryRef *ref)
{
	SetTo(ref);
}

FPath::FPath(const FEntry *entry)
{
	SetTo(entry);
}

FPath::FPath(const BDirectory *directory, const char *file, bool /*normalize*/)
{
	SetTo(directory, file);
}

const char *
FPath::Path() const
{
	return fPath.CStr();
}

status_t 
FPath::Append(const char *path)
{
	if (!fPath.Length() || fPath[fPath.Length() - 1] != '/')
		fPath += '/';
	fPath += path;
	fStatus = B_OK;

	return fStatus;
}

status_t 
FPath::SetTo(const char *path)
{
	fPath = path;
	fStatus = B_OK;

	return fStatus;
}

status_t 
FPath::SetTo(const BDirectory *directory, const char *file)
{
	fStatus = directory->GetPath(this);
	if (fStatus != B_OK)
		return fStatus;

	fStatus = Append(file);
	return fStatus;
}

status_t 
FPath::SetTo(const EntryRef *ref)
{
	fPath = ref->Path();
	fStatus = B_OK;

	return fStatus;
}

status_t 
FPath::SetTo(const FEntry *entry)
{
	fStatus = entry->GetPath(this);

	return fStatus;
}

status_t 
FPath::InitCheck() const
{
	return fStatus;
}

const char *
FPath::Leaf() const
{
	const char *result = (const char *)strrchr((char *)fPath.CStr(), '/');
	if (!result || result[1] == '\0')
		return fPath.CStr();

	return result + 1;
}

status_t 
FPath::GetParent(FPath *path) const
{
	String parentPath(fPath);
	int32 nameOffset = parentPath.FindLast('/');
	if (nameOffset == -1) 
		return ENOENT;

	parentPath.Truncate(nameOffset);
	path->SetTo(parentPath.CStr());
	return B_OK;
}


static status_t
StripDoubleSlash(String &result)
{
	// shake out all the multiple // sequences
	for (;;) {
		const char *start = result.CStr();
		const char *doubleSlash = strstr(start, "//");
		if (!doubleSlash)
			break;
		
		// got one, squeeze it out
		String tmp;
		tmp.Assign(start, doubleSlash - start);
		tmp+= doubleSlash + 1;
		result = tmp;
	}
	return B_OK;
}

static status_t
StripDot(String &result)
{
	for (;;) {
		// find all the /. sequences, 
		const char *start = result.CStr();
		const char *dot = strstr(start, "/.");
		if (!dot || (dot[2] != '\0' && dot[2] != '/'))
			break;
		
		String tmp;
		tmp.Assign(start, dot - start);
		tmp += dot + 2;
		result = tmp;
	}		

	return B_OK;
}

static status_t
StripDotDot(String &result)
{
	for (;;) {
		// find all the /.. sequences, 
		const char *start = result.CStr();
		const char *dotDot = strstr(start, "/../");
		if (!dotDot)
			break;
		
		if (dotDot == start)
			return EINVAL;
		
		const char *parent = dotDot - 1;

		for (;;) {
			if (*parent == '/') {
				parent++;
				break;
			}
			if (parent == start)
				break;
			--parent;
		}
		String tmp;
		tmp.Assign(start, parent - start);
		tmp += dotDot + 4;
		result = tmp;
	}
	// check for a trailing /.. at the end of the path
	const char *start = result.CStr();
	for (;;) {
		const char *dotDot = strstr(start, "/..");
		if (!dotDot) 
			break;
		
		if (dotDot[3] != '\0') {
			start = dotDot + 3;
			continue;
		}
		
		if (dotDot == start)
			return EINVAL;

		const char *parent = dotDot - 1;
		for (;;) {
			if (*parent == '/') {
				parent++;
				break;
			}
			if (parent == start)
				break;
			--parent;
		}
		result.Truncate(parent - start);
		break;
	}		

	return B_OK;
}

status_t 
FPath::Normalize(const char *path, String &result)
{
	result = path;
	status_t error = StripDoubleSlash(result);
	if (error != B_OK)
		return error;
	
	error = StripDotDot(result);
	if (error != B_OK)
		return error;
	
	return StripDot(result);
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
