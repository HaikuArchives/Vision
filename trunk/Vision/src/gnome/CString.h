// String.
//
// see License at the end of file

// ToDo:
// Get in line with BString

#ifndef __STRING_CLASS__
#define __STRING_CLASS__

#include "PlatformDefines.h"

#ifndef GTK_BRINGUP
#include <SupportDefs.h>
#endif

#include "Debug.h"

#include <string.h>

class BFont;
class BView;

class String {
public:
	String();
	String(const char *);
	String(const String &);
	virtual ~String();


	const char *CStr() const;
		// C string conversion
	int32 Length() const;

	
	String &operator=(const char *);
	String &operator=(const String &);
	String &operator=(char);
	
	String &Assign(const String &, int32 length);
	String &Assign(const char *, int32 length);

	String &operator+=(const char *);
	String &operator+=(char);
	String &operator+=(const String &);

	String &Append(const String &, int32 length);
	String &Append(const char *, int32 length);
	String &Append(const char, int32 length);

	String &Prepend(const char *);
	String &Prepend(const String &);
	String &Prepend(const char *, int32);
	String &Prepend(const String &, int32);

	String &Truncate(int32 newLength);


	bool operator==(const String &) const;
	bool operator==(const char *) const;
	bool operator!=(const String &) const;
	bool operator!=(const char *) const;
	
	// strcmp-style comparing
	int Compare(const String &) const;
	int Compare(const char *) const;
	int Compare(const String &, int32 n) const;
	int Compare(const char *, int32 n) const;
	int CaseCompare(const String &) const;
	int CaseCompare(const char *) const;
	int CaseCompare(const String &, int32 n) const;
	int CaseCompare(const char *, int32 n) const;
	
	int32 FindFirst(char) const;
	int32 FindFirst(char, int32) const;
	int32 FindFirst(const char *) const;
	int32 FindFirst(const String &) const;

	int32 FindLast(char) const;

	String &ReplaceAll(char replaceThis, char withThis, int32 fromOffset = 0);
	String &ReplaceFirst(const char *patter, const char *replacement);

	// unchecked char access
	char operator[](int32 index) const;
	char &operator[](int32 index);

	// checked char access
	char At(int32 index) const;

	char *UseAsCString(int32 maxLength);
		// make room for characters to be added
		// using CStr(); returns the equivalent of CStr()
		// <maxLength> includes space for trailing zero
		// while used as cstring, it is illegal to call other
		// String routines that rely on data/length consistency
	String &DoneUsingAsCString(int32 length = -1);
		// finish using String as cstring, adjusting length
		// if no length passed in, strlen of internal data is
		// used
	String &Adopt(String &stringToConsume);
		// more efficient assignment, just takes the <stringToConsume>
		// and sets it to 0, <stringToConsumes> is empty after the call

	String &Adopt(char *adoptThis);
		// adopt this must be new char [] allocated

	String &ToLower();
	String &ToUpper();
	String &EscapeMetachars();
	String &DeEscapeMetachars();
	String &AssignEscapingMetachars(const char *);
	String &AssignDeEscapingMetachars(const char *);
	String &AssignEscapingHTML(const char *);
	String &EscapeHTML();
	
	String &TruncToWidth(const BFont *, float maxWidth, uint32 mode,
		bool *changed = 0);
	String &TruncToWidth(const BView *, float maxWidth, uint32 mode,
		bool *changed = 0);

	float Width(BView *) const;
	float Width(BFont *) const;
	
	// simple sprintf replacement calls
	String &operator<<(const char *);
	String &operator<<(const String &);
	String &operator<<(char);
	String &operator<<(uint32);
	String &operator<<(int32);
	String &operator<<(uint64);
	String &operator<<(int64);
	String &operator<<(float);
		// hardcodes %.2f formatting 

protected:
	void Init(const char *, int32);
	void DoAssign(const char *, int32);
	void DoAppend(const char *, int32);
	void DoPrepend(const char *, int32);
	void DoOpenAtBy(int32 index, int32 by);
	void DoShrinkAtBy(int32 index, int32 by);

	char *data;
	ulong length;
#if DEBUG
	bool usingAsCString;
#endif
};

inline int32 
String::Length() const
{
	ASSERT(!usingAsCString);
	return length;
}

inline const char *
String::CStr() const
{
	if (!data)
		return "";
	return data;
}

inline bool 
String::operator!=(const String &string) const
{
	return !operator==(string);
}

inline bool 
String::operator!=(const char *str) const
{
	return !operator==(str);
}

inline char 
String::operator[](int32 index) const
{
	return data[index];
}

inline char &
String::operator[](int32 index)
{
	return data[index];
}

inline char 
String::At(int32 index) const
{
	if (!length || index > Length())
		return 0;
	return data[index];
}

inline String &
String::operator+=(const String &string)
{
	DoAppend(string.CStr(), string.Length());
	return *this;
}

inline bool 
String::operator==(const String &string) const
{
	ASSERT(!usingAsCString);
	return strcmp(CStr(), string.CStr()) == 0;
}

#ifndef GTK_BRINGUP
inline char
_tolower(char ch)
{
	return (ch >='A' && ch <= 'Z') ? ch - 'A' + 'a' :  ch;
}
#endif

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
