// BPath-like class
//
// see License at the end of file

#ifndef FPATH_H__
#define FPATH_H__

#include "PlatformDefines.h"

#include "CString.h"

class FDirectory;
class EntryRef;
class FEntry;

class FPath {
public:
	FPath();
	FPath(const char *path, const char *leaf = NULL, bool normalize = false);
	FPath(const FDirectory *dir, const char *leaf = NULL, bool normalize = false);
	FPath(const entry_ref *);
	FPath(const FEntry *);
	
	status_t SetTo(const char *);
	status_t SetTo(const FDirectory *, const char *);
	status_t SetTo(const EntryRef *);
	status_t SetTo(const FEntry *);
	
	const char *Path() const;
	status_t Append(const char *);

	const char *Leaf() const;
	status_t GetParent(FPath *) const;

	status_t InitCheck() const;
	
	static status_t Normalize(const char *path, String &result);
private:

	String fPath;
	status_t fStatus;
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
