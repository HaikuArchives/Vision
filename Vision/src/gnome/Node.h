// BNode-like class
//
// see License at the end of file

#ifndef FNODE_H__
#define FNODE_H__

#include "PlatformDefines.h"

#include "CString.h"
#include "fs_attr.h"
#include <sys/stat.h>

struct stat;
class FEntry;
class EntryRef;
class AttributeCache;

class FNode {
public:
	FNode();
	FNode(const EntryRef *);
	virtual ~FNode();
	
	virtual status_t GetStat(struct stat *);

	status_t InitCheck() const;
	status_t Sync();
	void Unset();

	virtual status_t GetAttrInfo(const char *, attr_info *);

	virtual ssize_t ReadAttr(const char *, uint32, uint32 offset,
		void *, size_t size);
	virtual ssize_t WriteAttr(const char *, uint32, uint32 offset,
		const void *, size_t size);
	virtual status_t RemoveAttr(const char *);

protected:
	void SetFD(int);
	
	void LoadAttributeCacheIfNeeded();
	status_t SaveAttributeCache();
	
	int fFd;
	status_t fStatus;

	// attribute support
	String fOriginalFileName;
	__ino_t fInode;
	AttributeCache *fAttributeCache;
	bool fNoAttributesFound;
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
