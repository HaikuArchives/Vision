// Fakeout defines
// This header needs to be included as first by every file.
//
// see License at the end of file

#ifndef PLATFORM_DEFINES__
#define PLATFORM_DEFINES__

#define GTK_BRINGUP
#define GTK_ONLY 1

#define BEOS_ONLY_ARGS(x) 
#define GTK_ONLY_ARGS(x) x

// map the BeOS fakeout classes into the B... namespace
#define String BString
#define FAlert BAlert
#define FRect BRect
#define FPoint BPoint
#define FView BView
#define FWindow BWindow
#define FList BList
#define FListItem BListItem
#define FStringItem BStringItem
#define FListView BListView
#define FFont BFont
#define FRegion BRegion
#define FScreen BScreen
#define FBitmap BBitmap
#define FApplication BApplication
#define FDirectory BDirectory
#define FEntry BEntry
#define FFile BFile
#define FFilePanel BFilePanel
#define FNode BNode
#define FPath BPath
#define entry_ref EntryRef
#define app_info AppInfo
#define FMenuBar BMenuBar
#define FMenu BMenu
#define FPopUpMenu BPopUpMenu
#define FMenuItem BMenuItem
#define FMenuField BMenuField
#define FSeparatorItem BSeparatorItem
#define FMessage BMessage
#define FMessageFilter BMessageFilter
#define FMessenger BMessenger
#define FHandler BHandler
#define FLooper BLooper
#define FBox BBox
#define FCheckBox BCheckBox
#define FButton BButton
#define FTextControl BTextControl
#define FTextView BTextView
#define FScrollView BScrollView
#define FScrollBar BScrollBar
#define FLocker BLocker
#define FSymLink BSymLink
#define FStringView BStringView
#define FStatusBar BStatusBar
#define FDataIO BDataIO
#define FPositionIO BPositionIO
#define FMallocIOI BMalocIO
#define BMemoryIO FMemoryIO

#include "StdDefs.h"
#include "Point.h"
#include "Rect.h"

#define B_UTF8_OPEN_QUOTE "\""
#define B_UTF8_CLOSE_QUOTE "\""

#define B_UTF8_ELLIPSIS "..."

void snooze(bigtime_t);
bigtime_t system_time();
void get_click_speed(bigtime_t *);
uint32 modifiers();

uint32 BeOSToGnomeModifiers(uint32);
uint32 GnomeToBeOSModifiers(uint32);

enum {
	B_ENTERED_VIEW,
	B_INSIDE_VIEW,
	B_EXITED_VIEW
};

enum {
	B_PRIMARY_MOUSE_BUTTON = 1,
	B_SECONDARY_MOUSE_BUTTON = 2,
	B_TERTIARY_MOUSE_BUTTON = 4
};

const uint32 B_WILL_DRAW = 1;
const uint32 B_PULSE_NEEDED = 2;
const uint32 B_FRAME_EVENTS = 4;
const uint32 B_NAVIGABLE = 8;
const uint32 B_NAVIGABLE_JUMP = 16;

const uchar B_BACKSPACE = 0x08;
const uchar B_RETURN = 0x0a;
const uchar B_ENTER = 0x0a;
const uchar B_SPACE = 0x20;
const uchar B_TAB = 0x09;
const uchar B_ESCAPE = 0x1b;

const uchar B_LEFT_ARROW = 0x1c;
const uchar B_RIGHT_ARROW = 0x1d;
const uchar B_UP_ARROW = 0x1e;
const uchar B_DOWN_ARROW = 0x1f;

const uchar B_INSERT = 0x05;
const uchar B_DELETE = 0x7f;
const uchar B_HOME = 0x01;
const uchar B_END = 0x04;
const uchar B_PAGE_UP = 0x0b;
const uchar B_PAGE_DOWN = 0x0c;

const uchar B_FUNCTION_KEY = 0x10;

const uint32 B_SHIFT_KEY = 0x1;
const uint32 B_COMMAND_KEY = 0x2;
const uint32 B_CONTROL_KEY = 0x4;
const uint32 B_CAPS_LOCK = 0x8;
const uint32 B_SCROLL_LOCK = 0x10;
const uint32 B_NUM_LOCK = 0x20;
const uint32 B_OPTION_KEY = 0x40;
const uint32 B_MENU_KEY = 0x80;
const uint32 B_LEFT_SHIFT_KEY = 0x100;
const uint32 B_RIGHT_SHIFT_KEY = 0x200;
const uint32 B_LEFT_COMMAND_KEY = 0x400;
const uint32 B_RIGHT_COMMAND_KEY = 0x800;
const uint32 B_LEFT_CONTROL_KEY = 0x1000;
const uint32 B_RIGHT_CONTROL_KEY = 0x2000;
const uint32 B_LEFT_OPTION_KEY = 0x4000;
const uint32 B_RIGHT_OPTION_KEY = 0x800;

const uint32 B_F1_KEY = 0x02;
const uint32 B_F2_KEY = 0x03;
const uint32 B_F3_KEY = 0x04;
const uint32 B_F4_KEY = 0x05;
const uint32 B_F5_KEY = 0x06;
const uint32 B_F6_KEY = 0x07;
const uint32 B_F7_KEY = 0x08;
const uint32 B_F8_KEY = 0x09;
const uint32 B_F9_KEY = 0x0a;
const uint32 B_F10_KEY = 0x0b;
const uint32 B_F11_KEY = 0x0c;
const uint32 B_F12_KEY = 0x0d;
const uint32 B_PRINT_KEY = 0x0e;
const uint32 B_SCROLL_KEY = 0x0f;
const uint32 B_PAUSE_KEY = 0x10;

		
const uint32 B_QUIT_REQUESTED = '_QRQ';
const uint32 B_SAVE_REQUESTED = '_SRQ';
const uint32 B_KEY_DOWN = '_KYD';
const uint32 B_MOUSE_DOWN = '_MDN';
const uint32 B_MOUSE_UP = '_MUP';
const uint32 B_REFS_RECEIVED = '_RRC';
const uint32 B_PULSE = '_PUL';

const uint32 B_BOOL_TYPE = 'BOOL';
const uint32 B_INT8_TYPE = 'BYTE';
const uint32 B_INT16_TYPE = 'SHRT';
const uint32 B_INT32_TYPE = 'LONG';
const uint32 B_INT64_TYPE = 'LLNG';
const uint32 B_RAW_TYPE = 'RAWT';
const uint32 B_REF_TYPE = 'RREF';
const uint32 B_POINTER_TYPE = 'PNTR';
const uint32 B_POINT_TYPE = 'BPNT';
const uint32 B_RECT_TYPE = 'RECT';
const uint32 B_MIME_TYPE = 'MIME';
const uint32 B_STRING_TYPE = 'CSTR';

#ifdef GTK_BRINGUP

#include "Errors.h"

enum DocumentLanguageType {
	kUnknown,
	kShell,				// every file with no suffix
	kCC,
	kCH,
	kCCPlus,
	kCHPlus,
	kJava,
	kHTML,
	kPerl,
	kYacc,
	kLex,
	kAsm,
	kPascal,
	kModulaDef,
	kModulaOberonImpl,
	kFortran,
	kRez,
	kCLikeAsm,			// asm with C-style comments, ifdefs
	kMakefile,			// makefile, Makefile, make*, etc.
	kPython,
	kVHDL,
	kTCL,
	kPlainText,
	kPHP,
	kLanguageLast = kPHP
};

#endif

const int32 kXLALT =  64;
const int32 kXLCTL =  37;
const int32 kXRCTL = 109;
const int32 kXRALT = 113;
const int32 kXLWIN = 115;
const int32 kXRWIN = 116;
const int32 kXMENU = 117;
const int32 kXRTSH =  62;
const int32 kXLFSH =  50;

const int32 B_LOW_PRIORITY = 100;

#define B_INFINITE_TIMEOUT	(9223372036854775807LL)
#define MAXHOSTNAMELEN 		64

#define __HONOR_STD 1
#include <exception>
#include <new>
#define __THROW_BAD_ALLOC throw "badly configured stl"
// need this cruft because of badly configured stl that insists on 
// streaming to crerr instead of throwing bad alloc.
// we don't want to link against libstdc++
// also bad_alloc doesn't seem to work for some dumb reason, I'll need
// to figure it out later.


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
