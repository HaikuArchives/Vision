// BDataIO, BPositionIO, BMemoryIO and BMallocIO-like classes
//
// see License at the end of file


#ifndef	DATAIO__
#define	DATAIO__

#include "PlatformDefines.h"

#include <sys/types.h>


class FDataIO {
public:
	FDataIO()
		{}
	virtual ~FDataIO()
		{}
	
	virtual	ssize_t Read(void *buffer, size_t size) = 0;
	virtual	ssize_t Write(const void *buffer, size_t size) = 0;

private:
	FDataIO(const FDataIO &);
	FDataIO &operator=(const FDataIO &);
};


class FPositionIO : public FDataIO {
public:
	FPositionIO() {}

	virtual	ssize_t Read(void *, size_t )
		{ return 0; }
	virtual	ssize_t Write(const void *, size_t )
		{ return 0; }

	virtual	ssize_t ReadAt(off_t pos, void *buffer, size_t size) = 0;
	virtual	ssize_t WriteAt(off_t pos, const void *buffer, size_t size) = 0;

	virtual off_t Seek(off_t position, uint32 seek_mode) = 0;
	virtual	off_t Position() const = 0;

	virtual status_t SetSize(off_t ) { return 0; }
};



class FMemoryIO : public FPositionIO {
public:
	FMemoryIO(void *, size_t);
	FMemoryIO(const void *, size_t);
	virtual ~FMemoryIO();

	virtual	ssize_t ReadAt(off_t pos, void *buffer, size_t size);
	virtual	ssize_t WriteAt(off_t pos, const void *buffer, size_t size);

	virtual	off_t Seek(off_t pos, uint32 seek_mode);
	virtual off_t Position() const { return fPosition; }

private:
	FMemoryIO(const FMemoryIO &);
	FMemoryIO &operator=(const FMemoryIO &);

	virtual bool DoSafetyChecks(off_t pos, const void *buffer, size_t size);

protected:
	bool fReadOnly;
	char *fData;
	size_t fLength;
	off_t fPosition;
};


class FMallocIO : public FMemoryIO {
public:
	FMallocIO();
	virtual ~FMallocIO();

	ssize_t	WriteAt(off_t pos, const void *buffer, size_t size);

	void SetBlockSize(size_t blocksize);
	status_t SetSize(off_t size);

	const void *Buffer() const
		{ return fData; }
	size_t BufferLength() const
		{ return fLength; }
	
private:
	size_t fBlockSize;
};

#endif

/*
License

Terms and Conditions

Copyright (c) 1999-2001, Gene Ragan

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
