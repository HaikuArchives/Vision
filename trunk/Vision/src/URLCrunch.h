
#ifndef URLCRUNCH_H_
#define URLCRUNCH_H_

#include <String.h>

class URLCrunch
{
	BString			buffer;
	int32				current_pos;

	public:

						URLCrunch (const char *, int32);
						~URLCrunch (void);
	int32				Crunch (BString *);
};

#endif
