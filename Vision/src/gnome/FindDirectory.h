// find_directory
//
// see License at the end of file

#ifndef FFIND_DIRECTORY_H__
#define FFIND_DIRECTORY_H__

#include "PlatformDefines.h"

#include "CString.h"

class FPath;

enum {
	B_USER_DIRECTORY = 1UL,
	B_USER_SETTINGS_DIRECTORY = 2UL,
	B_COMMON_TEMP_DIRECTORY = 3UL,
	B_USER_ADDONS_DIRECTORY = 4UL
};


status_t find_directory(uint32, FPath *, bool = true);

class FindDirectory {
public:
	FindDirectory();
	static status_t Find(uint32, FPath *, bool = true);

private:
	static status_t MakeDirectory(const char *);
	void SubstituteHome(String &) const;

	String fHome;
	static FindDirectory findDirectory;
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
