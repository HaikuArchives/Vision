// BNode-like class
//
// see License at the end of file

#include "Node.h"

#include <byteorder.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <Entry.h>
#include <FindDirectory.h>
#include <Path.h>
#include <ObjectList.h>

struct AttributeEntry {
	String fName;
	uint32 fType;
	size_t fSize;
	mutable void *fData;
	
	AttributeEntry(const char *name, uint32 type, size_t size, const void *data);
	AttributeEntry(const char *stream);
	~AttributeEntry();

	
	size_t FlattenedSize() const;
	status_t Save(int);

private:
	AttributeEntry(const AttributeEntry &);
	const AttributeEntry &operator=(const AttributeEntry &);
};

class AttributeCache {
public:
	AttributeCache(int = -1);
	status_t Save(int);

	status_t GetAttrInfo(const char *, attr_info *);
	ssize_t ReadAttr(const char *, uint32, size_t, void *, uint32);
	ssize_t WriteAttr(const char *, uint32, size_t, const void *, uint32);
	status_t RemoveAttr(const char *);

	static bool GetAttributeDirPath(FPath *);
private:
	status_t Read(int);
	
	ObjectList<AttributeEntry> fEntries;
};

FNode::FNode()
	:	fFd(-1),
		fStatus(B_NO_INIT),
		fAttributeCache(NULL),
		fNoAttributesFound(false)
{
}

FNode::FNode(const EntryRef *ref)
	:	fAttributeCache(NULL),
		fNoAttributesFound(false)
{
	fFd = open(ref->Path(), O_RDWR);
	if (fFd < 0)
		fFd = open(ref->Path(), O_RDONLY);
	if (fFd < 0) 
		fStatus = errno;
	else
		fStatus = B_OK;
}

FNode::~FNode()
{
	SetFD(-1);
	delete fAttributeCache;
}

status_t 
FNode::GetStat(struct stat *statStruct)
{
	if (fStatus != B_OK)
		return B_NO_INIT;

	return fstat(fFd, statStruct);
}

status_t 
FNode::InitCheck() const
{
	return fStatus;
}

status_t 
FNode::Sync()
{
	return fsync(fFd);
}

void 
FNode::SetFD(int fd)
{
	if (fFd >= 0)
		close(fFd);

	fFd = fd;
	if (fFd >= B_OK)
		fStatus = B_OK;
	else
		fStatus = B_NO_INIT;
}

AttributeEntry::AttributeEntry(const char *name, uint32 type, size_t size, const void *data)
	:	fName(name),
		fType(type),
		fSize(size),
		fData(malloc(size))
{
	memcpy(fData, data, size);
}


AttributeEntry::AttributeEntry(const char *stream)
{
	// skip the size of the entire block
	stream += sizeof(int32);
	
	// skip the size of the string
	stream += sizeof(int32);
	
	// assume it's a properly terminated C string
	fName = stream;
	stream += fName.Length() + 1;
	
	// read in the type
	fType = *((uint32 *)stream);
	stream += sizeof(uint32);

	// read in the size and data
	fSize = B_BENDIAN_TO_HOST_INT32(*((size_t *)stream));
	stream += sizeof(size_t);
	
	fData = malloc(fSize);
	memcpy(fData, stream, fSize);
}


#if 0
AttributeEntry::AttributeEntry(const AttributeEntry &stealThis)
	:	fName(stealThis.fName),
		fType(stealThis.fType),
		fSize(stealThis.fSize),
		fData(stealThis.fData)
{
	// This is only used for stuffing vector with a new item.
	// To avoid the overhead of having to allocate a new data
	// buffer, just steal the original one here
	stealThis.fData = NULL;
}

const AttributeEntry &
AttributeEntry::operator=(const AttributeEntry &stealThis)
{
	fName = stealThis.fName;
	fType = stealThis.fType;
	fSize = stealThis.fSize;
	fData = stealThis.fData;
	stealThis.fData = NULL;

	return *this;
}
#endif

AttributeEntry::~AttributeEntry()
{
	free(fData);
}

size_t 
AttributeEntry::FlattenedSize() const
{
	// room for the size of the entired flattened block
	size_t result = sizeof(int32);

	// room for name size
	result += sizeof(int32);
	
	// room for name string
	result += fName.Length() + 1;

	// room for type
	result += sizeof(uint32);
	
	// room for data size
	result += sizeof(size_t);
	
	// room for data
	result += fSize;

	return result;
}

status_t 
AttributeEntry::Save(int fd)
{
	int32 bufferSize = FlattenedSize();

	// allocate the flattenning buffer
	char *buffer = new char[bufferSize];
	char *scanner = buffer;

	// save entire block size	
	*((int32 *)scanner) = B_HOST_TO_BENDIAN_INT32(bufferSize);
	scanner += sizeof(int32);

	// save name size
	*((int32 *)scanner) = B_HOST_TO_BENDIAN_INT32(fName.Length()); 
	scanner += sizeof(int32);
	
	// save name string
	strcpy(scanner, fName.CStr());
	scanner += fName.Length() + 1;

	// save type
	*((uint32 *)scanner) = fType;
	scanner += sizeof(uint32);
	
	// save data size
	*((size_t *)scanner) = B_HOST_TO_BENDIAN_INT32(fSize);
	scanner += sizeof(size_t);

	// save data
	assert(buffer + bufferSize == scanner + fSize);
	memcpy(scanner, fData, fSize);

	// write out the result
	status_t result = write(fd, buffer, bufferSize);

	delete [] buffer;
	
	return result;
}


AttributeCache::AttributeCache(int fd)
	:	fEntries(10, true)
{
	if (fd >= 0)
		Read(fd);
	// fixme:
	// fix error handling here
}


status_t 
AttributeCache::Save(int fd)
{
	int32 result = B_OK;

	int32 count = fEntries.CountItems();
	for (int32 i = 0; i < count; i++) {
		result = fEntries.ItemAt(i)->Save(fd);
		if (result != B_OK)
			break;
	}

	return result;
}

status_t 
AttributeCache::Read(int fd)
{	
	int32 bufferSize = sizeof(int32);
	char *buffer = (char *)malloc(sizeof(int32));
	int32 result = B_OK;
	
	for (;;) {
		// read how much we have left -- read the first word of the
		// buffer that holds the size
		result = read(fd, buffer, sizeof(int32));
		if (result < (ssize_t)sizeof(int32)) {
			// none left, bail
			result = B_OK;
			break;
		}
		
		int32 blockSize = B_BENDIAN_TO_HOST_INT32(*(int32 *)buffer);
		if (blockSize > bufferSize) {
			// make sure we have enough space in the buffer
			bufferSize = blockSize;
			if (bufferSize > 10 * 1024) {
				result = B_ERROR;
				break;
			}
			buffer = (char *)realloc(buffer, bufferSize);
		}
		// now that we know how much we have left, read the rest of the buffer
		result = read(fd, &buffer[sizeof(int32)],
			blockSize - sizeof(int32));
		
		if (result < 0)
			break;

		if (result < bufferSize - (ssize_t)sizeof(int32)) {
			// unexpected end of data, bail
			result = B_ERROR;
			break;
		}

		// buffer read fine, unflatten
		fEntries.AddItem(new AttributeEntry(buffer));
	}
	free (buffer);
	return result;
}

bool 
AttributeCache::GetAttributeDirPath(FPath *path)
{
	if (find_directory(B_USER_SETTINGS_DIRECTORY, path, true) != B_OK)
		return false;

	path->Append("Eddie");
	mkdir(path->Path(), 0777);

	path->Append(".attributes");
	mkdir(path->Path(), 0777);


	struct stat tmp;	
	return lstat(path->Path(), &tmp) == 0;
}

status_t 
AttributeCache::GetAttrInfo(const char *name, attr_info *attrInfo)
{
	int32 count = fEntries.CountItems();
	for (int32 i = 0; i < count; i++) {
		if (fEntries.ItemAt(i)->fName == name) {
			attrInfo->type = fEntries.ItemAt(i)->fType;
			attrInfo->size = fEntries.ItemAt(i)->fSize;
			return B_OK;
		}
	}
	return B_ERROR;
}

ssize_t 
AttributeCache::ReadAttr(const char *name, uint32 type, size_t size,
	void *data, uint32 offset)
{
	int32 count = fEntries.CountItems();
	for (int32 i = 0; i < count; i++) {
		if (fEntries.ItemAt(i)->fName == name && fEntries.ItemAt(i)->fType == type) {
			ssize_t returnSize = size;
			if ((ssize_t)fEntries.ItemAt(i)->fSize - (ssize_t)offset < returnSize)
				returnSize = fEntries.ItemAt(i)->fSize - offset;
			
			if (returnSize <= 0)
				return 0;

			memcpy(data, fEntries.ItemAt(i)->fData, returnSize);

			return returnSize;
		}
	}
	return B_ERROR;
}

ssize_t 
AttributeCache::WriteAttr(const char *name, uint32 type, size_t size,
	const void *data, uint32 )
{
	RemoveAttr(name);
	AttributeEntry tmp(name, type, size, data);
	fEntries.AddItem(new AttributeEntry(name, type, size, data));
	
	return size;
}

status_t 
AttributeCache::RemoveAttr(const char *name)
{
	int32 count = fEntries.CountItems();
	for (int32 i = 0; i < count; i++) 
		if (fEntries.ItemAt(i)->fName == name) {
			fEntries.RemoveItemAt(i);
			return B_OK;
		}
	
	return ENOENT;
}


void 
FNode::LoadAttributeCacheIfNeeded()
{
	if (!fAttributeCache && !fNoAttributesFound) {
		FPath attributeDirectoryPath;
		if (!AttributeCache::GetAttributeDirPath(&attributeDirectoryPath))
			return;

		String nameBase;
		nameBase << fInode;
		String path;
		for (int32 count = 0; ; count++) {
			// Attributes are stored under a name that matches their orignating
			// file's inode. Since multiple files can share the same inode from
			// different volumes, we also tag on a number that makes them unique.
			// To fully identify the file, we also match it's path
			//
			// keep going through the files that match this inode
			// until we hit one that matches our path hash too

			path = attributeDirectoryPath.Path();
			path << "/" << nameBase << "-";
			path << count;
			
			int fd = open(path.CStr(), O_RDONLY);
			if (fd < 0) {
				// we didn't get any matching attribute file
				fNoAttributesFound = true;
				return;
			}
			
			bool attributeFileMatch = false;

			int32 pathLength;
			char pathBuffer[2048];

			if (read(fd, &pathLength, sizeof(int32)) == sizeof(int32)) {
				pathLength = B_BENDIAN_TO_HOST_INT32(pathLength);
				if (pathLength > 0
					&& pathLength < 2048
					&& read(fd, pathBuffer, pathLength) == pathLength
					&& fOriginalFileName == pathBuffer) {
					// got a match, read in the attributes
					fAttributeCache = new AttributeCache(fd);
					attributeFileMatch = true;
				} 
			}
				
			if (!attributeFileMatch) {
				// while we are at it, check if the unmatched
				// attribute file is stale and delete if so
				struct stat tmp;
				if (lstat(pathBuffer, &tmp) != 0) {
					// the file the attribute belongs to is dead,
					// kill it
					unlink(path.CStr());
				}
				
			}
			close(fd);
		
			if (fAttributeCache)
				// done
				break;
		}
	}
}

status_t 
FNode::SaveAttributeCache()
{
	if (!fAttributeCache)
		return B_OK;
	
	FPath attributeDirectoryPath;
	if (!AttributeCache::GetAttributeDirPath(&attributeDirectoryPath))
		return ENOENT;

	status_t result = B_OK;

	String nameBase;
	nameBase << fInode;

	String path;
	for (int32 count = 0; ; count++) {
		path = attributeDirectoryPath.Path();
		path << "/" << nameBase << "-";
		path << count;
		
		int fd = open(path.CStr(), O_RDONLY);
		if (fd < 0)
			break;
		// the attribute file already exists, check if it belongs to us
		bool validForeignAttribute = false;
		char pathBuffer[2048];
		int32 pathLength;

		if (read(fd, &pathLength, sizeof(int32)) == sizeof(int32)) {
			pathLength = B_BENDIAN_TO_HOST_INT32(pathLength);
			if (pathLength > 0
				&& pathLength < 2048
				&& read(fd, pathBuffer, pathLength) == pathLength) {
				// size is right
			
				if (fOriginalFileName != pathBuffer) {
					// it's not us
					struct stat tmp;
					if (lstat(pathBuffer, &tmp) == 0)
						// points to a valid file
						validForeignAttribute = true;
				}
			}
		}
		close(fd);

		if (!validForeignAttribute) {
			// delete the old file
			unlink(path.CStr());
			break;
		}
	}

	int fd = open(path.CStr(), O_CREAT | O_RDWR, 0666);
	if (fd < 0)
		return fd;
	
	int32 length = B_HOST_TO_BENDIAN_INT32(fOriginalFileName.Length() + 1);
	result = write(fd, &length, sizeof(int32));
	
	// write out the header that identifies us		
	result = write(fd, fOriginalFileName.CStr(), fOriginalFileName.Length() + 1);
	
	// write out the attributes
	result = fAttributeCache->Save(fd);

	close(fd);

	fNoAttributesFound = false;
	return result;
}

status_t 
FNode::GetAttrInfo(const char *name, attr_info *attrInfo)
{
	LoadAttributeCacheIfNeeded();
	if (!fAttributeCache)
		return ENOENT;

	return fAttributeCache->GetAttrInfo(name, attrInfo);
}

ssize_t 
FNode::ReadAttr(const char *name, uint32 type, uint32 offset, void *data, size_t size)
{
	LoadAttributeCacheIfNeeded();
	if (!fAttributeCache)
		return B_ERROR;

	return fAttributeCache->ReadAttr(name, type, size, data, offset);
}

ssize_t 
FNode::WriteAttr(const char *name, uint32 type, uint32 offset, const void *data, size_t size)
{
	LoadAttributeCacheIfNeeded();
	
	if (!fAttributeCache)
		fAttributeCache = new AttributeCache();
	
	ssize_t result = fAttributeCache->WriteAttr(name, type, size, data, offset);

	if (result > 0)
		result = SaveAttributeCache();

	return result;
}

status_t 
FNode::RemoveAttr(const char *name)
{
	LoadAttributeCacheIfNeeded();
	if (!fAttributeCache)
		return B_ERROR;

	status_t result = fAttributeCache->RemoveAttr(name);
	if (result > 0)
		result = SaveAttributeCache();

	return result;
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
