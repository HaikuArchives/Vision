// File
//
// See License at the end fo this file

#ifndef FFILE_H
#define FFILE_H

#include "PlatformDefines.h"

#include "StorageDefs.h"

#include "Entry.h"
#include "Node.h"
#include "DataIO.h"
#include <fcntl.h>

class FDirectory;

class FFile : public FNode, public FPositionIO {
public:
	FFile();
	FFile(const FFile &);
	FFile(const EntryRef *, uint32 mode);
	FFile(const FEntry *, uint32 mode);
	FFile(const char *, uint32 mode);
	FFile(const FDirectory *, const char *, uint32 mode);

	virtual ~FFile();


	status_t SetTo(const EntryRef *, uint32);
	status_t SetTo(const FEntry *, uint32);
	status_t SetTo(const char *, uint32);
	status_t SetTo(const FDirectory *, const char *, uint32 mode);
	
	FFile &operator=(const FFile &);

	
	virtual ssize_t Read(void *, size_t);
	virtual ssize_t Write(const void *, size_t);
	virtual off_t Seek(off_t, uint32);

	virtual	ssize_t ReadAt(off_t pos, void *buffer, size_t size);
	virtual	ssize_t WriteAt(off_t pos, const void *buffer, size_t size);
	virtual	off_t Position() const;

	status_t SetSize(off_t);
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
