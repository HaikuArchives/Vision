// spawn_thread and related calls
//
// see License at the end of file

#ifndef FTHREAD__
#define FTHREAD__

#include "PlatformDefines.h"
#include <Debug.h>

const uint32 B_DISPLAY_PRIORITY = 100;
const uint32 B_NORMAL_PRIORITY = 1000;

thread_id spawn_thread(status_t (*)(void *), const char *, uint32, void *);
status_t resume_thread(thread_id);
status_t wait_for_thread(thread_id, status_t *);


inline thread_id find_thread(const char *)
	{ ASSERT(!"implement me");  return 0; }

inline status_t kill_thread(thread_id)
	{ ASSERT(!"implement me");  return B_ERROR; }
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
