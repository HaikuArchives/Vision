// BMessage-like class
//
// see License at the end of file

#ifndef FMESSAGE_H__
#define FMESSAGE_H__

#include "PlatformDefines.h"
#include "CString.h"

class FMessageDetails;
class FMessenger;
class EntryRef;

class FMessage {
public:
	FMessage();
	FMessage(uint32);
	FMessage(const FMessage &);

	virtual ~FMessage();

	uint32 what;

	const FMessage &operator=(const FMessage &);

	status_t AddData(const char *name, uint32 type, const void *data, int32 numBytes);
	status_t FindData(const char *, uint32 type, const void **data, int32 *numBytes) const;
	status_t FindData(const char *, uint32 type, int32, const void **data, 
		int32 *numBytes) const;
	bool HasData(const char *, uint32) const;

	status_t AddBool(const char *, bool);
	status_t FindBool(const char *, bool *) const;
	status_t FindBool(const char *, int32, bool *) const;
	bool HasBool(const char *);
	
	status_t AddInt8(const char *, int8);
	status_t FindInt8(const char *, int8 *) const;
	status_t FindInt8(const char *, int32, int8 *) const;
	bool HasInt8(const char *);

	status_t AddInt16(const char *, int16);
	status_t FindInt16(const char *, int16 *) const;
	status_t FindInt16(const char *, int32, int16 *) const;
	bool HasInt16(const char *);

	status_t AddInt32(const char *, int32);
	status_t FindInt32(const char *, int32 *) const;
	status_t FindInt32(const char *, int32, int32 *) const;
	bool HasInt32(const char *);

	status_t AddInt64(const char *, int64);
	status_t FindInt64(const char *, int64 *) const;
	status_t FindInt64(const char *, int32, int64 *) const;
	bool HasInt64(const char *);

	status_t AddString(const char *, const char *);
	status_t FindString(const char *, const char **) const;
	status_t FindString(const char *, int32, const char **) const;
	bool HasString(const char *);

	status_t AddRef(const char *, const EntryRef *);
	status_t FindRef(const char *, EntryRef *) const;
	status_t FindRef(const char *, int32, EntryRef *) const;
	bool HasRef(const char *) const;

	status_t AddPointer(const char *, const void *);
	status_t FindPointer(const char *, void **) const;
	status_t FindPointer(const char *, int32, void **) const;
	bool HasPointer(const char *);
	
	status_t AddPoint(const char *, const BPoint *);
	status_t FindPoint(const char *, BPoint *) const;
	status_t FindPoint(const char *, int32, BPoint *) const;
	bool HasPoint(const char *);
	
	void GetInfo(const char *, uint32 *type, int32 *count);
	
	bool WasDropped() const;
	FPoint DropPoint() const;
	
	String &PrintToStream(String &) const;
	void PrintToStream() const;
	
	FMessenger ReturnAddress() const;

	// only Gnome drag&drop emulator calls this
	void SetWasDropped();
	void SetDropPoint(FPoint);
private:
	template <class type, uint32 typeCode> void AddScalar(const char *, const type &data);
	template <class type, uint32 typeCode> status_t FindScalar(const char *, int32 index, type *data) const;

	FMessageDetails *fDetails;
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
