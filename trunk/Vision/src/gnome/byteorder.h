// byte swapping calls
//
// see License at the end of file


#ifndef BYTEORDER__
#define BYTEORDER__

#include "PlatformDefines.h"

#include <StdDefs.h>
#include <endian.h>

// ToDo:
// switch to use glib swapping calls

union ShortSwapper {
	int16 value;
	struct Bytes {
		char first;
		char second;
	} bytes;
};

inline int16
SwapInt16(int16 original)
{
	ShortSwapper source;
	ShortSwapper result;

	source.value = original;
	result.bytes.first = source.bytes.second;
	result.bytes.second = source.bytes.first;

	return result.value;
}

union WordSwapper {
	int32 word;
	struct Bytes {
		char first;
		char second;
		char third;
		char fourth;
	} bytes;
};

inline int32
SwapInt32(int32 original)
{
	WordSwapper source;
	WordSwapper result;

	source.word = original;
	result.bytes.first = source.bytes.fourth;
	result.bytes.second = source.bytes.third;
	result.bytes.third = source.bytes.second;
	result.bytes.fourth = source.bytes.first;

	return result.word;
}

#if __BYTE_ORDER == __LITTLE_ENDIAN

#define B_BENDIAN_TO_HOST_FLOAT(x) (float)(SwapInt32((int32)x))
#define B_BENDIAN_TO_HOST_INT32(x) (int32)(SwapInt32(x))
#define B_BENDIAN_TO_HOST_INT16(x) (int16)(SwapInt16(x))

#define B_HOST_TO_BENDIAN_FLOAT(x) (float)(SwapInt32((int32)x))
#define B_HOST_TO_BENDIAN_INT32(x) (int32)(SwapInt32(x))
#define B_HOST_TO_BENDIAN_INT16(x) (int16)(SwapInt16(x))

#define B_LENDIAN_TO_HOST_FLOAT(x) (x)
#define B_LENDIAN_TO_HOST_INT32(x) (x)
#define B_LENDIAN_TO_HOST_INT16(x) (x)

#define B_HOST_TO_LENDIAN_FLOAT(x) (x)
#define B_HOST_TO_LENDIAN_INT32(x) (x)
#define B_HOST_TO_LENDIAN_INT16(x) (x)

#else

#error "are you sure this is big endian?"

#define B_BENDIAN_TO_HOST_FLOAT(x) (x)
#define B_BENDIAN_TO_HOST_INT32(x) (x)
#define B_BENDIAN_TO_HOST_INT16(x) (x)

#define B_LENDIAN_TO_HOST_FLOAT(x) (float)(SwapInt32((int32)x))
#define B_LENDIAN_TO_HOST_INT32(x) (int32)(SwapInt32(x))
#define B_LENDIAN_TO_HOST_INT16(x) (int32)(SwapInt16(x))

//...

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
