// ASSERT and other debugging macros
//
// see License at the end of file

#ifndef DEBUG__
#define DEBUG__

#include "PlatformDefines.h"

#include <glib.h>

#if DEBUG
#define ASSERT(x) g_assert(x) 
#define DEBUG_ONLY(x) x
#define PRINT(x) g_print x
#define TRESPASS() g_assert(!"should not be here");
#define TRACE() g_print("Trace: %s:%d %s\n", __FILE__, __LINE__, __FUNCTION__);
#else
#define ASSERT(x)
#define DEBUG_ONLY(x)
#define PRINT(x)
#define TRESPASS()
#define TRACE()
#endif

#define debugger(x) g_assert(!x)

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
