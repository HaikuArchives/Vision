// BAlert-like class
//
// see License at the end of file

#ifndef FALERT__
#define FALERT__

#include "PlatformDefines.h"

typedef int32 alert_type;
typedef int32 button_width;

const int32 B_WIDTH_AS_USUAL = 0;
const int32 B_INFO_ALERT = 0;

typedef struct _GtkWidget GtkWidget;

class FAlert {
public:
	FAlert(const char *name, const char *title, const char *button1,
	 	const char *button2 = NULL, const char *button3 = NULL,
		button_width = 0, alert_type = 0);

	void SetShortcut(int32 index, int32 shortcutKey);

	int32 Go();

private:
	GtkWidget *fDialog;
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
