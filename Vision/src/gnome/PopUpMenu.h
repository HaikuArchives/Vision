// BPopUpMenu-like class
//
// see License at the end of file

#ifndef FPOP_UP_MENU__
#define FPOP_UP_MENU__

#include "PlatformDefines.h"

#include "Point.h"
#include "Menu.h"

class FFont;
class FMenuItem;

class FPopUpMenu : public FMenu {
public:
	FPopUpMenu(const char *, bool = true, bool = true);
	FMenuItem *Go(FPoint, bool = false, bool = false, bool async = false);

protected:
	virtual void Invoke(FMenuItem *);

private:
	static void PopUpDone(GtkMenuShell *, void *);

	FMenuItem *fInvokedItem;
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
