// BDataIO, BPositionIO, BMemoryIO and BMallocIO-like classes
//
// see License at the end of file

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Debug.h"

#include "DataIO.h"


FDataIO::FDataIO(const FDataIO &dataIO)
{
	this->operator=(dataIO);
}

FDataIO &
FDataIO::operator=(const FDataIO &dataIO)
{
	if (&dataIO == this)
		return *this;

	return *this;
}

FMemoryIO::FMemoryIO(void *p, size_t length)
{
	fReadOnly = false;
	fData = (char *)p;
	fLength = length;
	fPosition = 0;
}

FMemoryIO::FMemoryIO(const void *p, size_t length)
{
	fReadOnly = true;
	fData = (char *)p;
	fLength = length;
	fPosition = 0;
}

FMemoryIO::~FMemoryIO()
{
}


FMemoryIO::FMemoryIO(const FMemoryIO &memoryIO)
	:	FPositionIO()
{
	this->operator=(memoryIO);
}


FMemoryIO &
FMemoryIO::operator=(const FMemoryIO &memoryIO)
{
	if (&memoryIO == this)
		return *this;
		
	fReadOnly = memoryIO.fReadOnly;
	fData = memoryIO.fData;
	fLength = memoryIO.fLength;
	fPosition = memoryIO.fPosition;

	return *this;
}

bool
FMemoryIO::DoSafetyChecks(off_t pos, const void *buffer, size_t size)
{	
	if (fData == NULL)
		return false;

	if (buffer == NULL)
		return false;
		
	if (pos >= (off_t)fLength)
		return false;
	
	if (size >= fLength)
		return false;
		
	if (pos <= 0 || size <= 0)
		return false;
		
	return true;
}

ssize_t
FMemoryIO::ReadAt(off_t position, void *buffer, size_t size)
{	
	// Safety checks
	if (!DoSafetyChecks (position, buffer, size))
		return 0;
	
	Seek(position, SEEK_SET);
	
	// Calculate amount to read
	size_t amountToRead;
	
	if (fLength - fPosition >= size)
		amountToRead = size;
	else
		amountToRead = fLength - fPosition;
	
	char *cursor = fData;
	cursor += fPosition;
	
	memcpy(buffer, cursor, amountToRead);
	
	fPosition += amountToRead;
	
	return amountToRead;
}

ssize_t
FMemoryIO::WriteAt(off_t position, const void *buffer, size_t size)
{
	if (fReadOnly)
		return 0;
	
	// Safety checks
	if (!DoSafetyChecks (position, buffer, size))
		return 0;
		
	Seek(position, SEEK_SET);

	// Calculate amount to write
	size_t amountToWrite;
	if (fLength - fPosition >= size)
		amountToWrite = size;
	else
		amountToWrite = fLength - fPosition;
	char *cursor = fData;
	cursor += fPosition;
	
	memcpy (cursor, buffer, amountToWrite);
		
	return 0;
}

off_t
FMemoryIO::Seek(off_t position, uint32 seekMode)
{
	// Safety checks
	if (fData == 0)
		return 0;
	
	if (position > (off_t)fLength)
		return 0;
		
	off_t retVal = 0;
					
	switch (seekMode) {
		case SEEK_SET:
			if (position > (off_t)fLength)
				return 0;
			fPosition = position;
			retVal = fPosition;
			break;
			
		case SEEK_CUR:
			if (position + fPosition > (off_t)fLength)
				fPosition = fLength;
			else
				fPosition += position;
			retVal = fPosition;
			break;
					
		case SEEK_END:
			if (fLength - position > 0)
				fPosition -= position;
			else
				fPosition = 0;
			retVal = fPosition;
			break;
			
		default:
			TRESPASS ();
			break;			
	
	}
	
	return retVal;
}

FMallocIO::FMallocIO()
	:	FMemoryIO((char *)0, (size_t)0)
{
	fBlockSize = 256;
	fLength = 0;
	fData = 0;
	fPosition = 0;
	fReadOnly = false;
}


FMallocIO::~FMallocIO()
{
	free (fData);
}


ssize_t
FMallocIO::WriteAt(off_t pos, const void *buffer, size_t size)
{
	// We may need to allocate the initial buffer
	if (fData == 0)
		fData = (char *)malloc (size);
		
	// We may need to allocate additional memory
	if (pos + size > fLength) {
		char *newData = (char *)malloc (pos + size);		
		memcpy (newData, fData, fLength);
		fLength = pos + size;
		free (fData);
		fData = newData;		
	}
	
	return FMemoryIO::WriteAt(pos, buffer, size);
}


status_t
FMallocIO::SetSize(off_t size)
{
	if (size == 0)
		return 0;
	
	if (size == (off_t)fLength)
		return 1;
					
	// We may need to allocate the initial buffer
	if (fData == 0)
		fData = (char *)malloc (size);
			
	char *newData;
	
	if (size < (off_t)fLength) {
		// Handle shrink case
		newData = (char *)malloc (size);
		memcpy (newData, fData, size);
		free (fData);
		fLength = size;
		fData = newData;
	} else {
		// Handle grow case
		newData = (char *)malloc (size);
		memcpy (newData, fData, fLength);
		free (fData);
		fData = newData;	
	}
		
	return 1;
}


void
FMallocIO::SetBlockSize(size_t blocksize)
{
	if (blocksize > 0)
		fBlockSize = blocksize;
}

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
