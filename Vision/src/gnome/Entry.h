// entry_ref, Entry
//
// See License at the end fo this file

#ifndef FENTRY__
#define FENTRY__

#include "PlatformDefines.h"
#include "StdDefs.h"
#include "CString.h"

class FPath;
struct stat;
class FDirectory;

class EntryRef {
public:
	EntryRef();
	EntryRef(const EntryRef &);
	EntryRef(const char *);

	~EntryRef();
	
	status_t SetTo(const char *);
	status_t SetTo(const EntryRef &);
	status_t SetName(const char *);

	bool operator==(const EntryRef &) const;
	bool operator!=(const EntryRef &) const;
	EntryRef &operator=(const EntryRef &);


	status_t set_name(const char *name)
		{ return SetName(name); }

	const char *name;

	const char *Path() const;
private:

	void SetUpName();

	String fPath;
	
	friend class FEntry;
};

inline status_t get_ref_for_path(const char *path, EntryRef *ref)
	{ return ref->SetTo(path); }

class FEntry {
public:
	FEntry();
	FEntry(const char *, bool resolve = true);
	FEntry(const EntryRef *, bool resolve = true);
	FEntry(const FDirectory *, const char *);

	status_t SetTo(const char *, bool resolve = true);
	status_t SetTo(const EntryRef *, bool resolve = true);
	status_t SetTo(const FDirectory *, const char *);

	status_t GetRef(EntryRef *) const;
	status_t GetPath(FPath *) const;
	status_t GetParent(FEntry *) const;
	status_t GetParent(FDirectory *) const;

	status_t InitCheck() const;
	bool Exists() const;

	bool IsFile() const;
	bool IsDirectory() const;
	bool IsSymLink() const;

	status_t GetName(char *) const;
	status_t GetStat(struct stat *);

	status_t Remove();

private:
	EntryRef fRef;
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
