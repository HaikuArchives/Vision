// spawn_thread and related calls
//
// see License at the end of file

#include "Debug.h"
#include "Thread.h"

#include <stdio.h>
#include <pthread.h>
#include <malloc.h>

struct ThreadMainGlueParams {
	status_t (*threadEntry)(void *);
	void *params;
};

static void *
ThreadMainGlue(void *data)
{
	ThreadMainGlueParams *params = (ThreadMainGlueParams *)data;
	(params->threadEntry)(params->params);
	free(params);
	return NULL;
}

thread_id
spawn_thread(status_t (* threadEntry)(void *), const char *, uint32, void *threadParams)
{
	pthread_attr_t thread_attr;
	pthread_t thread;
	ThreadMainGlueParams *params;
	int err;

	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate (&thread_attr, PTHREAD_CREATE_DETACHED);
	params = (ThreadMainGlueParams *)malloc(sizeof(ThreadMainGlueParams));
	params->threadEntry = threadEntry;
	params->params = threadParams;
	err = pthread_create (&thread, &thread_attr, ThreadMainGlue, params);
	//err = pthread_create (&thread, NULL, ThreadMainGlue, params);
	pthread_attr_destroy(&thread_attr);
	
	if (err != 0)
		return B_ERROR;
	
	return (thread_id)thread;
}

status_t
resume_thread(thread_id)
{
	return B_OK;
}

status_t
wait_for_thread(thread_id, status_t *)
{
	return B_OK;
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
