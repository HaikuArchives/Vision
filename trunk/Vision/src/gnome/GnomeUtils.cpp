#include "PlatformDefines.h"

#include <sys/time.h>
#include <gdk/gdk.h>
#include <unistd.h>

void 
snooze(bigtime_t time)
{
	usleep(time);
}

bigtime_t 
system_time()
{
	struct timeval timeOfDay;
	gettimeofday (&timeOfDay, NULL);
	return (bigtime_t)timeOfDay.tv_usec + (((bigtime_t)timeOfDay.tv_sec) * 1000000LL);
}

void 
get_click_speed(bigtime_t *result)
{ 
	*result = 350000; 
}

uint32 
BeOSToGnomeModifiers(uint32 beosModifiers)
{
	uint32 result = 0;

	if (beosModifiers & B_SHIFT_KEY)
		result |= GDK_SHIFT_MASK;
	if (beosModifiers & B_CONTROL_KEY)
		result |= GDK_CONTROL_MASK;
	if (beosModifiers & B_COMMAND_KEY)
		result |= GDK_MOD1_MASK;
	if (beosModifiers & B_OPTION_KEY)
		result |= GDK_MOD4_MASK;
	return result;
}

uint32 
GnomeToBeOSModifiers(uint32 gnomeModifiers)
{
	uint32 result = 0;

	if (gnomeModifiers & GDK_SHIFT_MASK)
		result |= B_SHIFT_KEY;
	if (gnomeModifiers & GDK_CONTROL_MASK)
		result |= B_CONTROL_KEY;
	if (gnomeModifiers & GDK_MOD1_MASK)
		result |= B_COMMAND_KEY;
	if (gnomeModifiers & GDK_MOD4_MASK)
		result |= B_OPTION_KEY;

	return result;
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
