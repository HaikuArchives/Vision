// BMessage-like class
//
// see License at the end of file

#include "PlatformDefines.h"

#include "CString.h"
#include "Entry.h"
#include "Message.h"
#include "ObjectList.h"

#include <error.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

class FMessageAtom {
public:
	FMessageAtom(const char *name, uint32 type, const void *data, size_t size);
	FMessageAtom(const FMessageAtom &);
	~FMessageAtom();

	int Compare(const char *name) const;
	int Compare(const char *name, uint32 type) const;
	const void *Data() const;
	size_t Size() const;
	const char *Name() const;
	uint32 Type() const;
	
	void PrintToStream(String &);
	
private:
	String fName;
	uint32 fType;
	size_t fSize;
	char *fData;
};

class FMessageDetails {
public:
	FMessageDetails();
	FMessageDetails(const FMessageDetails &);
	
	void AddData(const char *name, uint32 type, const void *data, int32 size);
	bool FindData(const char *name, uint32  type, int32 index, const void **data, 
		int32 *numBytes) const;
	
	bool WasDropped() const
		{ return fWasDropped; }
	FPoint DropPoint() const
		{ return fDropPoint; }
	
	void SetWasDropped()
		{ fWasDropped = true; }

	void SetDropPoint(FPoint point)
		{ fDropPoint = point; }

	void PrintToStream(String &);

	void GetInfo(const char *, uint32 *type, int32 *count);
	
private:
	ObjectList<FMessageAtom> fData;
	bool fWasDropped;
	FPoint fDropPoint;
};

FMessageAtom::FMessageAtom(const char *name, uint32 type, const void *data, size_t size)
	:	fName(name),
		fType(type),
		fSize(size)
{
	fData = new char [size];
	memcpy(fData, data, size);
}

FMessageAtom::FMessageAtom(const FMessageAtom &cloneThis)
	:	fName(cloneThis.fName),
		fType(cloneThis.fType),
		fSize(cloneThis.fSize)
{
	fData = new char [fSize];
	memcpy(fData, cloneThis.fData, fSize);
}

FMessageAtom::~FMessageAtom()
{
	delete [] fData;
}

int 
FMessageAtom::Compare(const char *name) const
{
	return fName.Compare(name);
}

int 
FMessageAtom::Compare(const char *name, uint32 type) const
{
	int result = fName.Compare(name);
	if (result != 0)
		return result;

	return fType < type ? -1 : (fType > type ? 1 : 0);
}

const char *
FMessageAtom::Name() const
{
	return fName.CStr();
}
	
const void *
FMessageAtom::Data() const
{
	return fData;
}
size_t
FMessageAtom::Size() const
{
	return fSize;
}

uint32 
FMessageAtom::Type() const
{
	return fType;
}

FMessageDetails::FMessageDetails()
	:	fData(10, true),
		fWasDropped(false)
{
}

FMessageDetails::FMessageDetails(const FMessageDetails &copyThis)
	:	fData(copyThis.fData),
		fWasDropped(copyThis.fWasDropped),
		fDropPoint(copyThis.fDropPoint)
{
}



class MatchNameAndTypePredicate : public UnaryPredicate<FMessageAtom> {
public:
	MatchNameAndTypePredicate(const char *name, uint32 type)
		:	fName(name),
			fType(type)
		{}

	virtual int operator()(const FMessageAtom *item) const
		{ return item->Compare(fName, fType); }
private:
	const char *fName;
	uint32 fType;
};

void 
FMessageDetails::AddData(const char *name, uint32 type, const void *data, int32 size)
{
	// this may need fixing -- probably inserts the last atom
	// for given name/type first
	fData.BinaryInsert(new FMessageAtom(name, type, data, size), 
		MatchNameAndTypePredicate(name, type));
}
	
bool 
FMessageDetails::FindData(const char *name, uint32  type, int32 index, const void **data, 
	int32 *numBytes) const
{
	MatchNameAndTypePredicate predicate(name, type);
	const FMessageAtom *item = fData.Search(predicate);
	int resultingIndex = -1;
	if (data)
		resultingIndex = fData.IndexOf(item);
	
	if (resultingIndex >= 0) {
		for (int32 count = 0; ; count++, resultingIndex++) {
			if (count >= index)
				break;
			item = fData.ItemAt(resultingIndex);
			if (!item) {
				resultingIndex = -1;
				break;
			}
			if (item->Compare(name, type) != 0) {
				resultingIndex = -1;
				break;
			}
		}
	}
	if (resultingIndex < 0 || resultingIndex >= fData.CountItems()) {
		*data = NULL;
		*numBytes = 0;
		return false;
	}

	item = fData.ItemAt(resultingIndex);
	*data = item->Data();
	*numBytes = item->Size();
	return true;
}

void 
FMessageDetails::GetInfo(const char *name, uint32 *type, int32 *count)
{
	*count = 0;
	for (int32 index = 0; index < fData.CountItems(); index++)
		if (fData.ItemAt(index)->Compare(name) == 0) {
			(*count)++;
			*type = fData.ItemAt(index)->Type();
		}
}


FMessage::FMessage()
	:	what(0),
		fDetails(NULL)
{
}

FMessage::FMessage(uint32 what)
	:	what(what),
		fDetails(NULL)
{
}

FMessage::FMessage(const FMessage &cloneThis)
	:	what(cloneThis.what),
		fDetails(cloneThis.fDetails ? 
			new FMessageDetails(*cloneThis.fDetails) : NULL)
{
}

const FMessage &
FMessage::operator=(const FMessage &cloneThis)
{
	what = cloneThis.what;
	if (&cloneThis != this) {
		delete fDetails;
		fDetails = NULL;
		if (cloneThis.fDetails)
			fDetails = new FMessageDetails(*cloneThis.fDetails);
	}
	return *this;
}


FMessage::~FMessage()
{
	delete fDetails;
}

status_t 
FMessage::AddData(const char *name, uint32 type, const void *data, int32 numBytes)
{
	if (!fDetails)
		fDetails = new FMessageDetails;

	fDetails->AddData(name, type, data, numBytes);

	return B_OK;
}

status_t 
FMessage::FindData(const char *name, uint32 type, const void **data, int32 *numBytes) const
{
	return FindData(name, type, 0, data, numBytes);
}

status_t 
FMessage::FindData(const char *name, uint32 type, int32 index, 
	const void **data, int32 *numBytes) const
{
	if (fDetails && fDetails->FindData(name, type, index, data, numBytes)) 
		return B_OK;

	*data = 0;
	*numBytes = 0;
	return -1;
}

void 
FMessage::GetInfo(const char *name, uint32 *type, int32 *count)
{
	if (!fDetails) {
		*count = 0;
		return;
	}
	
	fDetails->GetInfo(name, type, count);
}

template <class type, uint32 typeCode> 
void 
FMessage::AddScalar(const char *name, const type &data)
{
	AddData(name, typeCode, &data, sizeof(type));
}

template <class type, uint32 typeCode> 
status_t 
FMessage::FindScalar(const char *name, int32 index, type *data) const
{
	if (fDetails) {
		const void *tempData;
		int32 size;
		if (fDetails->FindData(name, typeCode, index, &tempData, &size)
			&& size == sizeof(type)) {
			memcpy(data, tempData, size);
			return B_OK;
		}
	}
	memset(data, 0, sizeof(type));
	return -1;
}

bool 
FMessage::HasData(const char *name, uint32 typeCode) const
{
	if (!fDetails)
		return false;

	const void *tempData;
	int32 size;
	return fDetails->FindData(name, typeCode, 0, &tempData, &size);
}

status_t 
FMessage::AddBool(const char *name, bool value)
{
	AddScalar<bool, B_BOOL_TYPE>(name, value);

	return B_OK;
}

status_t 
FMessage::FindBool(const char *name, bool *value) const
{
	return FindScalar<bool, B_BOOL_TYPE>(name, 0, value);
}

status_t 
FMessage::FindBool(const char *name, int32 index, bool *value) const
{
	return FindScalar<bool, B_BOOL_TYPE>(name, index, value);
}

status_t 
FMessage::AddInt8(const char *name, int8 value)
{
	AddScalar<int8, B_INT8_TYPE>(name, value);

	return B_OK;
}

status_t 
FMessage::FindInt8(const char *name, int8 *value) const
{
	return FindScalar<int8, B_INT8_TYPE>(name, 0, value);
}

status_t 
FMessage::FindInt8(const char *name, int32 index, int8 *value) const
{
	return FindScalar<int8, B_INT8_TYPE>(name, index, value);
}

status_t 
FMessage::AddInt32(const char *name, int32 value)
{
	AddScalar<int32, B_INT32_TYPE>(name, value);

	return B_OK;
}


status_t 
FMessage::FindInt32(const char *name, int32 *value) const
{
	return FindScalar<int32, B_INT32_TYPE>(name, 0, value);
}

status_t 
FMessage::FindInt32(const char *name, int32 index, int32 *value) const
{
	return FindScalar<int32, B_INT32_TYPE>(name, index, value);
}

status_t 
FMessage::AddInt64(const char *name, int64 value)
{
	AddScalar<int64, B_INT64_TYPE>(name, value);

	return B_OK;
}


status_t 
FMessage::FindInt64(const char *name, int64 *value) const
{
	return FindScalar<int64, B_INT64_TYPE>(name, 0, value);
}

status_t 
FMessage::FindInt64(const char *name, int32 index, int64 *value) const
{
	return FindScalar<int64, B_INT64_TYPE>(name, index, value);
}

status_t 
FMessage::AddPointer(const char *name, const void *value)
{
	AddScalar<const void *, B_POINTER_TYPE>(name, value);
	return B_OK;
}

status_t 
FMessage::FindPointer(const char *name, void **value) const
{
	return FindScalar<void *, B_POINTER_TYPE>(name, 0, value);
}

status_t 
FMessage::FindPointer(const char *name, int32 index, void **value) const
{
	return FindScalar<void *, B_POINTER_TYPE>(name, index, value);
}

status_t 
FMessage::AddString(const char *name, const char *string)
{
	return AddData(name, B_STRING_TYPE, string, strlen(string) + 1);
}

status_t 
FMessage::FindString(const char *name, const char **string) const
{
	int32 length;
	return FindData(name, B_STRING_TYPE, (const void **)string, &length);
}

status_t 
FMessage::FindString(const char *name, int32 index, const char **string) const
{
	int32 length;
	return FindData(name, B_STRING_TYPE, index, (const void **)string, &length);
}

status_t 
FMessage::AddRef(const char *name, const EntryRef *ref)
{
	return AddData(name, B_REF_TYPE, (const void **)ref->Path(), strlen(ref->Path()) + 1);
}

status_t 
FMessage::FindRef(const char *name, EntryRef *ref) const
{
	return FindRef(name, 0, ref);
}

status_t 
FMessage::FindRef(const char *name, int32 index, EntryRef *ref) const
{
	int32 length;
	const char *path;
	
	status_t result = FindData(name, B_REF_TYPE, index, (const void **)&path, &length);
	if (result == B_OK) 
		ref->SetTo(path);

	return result;
}

bool 
FMessage::HasBool(const char *name)
{
	return HasData(name, B_BOOL_TYPE);
}

bool 
FMessage::HasInt8(const char *name)
{
	return HasData(name, B_INT8_TYPE);
}

bool 
FMessage::HasInt16(const char *name)
{
	return HasData(name, B_INT16_TYPE);
}

bool 
FMessage::HasInt32(const char *name)
{
	return HasData(name, B_INT32_TYPE);
}

bool 
FMessage::HasInt64(const char *name)
{
	return HasData(name, B_INT64_TYPE);
}

bool 
FMessage::HasString(const char *name)
{
	return HasData(name, B_STRING_TYPE);
}

bool 
FMessage::HasRef(const char *name) const
{
	return HasData(name, B_REF_TYPE);
}

bool 
FMessage::HasPointer(const char *name)
{
	return HasData(name, B_POINTER_TYPE);
}

bool 
FMessage::HasPoint(const char *name)
{
	return HasData(name, B_POINT_TYPE);
}


bool 
FMessage::WasDropped() const
{
	if (!fDetails)
		return false;
	return fDetails->WasDropped();
}

void 
FMessage::SetWasDropped()
{
	if (!fDetails)
		fDetails = new FMessageDetails;
	
	fDetails->SetWasDropped();
}

FPoint 
FMessage::DropPoint() const
{
	if (!fDetails)
		return FPoint(0, 0);
	return fDetails->DropPoint();
}

void 
FMessage::SetDropPoint(FPoint point)
{
	if (!fDetails)
		fDetails = new FMessageDetails;
	
	fDetails->SetDropPoint(point);
}

static void
PrintHexByte(String &stream, uint32 byte)
{
	stream << "0123456789abcdef"[byte / 0x10]
		<< "0123456789abcdef"[byte & 0xf];
}

static void
PrintAsciiByte(String &stream, uint32 byte)
{
	if (!isprint(byte))
		stream << '.';
	else
		stream << (char) byte;
}

static void
PrintIntAsChars(String &stream, uint32 number)
{
	stream << " \'";
	PrintAsciiByte(stream,(number / 0x1000000) & 0xff);
	PrintAsciiByte(stream,(number / 0x10000) & 0xff);
	PrintAsciiByte(stream,(number / 0x100) & 0xff);
	PrintAsciiByte(stream, number & 0xff);
	stream << "\'";
}

template<class T>
static String &
PrintScalar(String &stream, T number)
{
	stream << (int64)number << " ";
	stream << "0x";
	
	T tmp = 0;
	for (uint i = 0; i < sizeof(T); i++) {
		tmp <<= 8;
		tmp |= (number & 0xff);
		number >>= 8;
	}
	
	T tmp2 = tmp;
	for (uint i = 0; i < sizeof(T); i++) {
		PrintHexByte(stream, tmp & 0xff);
		tmp >>= 8;
	}

	stream << " \'";
	for (uint i = 0; i < sizeof(T); i++) {
		PrintAsciiByte(stream, tmp2 & 0xff);
		tmp2 >>= 8;
	}
	stream << "\'";

	return stream;
}

void 
FMessageAtom::PrintToStream(String &stream)
{
	stream << "\"" << fName << "\": ";
	PrintIntAsChars(stream, fType);
	stream << " size:" << (int32)fSize << " ";
	
	switch (fType) {
		case B_BOOL_TYPE:
			stream << "bool " << (*(bool *)fData ? "true" : "false");
			break;
		
		case B_INT8_TYPE:
			PrintScalar(stream, *(int8 *)fData);
			break;

		case B_INT16_TYPE:
			PrintScalar(stream, *(int16 *)fData);
			break;

		case B_INT32_TYPE:
			PrintScalar(stream, *(int32 *)fData);
			break;

		case B_INT64_TYPE:
			PrintScalar(stream, *(int64 *)fData);
			break;

		case B_RAW_TYPE:
			stream << "raw data ...";
			break;

		case B_REF_TYPE:
			stream << "ref:" << fData;
			break;

		case B_POINTER_TYPE:
			{
				void *tmp = *(void **)fData;
				if (sizeof(void *) == 4)
					PrintScalar(stream, (int32)tmp);
				else 
					PrintScalar(stream, reinterpret_cast<int64>(tmp));
				break;
			}
			
		case B_STRING_TYPE:
			stream << fData;
			break;

		case B_POINT_TYPE:
		case B_RECT_TYPE:
		default:
			break;
	}
	stream << "\n";
}

void 
FMessageDetails::PrintToStream(String &stream)
{
	if (fWasDropped) 
		stream << "dropped at: " << fDropPoint.x << " " << fDropPoint.y << "\n";
	
	// gcc is unhappy here unless we spell out String & as the template parameter
	EachListItem<FMessageAtom, String &>(&fData, &FMessageAtom::PrintToStream, stream);
}


String &
FMessage::PrintToStream(String &stream) const
{
	stream << "Message:\n";
	stream << "what: ";
	PrintScalar<uint32>(stream, what);
	stream << "\n";
	if (fDetails)
		fDetails->PrintToStream(stream);
	return stream;
}


void 
FMessage::PrintToStream() const
{
	String stream;
	PrintToStream(stream);
	printf(stream.CStr());
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
