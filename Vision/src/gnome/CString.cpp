// String.
//
// see License at the end of file

#include "PlatformDefines.h"

#ifndef OSX_BRINGUP
#include <Font.h>
#include <View.h>
#endif

#ifndef GTK_BRINGUP
#ifndef OSX_BRINGUP
#include <InterfaceDefs.h>
#endif
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "CString.h"


String::String()
	:	data(0),
		length(0)
{
#if DEBUG
	usingAsCString = false;
#endif
}


String::String(const char *str)
{
	uint32 length = str ? strlen(str) : 0;
	Init(str, length);
}


String::String(const String &string)
{
	Init(string.CStr(), string.Length());
}


String::~String()
{
	delete[] data;
}


void 
String::Init(const char *str, int32 length)
{
	data = 0;
	this->length = 0;

	if (str)
		data = new char[length + 1];
	if (data)
		strcpy(data, str);

	this->length = length;
#if DEBUG
	usingAsCString = false;
#endif
}

void 
String::DoAssign(const char *str, int32 length)
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
	ASSERT((!str && !length) || (uint32)length == strlen(str));
#endif
	
	if (length) {
		if (length != (int32)this->length) {
			delete [] data;
			data = new char[length + 1];
		}

		strcpy(data, str);
		data[length] = '\0';
	} else {
		delete [] data;
		data = 0;
	}

	this->length = length;
}

String &
String::Assign(const char *str, int32 length)
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
	ASSERT((!str && !length) || (uint32)length <= strlen(str));
#endif

	if (!str)
		length = 0;

	if (length) {
		int32 tmpLength = strlen(str);
		if (tmpLength < length)
			length = tmpLength;

		if (length != (int32)this->length) {
			delete [] data;
			data = new char[length + 1];	
		}

		strncpy(data, str, length);
		data[length] = '\0';
	} else {
		delete [] data;
		data = 0;
	}

	this->length = length;
	return *this;
}

void 
String::DoAppend(const char *str, int32 length) 
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
	ASSERT((!str && !length) || (uint32)length <= strlen(str));
#endif

	if (!str || !length)
		return;

	char *newData = new char[Length() + length + 1];
	if (newData) {
		if (data)
			strncpy(newData, data, Length());
		strncpy(newData + Length(), str, length);
		this->length += length;
		newData[Length()] = '\0';
		delete [] data;
		data = newData;
	}
}

void 
String::DoPrepend(const char *str, int32 length) 
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
	ASSERT((!str && !length) || (uint32)length <= strlen(str));
#endif

	if (!str || !length)
		return;

	char *newData = new char[Length() + length + 1];
	if (newData) {
		strncpy(newData, str, length);
		if (data)
			strncpy(newData + length, data, Length());
		this->length += length;
		newData[Length()] = '\0';
		delete [] data;
		data = newData;
	}
}

String &
String::operator=(const String &string)
{
	if (&string != this)
		// guard against self asignment
		DoAssign(string.CStr(), string.Length());

	return *this;
}

String &
String::operator=(const char *str)
{
	int32 length;
	if (str)
		length = strlen(str);
	else
		length = 0;

	DoAssign(str, length);
	return *this;
}

String &
String::operator=(char ch)
{
//	const char tmp[2] = {ch, '\0'};
	char tmp[2];
	tmp[0] = ch;
	tmp[1] = '\0';
	DoAssign(tmp, 1);
	return *this;
}

String &
String::Assign(const String &string, int32 length)
{
	if (&string != this) {
		if (string.Length() < length)
			length = string.Length();

		Assign(string.CStr(), length);
	}
	return *this;
}

String &
String::operator+=(const char *str)
{
	int32 length = str ? strlen(str) : 0;
	DoAppend(str, length);
	return *this;
}

String &
String::operator+=(char ch)
{
//	const char tmp[2] = {ch, '\0'};
	char tmp[2];
	tmp[0] = ch;
	tmp[1] = '\0';
	DoAppend(tmp, 1);
	return *this;
}

String &
String::Append(const String &string, int32 length)
{
	if (&string != this) {
		if (string.Length() < length)
			length = string.Length();
	
		DoAppend(string.CStr(), length);
	}
	return *this;
}

String &
String::Append(const char *str, int32 length)
{
	int32 tmpLength;
	if (str)
		tmpLength = strlen(str);
	else
		tmpLength = 0;

	if (tmpLength < length)
		length = tmpLength;

	DoAppend(str, length);
	return *this;
}

String &
String::Append(const char ch, int32 length)
{
	if (length) {
		int32 oldLength = Length();
		char *newData = new char[oldLength + length + 1];
		if (data)
			strncpy(newData, data, oldLength);
		this->length += length;
		newData[Length()] = '\0';
		delete [] data;
		data = newData;
		memset(data + oldLength, ch, length);
	}
	return *this;
}


String &
String::Prepend(const char *str)
{
	int32 length;
	if (str)
		length = strlen(str);
	else
		length = 0;

	DoPrepend(str, length);
	return *this;
}

String &
String::Prepend(const String &string)
{
	if (&string != this) 
		DoPrepend(string.CStr(), string.Length());

	return *this;
}

String &
String::Prepend(const char *str, int32 length)
{
	int32 tmpLength;
	if (str)
		tmpLength = strlen(str);
	else
		tmpLength = 0;

	if (tmpLength < length)
		length = tmpLength;

	DoPrepend(str, length);
	return *this;
}

String &
String::Prepend(const String &string, int32 length)
{
	if (&string != this) {
		if (string.Length() < length)
			length = string.Length();
	
		DoPrepend(string.CStr(), length);
	}
	return *this;
}

String &
String::Truncate(int32 newLength)
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
#endif

	if (newLength < Length()) {
		length = newLength;
		if (data)
			data[Length()] = '\0';
	}
	return *this;
}

bool 
String::operator==(const char *str) const
{
#ifndef OSX_BRINGUP
	ASSERT(!usingAsCString);
#endif

	if (!Length() || !str)
		// can't pass zero pointers to strcmp
		return (!str || *str == 0) == (Length() == 0);

	return strcmp(str, data) == 0;
}

int 
String::Compare(const String &string) const
{
	return strcmp(CStr(), string.CStr());
}

int 
String::Compare(const char *str) const
{
#ifndef OSX_BRINGUP
	ASSERT(str);
#endif
	return strcmp(CStr(), str);
}

int 
String::Compare(const String &string, int32 n) const
{
	return strncmp(CStr(), string.CStr(), n);
}

int 
String::Compare(const char *str, int32 n) const
{
#ifndef OSX_BRINGUP
	ASSERT(str);
#endif
	return strncmp(CStr(), str, n);
}

int 
String::CaseCompare(const String &string) const
{
	return strcasecmp(CStr(), string.CStr());
}

int 
String::CaseCompare(const char *str) const
{
#ifndef OSX_BRINGUP
	ASSERT(str);
#endif

	return strcasecmp(CStr(), str);
}

int 
String::CaseCompare(const String &string, int32 n) const
{
	return strncasecmp(CStr(), string.CStr(), n);
}

int 
String::CaseCompare(const char *str, int32 n) const
{
#ifndef OSX_BRINGUP
	ASSERT(str);
#endif

	return strncasecmp(CStr(), str, n);
}

int32 
String::FindFirst(char ch) const
{
	int32 length = Length();
	if (!length)
		return -1;

	for (int32 index = 0; index < length; index++)
		if (data[index] == ch)
			return index;

	return -1;
}

int32 
String::FindFirst(char ch, int32 fromOffset) const
{
	int32 length = Length();
	if (length <= fromOffset)
		return -1;

	if (fromOffset < 0)
		fromOffset = 0;
	for (int32 index = fromOffset; index < length; index++)
		if (data[index] == ch)
			return index;

	return -1;
}

int32 
String::FindFirst(const char *str) const
{
	if (!Length())
		return -1;

	const char *s1 = data;
	const char *p1 = str;
	char firstc, c1;
	
	if ((firstc = *p1++) == 0)
		return 0;

	while((c1 = *s1++) != 0)
		if (c1 == firstc) {
			const char *s2 = s1;
			const char *p2 = p1;
			char c2;

			while ((c1 = *s2++) == (c2 = *p2++) && c1)
				;
			
			if (!c2)
				return (s1 - 1) - data;
		}
	
	return -1;
}

int32 
String::FindFirst(const String &str) const
{
	return FindFirst(str.CStr());
}

int32 
String::FindLast(char ch) const
{
	int32 length = Length();
	if (!length)
		return -1;

	int32 result = -1;
	for (int32 index = 0; index < length; index++)
		if (data[index] == ch) 
			result = index;

	return result;
}	

String &
String::ReplaceAll(char ch1, char ch2, int32 fromOffset)
{
	int32 length = Length();
	for (int32 index = fromOffset; index < length; index++)
		if (data[index] == ch1) {
			data[index] = ch2;
		}

	return *this;
}

void 
String::DoOpenAtBy(int32 index, int32 by)
{
#ifndef OSX_BRINGUP
	ASSERT(by > 0);
	ASSERT(index < (int32)length);
#endif

	char *newData = new char[length + by + 1];

	if (index > 0)
		memcpy(newData, data, index - 1);

	if (length > (uint32)index)
		memcpy(&newData[index + by], &data[index], length - index);

	data = newData;
	length += by;
	data [length] = '\0';
}

void 
String::DoShrinkAtBy(int32 index, int32 by)
{
#ifndef OSX_BRINGUP
	ASSERT(by > 0);
	ASSERT(index < (int32)length);
#endif

	char *newData = new char[length - by + 1];

	if (index > 0)
		memcpy(data, newData, index - 1);

	if (length > (uint32)index + by)
		memcpy(&newData[index], &data[index + by], length - index - by);

	data = newData;
	length -= by;
	data [length] = '\0';
}

String &
String::ReplaceFirst(const char *pattern, const char *replacement)
{
	if (pattern) {
		int32 offset = FindFirst(pattern);
		if (offset >= 0) {
			int32 patternLength = strlen(pattern);
			int32 replaceLength = 0;
			if (replacement)
				replaceLength = strlen(replacement);
			
			if (patternLength < replaceLength) 
				DoOpenAtBy(offset, replaceLength - patternLength);
			else if (patternLength > replaceLength)
				DoShrinkAtBy(offset, patternLength - replaceLength);

			if (replacement) 
				memcpy(data + offset, replacement, replaceLength);
		}
	}
	return *this;
}

char *
String::UseAsCString(int32 maxLength)
{
	if (maxLength > Length()) {
		char *newData = new char[maxLength + 1];
		if (newData) {
			newData[0] = '\0';
			if (data)
				strncpy(newData, data, Length());
			delete [] data;
			data = newData;
		} else
			return 0;
	}
#if DEBUG
	usingAsCString = true;
#endif
	return data;
}

String &
String::DoneUsingAsCString(int32 length)
{
#ifndef OSX_BRINGUP
	ASSERT(usingAsCString);
#endif

	if (length >=0) {
		this->length = length;
		data[length] = '\0';
	} else
		this->length = strlen(data);
#if DEBUG
	usingAsCString = false;
#endif
	return *this;
}

String &
String::Adopt(String &stringToAdopt)
{
	if (&stringToAdopt == this)
		return *this;

	delete[] data;
	data = stringToAdopt.data;
	length = stringToAdopt.Length();
	stringToAdopt.data = 0;
	stringToAdopt.length = 0;
	return *this;
}

String &
String::Adopt(char *adoptThis)
{
	if (adoptThis == data)
		return *this;
	
	delete [] data;
	data = adoptThis;
	if (adoptThis)
		length = strlen(adoptThis);
	else
		length = 0;
	
	return *this;
}


String &
String::ToLower()
{
	for (int32 index = 0; index < Length(); index++)
		data[index] = _tolower(data[index]);

	return *this;
}

String &
String::ToUpper()
{
	for (int32 index = 0; index < Length(); index++)
		data[index] = toupper(data[index]);

	return *this;
}

String &
String::operator<<(const char *str)
{
	*this += str;

	return *this;
}

String &
String::operator<<(const String &string)
{
	*this += string;

	return *this;
}

String &
String::operator<<(char ch)
{
	*this += ch;

	return *this;
}

String &
String::operator<<(int32 num)
{
	// for now
	char buffer[64];
	sprintf(buffer, "%ld", num);
	*this += buffer;

	return *this;
}

String &
String::operator<<(uint32 num)
{
	// for now
	char buffer[64];
	sprintf(buffer, "%lu", num);
	*this += buffer;

	return *this;
}

String &
String::operator<<(int64 num)
{
	// for now
	char buffer[64];
	sprintf(buffer, "%Ld", num);
	*this += buffer;

	return *this;
}

String &
String::operator<<(uint64 num)
{
	// for now
	char buffer[64];
	sprintf(buffer, "%Lu", num);
	*this += buffer;

	return *this;
}

String &
String::operator<<(float num)
{
	// for now
	char buffer[64];
	sprintf(buffer, "%.2f", num);
	*this += buffer;

	return *this;
}

inline bool
IsMeta(char ch)
{
    return ch == '[' || ch == ']' || ch == '(' || ch == ')' || 
		ch == '*' || ch == '"' || ch == '\'' ||
        ch == '?' || ch == '^' || ch == '\\' || ch == ' ' || ch == '\t' ||
        ch == '\n' || ch == '\r';
}

String &
String::EscapeMetachars()
{
	String tmp(*this);
	return AssignEscapingMetachars(tmp.CStr());
}

String &
String::DeEscapeMetachars()
{
	String tmp(*this);
	return AssignDeEscapingMetachars(tmp.CStr());
}


String & 
String::AssignEscapingMetachars(const char *str)
{
	(*this) = "";
	
	const char *from = str;
	for (;;) {
		int32 count = 0;
		while(from[count] && !IsMeta(from[count]) )
			count++;
		Append(from, count);
		if (!from[count])
			break;
		char tmp[3];
		tmp[0] = '\\';
		tmp[1] = from[count];
		tmp[2] = '\0';
		(*this)+= tmp;
		from += count + 1;
	}
	return *this;
}

String &
String::AssignDeEscapingMetachars(const char *str)
{
	(*this) = "";
	
	for (;*str; str++)
		if (*str != '\\')
			(*this) += *str;
	
	return *this;
}

String &
String::AssignEscapingHTML(const char *str)
{
	(*this) = "";
	
	const char *from = str;
	for (;;) {
		int32 count = 0;
		while (from[count]
			&& from[count] != '<'
			&& from[count] != '>'
			&& from[count] != '&'
			&& from[count] != '"')
			count++;

		Append(from, count);
		if (!from[count])
			break;

		switch (from[count]) {
			case '<':
				(*this)+= "&lt;";
				break;
			case '>':
				(*this)+= "&gt;";
				break;
			case '&':
				(*this)+= "&amp;";
				break;
			case '"':
				(*this)+= "&quot;";
				break;
				
#ifndef OSX_BRINGUP
			default:
				TRESPASS();
#endif
		}

		from += count + 1;
	}
	return *this;
}

String &
String::EscapeHTML()
{
	String tmp(*this);
	return AssignEscapingHTML(tmp.CStr());
}

#ifndef OSX_BRINGUP
String &
String::TruncToWidth(const BFont *font, float maxWidth, uint32 mode, bool *changed)
{
#ifndef GTK_BRINGUP
	bool changedResult = false;
	if (length) {
		char *tmp = new char[length + 3];
		if (tmp) {
			font->GetTruncatedStrings(const_cast<const char **>(&data), 1, mode,
				maxWidth, &tmp);
			int32 tmpLength = strlen(tmp);
			if (strlen(tmp) != (ulong)Length()) {
				changedResult = true;
				delete [] data;
				data = tmp;
				length = tmpLength;
			} else
				delete [] tmp;
		}
	}
	
	if (changed)
		*changed = changedResult;
#else
	font->Truncate(*this, maxWidth, mode, changed);
#endif
	return *this;
}

String &
String::TruncToWidth(const BView *view, float maxWidth, uint32 mode, bool *changed)
{
	BFont font;
	const_cast<BView *>(view)->GetFont(&font);
	return TruncToWidth(&font, maxWidth, mode, changed);
}

float 
String::Width(BView *view) const
{
	return view->StringWidth(CStr());
}

float 
String::Width(BFont *font) const
{
	return font->StringWidth(CStr());
}
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
