#include "PlatformDefines.h"

#include "image.h"

#include "Debug.h"

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>

status_t 
load_add_on(const char *path, image_id &result)
{
	result = dlopen(path, RTLD_NOW);
	if (!result) {
		printf("error loading image %s\n", dlerror());
		return B_ERROR;
	}
	
	return B_OK;
}

status_t 
unload_add_on(image_id image)
{
	return dlclose(image);
}

status_t 
get_image_symbol (image_id image, const char *symbol, int32, void **result)
{
	*result = dlsym(image, symbol);
	if (!*result) {
		printf("error looking up symbol %s %s\n", symbol,  dlerror());
		return B_ERROR;
	}

	return B_OK;
}

thread_id 
load_image(int, const char **, const char **)
{
	ASSERT(!"not implemented");
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
