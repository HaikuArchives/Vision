// BDirectory-like class
//
// see License at the end of file

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include "PlatformDefines.h"

#include "Entry.h"
#include <dirent.h>

class FFile;
class FSymLink;
class EntryRef;
class FEntry;


class FDirectory {
public:
	FDirectory();
	FDirectory(const char *);
	FDirectory(const FDirectory &);
	FDirectory(const FDirectory *, const char *);
	FDirectory(const FEntry *);
	FDirectory(const EntryRef *);
	
	virtual ~FDirectory();


	status_t SetTo(const char *);
	status_t SetTo(const FEntry *);
	status_t SetTo(const EntryRef *);
	status_t SetTo(const FDirectory *, const char *);

	status_t GetRef(EntryRef *) const;
	status_t GetEntry(FEntry *) const;
	status_t GetPath(FPath *) const;

	bool operator==(const FDirectory &) const;
	FDirectory &operator=(const FDirectory &);

	bool Contains(const char *) const;
	
	status_t InitCheck() const;

	status_t CreateFile(const char *, FFile *, bool);
	status_t CreateDirectory(const char *path, FDirectory *dir);
	status_t CreateSymLink(const char *, const char *, FSymLink *);

	// not in BeOS
	static status_t CreateDirectory(const char *);
		// create a directory from a path, creating all
		// the neccessary parents if needed.

	status_t Rewind();
	status_t GetNextRef(EntryRef *);
	status_t GetNextEntry(FEntry *, bool = true);

	status_t FindEntry(const char *, FEntry *, bool traverse = false) const;
	
private:
	void Close();

	EntryRef fRef;
	status_t fStatus;
	DIR *fDirectoryIterator;
	dirent *fDirentBuffer;
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
