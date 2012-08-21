/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Vision.
 *
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 *
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 *								 Ted Stodgell <kart@hal-pc.org>
 */

class VisionApp * vision_app;


/*
	-- #beos was here --
	<Brazilian> And then I says to the gnu, "Is that a horn on your head or
							are you just happy to see me?"
*/

#include <Alert.h>
#include <Resources.h>
#include <FindDirectory.h>
#include <Font.h>
#include <Locale.h>
#include <MenuItem.h>
#include <Mime.h>
#include <Path.h>
#include <Autolock.h>
#include <Roster.h>
#include <Beep.h>
#include <UTF8.h>

#include <algorithm>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "AboutWindow.h"
#include "Vision.h"
#include "ClientWindow.h"
#include "DCCConnect.h"
#include "DCCFileWindow.h"
#include "NetworkManager.h"
#include "NetworkWindow.h"
#include "SettingsFile.h"
#include "SetupWindow.h"
#include "PrefsWindow.h"
#include "Theme.h"
#include "URLCrunch.h"
#include "Utilities.h"
#include "WindowList.h"
// sound event name definitions
const char *kSoundEventNames[] = { "Vision Nick Notification", 0 };

const char *kAliasPathName = "Vision/Aliases";
const char *kTrackerSig = "application/x-vnd.Be-TRAK";

NetworkManager *network_manager = NULL;

// And so it begins....
int
main (void)
{
	// Seed it!
	srand (time (0));

	vision_app = new VisionApp;
	vision_app->Run();
	delete vision_app;

	return B_OK;
}


//////////////////////////////////////////////////////////////////////////////
/// Begin BApplication functions
//////////////////////////////////////////////////////////////////////////////


VisionApp::VisionApp (void)
	: BApplication ("application/x-vnd.Ink-Vision"),
			fAboutWin (NULL),
			fSetupWin (NULL),
			fClientWin (NULL),
			fPrefsWin (NULL),
			fNetWin (NULL),
			fDccFileWin (NULL),
			fVisionSettings (NULL),
			fWinThread (-1),
			fIdentSocket (-1),
			fActiveTheme (new Theme ("active theme", MAX_COLORS + 1, MAX_COLORS + 1, MAX_FONTS + 1))
{
	// some setup
	fSettingsLoaded = false;
	fDebugRecv = false;
	fDebugSend = false;
	fDebugSettings = false;
	fNumBench = false;
	fShuttingDown = false;
	fDebugShutdown = false;
	fDisableAutostart = false;
	fStartupTime = system_time();

	app_info info;
	if (GetAppInfo(&info) == B_OK) fAppRef = info.ref;

	URLCrunch::UpdateTagList();

}

VisionApp::~VisionApp (void)
{
	int32 i (0);
	for (; i < MAX_FONTS; i++)
		delete fClientFont[i];

	delete fActiveTheme;
}

int32
VisionApp::ThreadStates (void)
{
	thread_id team (Team());
	int32 cookie (0);
	thread_info info;

	BString buffer;
	int32 t_count (0);

	while (get_next_thread_info (team, &cookie, &info) == B_NO_ERROR)
	{
		// wake up any threads that're snoozing for their next reconnect run
		if (strstr(info.name, "s>") != NULL)
		{
			switch(info.state)
			{
				case B_THREAD_ASLEEP:
					suspend_thread(info.thread);
					// fall through
				case B_THREAD_SUSPENDED:
					resume_thread(info.thread);
					break;
				default:
					break;
			}
		}

		if (fDebugShutdown)
		{
			buffer += "thread: ";
			buffer << info.thread;
			buffer += " name:	";
			buffer += info.name;
			buffer += " state: ";

			switch ((int32)info.state)
			{
				case B_THREAD_RUNNING:
					buffer += "running\n";
					break;

				case B_THREAD_READY:
					buffer += "ready\n";
					break;

				case B_THREAD_RECEIVING:
					buffer += "receiving\n";
					break;

				case B_THREAD_ASLEEP:
					buffer += "asleep\n";
					break;

				case B_THREAD_SUSPENDED:
					buffer += "suspended\n";
					break;

				case B_THREAD_WAITING:
					buffer += "waiting\n";
					break;

				default:
					buffer += "???\n";
			}
		}
		++t_count;
	}

	if (fDebugShutdown && buffer.Length())
	{
		printf("%s\n", buffer.String());
#if 0
		BAlert *alert (new BAlert (
			"Too many threads",
			buffer.String(),
			t_count > 1 ? "Damn" : "Cool",
			0,
			0,
			B_WIDTH_AS_USUAL,
			t_count > 1 ? B_STOP_ALERT : B_INFO_ALERT));
			alert->Go();
#endif
	}

	return t_count;
}

void
VisionApp::InitDefaults (void)
{
	const rgb_color myBlack						 = {0,0,0, 255};
	const rgb_color myWhite						 = {255, 255, 255, 255};
	const rgb_color NOTICE_COLOR				= {10,90,170, 255};
	const rgb_color ACTION_COLOR				= {128,0,128, 255};
	const rgb_color QUIT_COLOR					= {180,10,10, 255};
	const rgb_color ERROR_COLOR				 = {210,5,5, 255};
	const rgb_color URL_COLOR					 = {5,5,150, 255};
	const rgb_color NICK_COLOR					= {10,10,190, 255};
	const rgb_color MYNICK_COLOR				= {200,10,20, 255};
	const rgb_color JOIN_COLOR					= {10,130,10, 255};
	const rgb_color KICK_COLOR					= {250,130,10, 255};
	const rgb_color WHOIS_COLOR				 = {10,30,170, 255};
	const rgb_color OP_COLOR						= {140,10,40, 255};
	const rgb_color VOICE_COLOR				 = {160, 20, 20, 255};
	const rgb_color CTCP_REQ_COLOR			= {10,10,180, 255};
	const rgb_color CTCP_RPY_COLOR			= {10,40,180, 255};
	const rgb_color IGNORE_COLOR				= {100, 100, 100, 255};
	const rgb_color INPUT_COLOR				 = {0, 0, 0, 255};
	const rgb_color INPUT_BG_COLOR			= {255, 255, 255, 255};
	const rgb_color WINLIST_BG_COLOR		= {238, 242, 242, 255};
	const rgb_color WINLIST_PAGE6_COLOR = {100, 100, 100, 255};
	const rgb_color WINLIST_SEL_COLOR	 = ui_color (B_PANEL_BACKGROUND_COLOR);
	const rgb_color WALLOPS_COLOR			 = {10,30,170, 255};
	const rgb_color NICK_DISPLAY				= {47, 47, 47, 255};
	const rgb_color MIRC_BLUE					 = { 0, 0, 127, 255};
	const rgb_color MIRC_GREEN					= { 0, 127, 0, 255};
	const rgb_color MIRC_RED						= { 127, 0, 0, 255};
	const rgb_color MIRC_BROWN					= { 224, 192, 128, 255};
	const rgb_color MIRC_PURPLE				 = { 127, 0, 127, 255};
	const rgb_color MIRC_ORANGE				 = { 192, 127, 0, 255};
	const rgb_color MIRC_YELLOW				 = { 255, 255, 0, 255};
	const rgb_color MIRC_LIME					 = { 0, 255, 0, 255};
	const rgb_color MIRC_TEAL					 = { 0, 127, 127, 255};
	const rgb_color MIRC_AQUA					 = { 0, 255, 255, 255};
	const rgb_color MIRC_LT_BLUE				= { 0, 0, 255, 255};
	const rgb_color MIRC_PINK					 = { 255, 127, 127, 255};
	const rgb_color MIRC_GREY					 = { 127, 127, 127, 255};
	const rgb_color MIRC_SILVER				 = { 192, 192, 192, 255};

	fColors[C_TEXT]											= myBlack;
	fColors[C_BACKGROUND]								= myWhite;
	fColors[C_NAMES]										 = myBlack;
	fColors[C_NAMES_BACKGROUND]					= myWhite;
	fColors[C_NAMES_SELECTION]					 = WINLIST_SEL_COLOR;
	fColors[C_URL]											 = URL_COLOR;
	fColors[C_SERVER]										= myBlack;
	fColors[C_NOTICE]										= NOTICE_COLOR;
	fColors[C_ACTION]										= ACTION_COLOR;
	fColors[C_QUIT]											= QUIT_COLOR;
	fColors[C_ERROR]										 = ERROR_COLOR;
	fColors[C_NICK]											= NICK_COLOR;
	fColors[C_MYNICK]										= MYNICK_COLOR;
	fColors[C_JOIN]											= JOIN_COLOR;
	fColors[C_KICK]											= KICK_COLOR;
	fColors[C_WHOIS]										 = WHOIS_COLOR;
	fColors[C_OP]												= OP_COLOR;
	fColors[C_HELPER]										= OP_COLOR;
	fColors[C_VOICE]										 = VOICE_COLOR;
	fColors[C_CTCP_REQ]									= CTCP_REQ_COLOR;
	fColors[C_CTCP_RPY]									= CTCP_RPY_COLOR;
	fColors[C_IGNORE]										= IGNORE_COLOR;
	fColors[C_INPUT]										 = INPUT_COLOR;
	fColors[C_INPUT_BACKGROUND]					= INPUT_BG_COLOR;
	fColors[C_WINLIST_BACKGROUND]				= WINLIST_BG_COLOR;
	fColors[C_WINLIST_NORMAL]						= myBlack;
	fColors[C_WINLIST_NEWS]							= JOIN_COLOR;
	fColors[C_WINLIST_NICK]							= QUIT_COLOR;
	fColors[C_WINLIST_SELECTION]				 = WINLIST_SEL_COLOR;
	fColors[C_WINLIST_PAGESIX]					 = WINLIST_PAGE6_COLOR;
	fColors[C_WALLOPS]									 = WALLOPS_COLOR;
	fColors[C_NICKDISPLAY]							 = NICK_DISPLAY;
	fColors[C_TIMESTAMP]								 = myBlack;
	fColors[C_TIMESTAMP_BACKGROUND]			= myWhite;
	fColors[C_SELECTION]								 = myBlack;
	fColors[C_MIRC_WHITE]								= myWhite;
	fColors[C_MIRC_BLACK]								= myBlack;
	fColors[C_MIRC_BLUE]								 = MIRC_BLUE;
	fColors[C_MIRC_GREEN]								= MIRC_GREEN;
	fColors[C_MIRC_RED]									= MIRC_RED;
	fColors[C_MIRC_BROWN]								= MIRC_BROWN;
	fColors[C_MIRC_PURPLE]							 = MIRC_PURPLE;
	fColors[C_MIRC_ORANGE]							 = MIRC_ORANGE;
	fColors[C_MIRC_YELLOW]							 = MIRC_YELLOW;
	fColors[C_MIRC_LIME]								 = MIRC_LIME;
	fColors[C_MIRC_TEAL]								 = MIRC_TEAL;
	fColors[C_MIRC_AQUA]								 = MIRC_AQUA;
	fColors[C_MIRC_LT_BLUE]							= MIRC_LT_BLUE;
	fColors[C_MIRC_PINK]								 = MIRC_PINK;
	fColors[C_MIRC_GREY]								 = MIRC_GREY;
	fColors[C_MIRC_SILVER]							 = MIRC_SILVER;
	fColors[C_NOTIFY_ON]								 = JOIN_COLOR;
	fColors[C_NOTIFY_OFF]								= myBlack;
	fColors[C_NOTIFYLIST_BACKGROUND]		 = WINLIST_BG_COLOR;
	fColors[C_NOTIFYLIST_SELECTION]			= WINLIST_SEL_COLOR;

	fClientFont[F_TEXT]		= new BFont (be_plain_font);
	fClientFont[F_SERVER]	= new BFont (be_plain_font);
	fClientFont[F_URL]		 = new BFont (be_plain_font);
	fClientFont[F_NAMES]	 = new BFont (be_plain_font);
	fClientFont[F_INPUT]	 = new BFont (be_plain_font);
	fClientFont[F_WINLIST] = new BFont (be_plain_font);
	fClientFont[F_LISTAGENT] = new BFont (be_plain_font);
	fClientFont[F_TIMESTAMP] = new BFont (be_plain_font);

	fEvents[E_JOIN]						= "*** $N ($I@$A) has joined the channel.";
	fEvents[E_PART]						= "*** $N has left the channel.";
	fEvents[E_NICK]						= "*** $N is now known as $n.";
	fEvents[E_QUIT]						= "*** $N ($I@$A) has quit IRC ($R)";
	fEvents[E_KICK]						= "*** $N has been kicked from $C by $n ($R)";
	fEvents[E_TOPIC]					 = "*** $C Topic changed by $N: $T";
	fEvents[E_SNOTICE]				 = "-$N- $R";
	fEvents[E_UNOTICE]				 = "-$N- $R";
	fEvents[E_NOTIFY_ON]			 = "*** $N has joined IRC.";
	fEvents[E_NOTIFY_OFF]			= "*** $N has left IRC.";

	fCommands[CMD_KICK]				= "Ouch!";
	fCommands[CMD_QUIT]				= "Vision[$V]: i've been blurred!";
	fCommands[CMD_IGNORE]			= "*** $N is now ignored ($i).";
	fCommands[CMD_UNIGNORE]		= "*** $N is no longer ignored.";
	fCommands[CMD_AWAY]				= "is idle: $R";
	fCommands[CMD_BACK]				= "has returned";
#ifdef __HAIKU__
	fCommands[CMD_UPTIME]			= "OS Uptime [Haiku]: $U";
#else
	fCommands[CMD_UPTIME]			= "OS Uptime [BeOS]: $U";
#endif
	uint32 i = 0;

	for( const char* eventName = kSoundEventNames[i]; eventName != NULL; i++,
		eventName = kSoundEventNames[i] )
		add_system_beep_event(eventName);
}

void
VisionApp::InitSettings (void)
{
	// initialize arrays with Vision's default settings in case of new user
	InitDefaults();

	Theme::TimestampFore = C_TIMESTAMP;
	Theme::TimestampBack = C_TIMESTAMP;
	Theme::TimespaceFore = MAX_COLORS;
	Theme::TimespaceBack = MAX_COLORS;
	Theme::TimespaceFont = MAX_FONTS;
	Theme::TimestampFont = F_TIMESTAMP;
	Theme::NormalFore = C_TEXT;
	Theme::NormalBack = C_TEXT;
	Theme::NormalFont = F_TEXT;
	Theme::SelectionBack = C_SELECTION;

	if (fDebugSettings)
		printf (":SETTINGS: loading...\n");

	fVisionSettings = new SettingsFile ("VisionSettings", "Vision");

	if (fVisionSettings->InitCheck() == B_OK)
	{
		fVisionSettings->Load();
	}
	else
		printf(":ERROR: Error Loading /Vision/VisionSettings\n");

	LoadAliases();

	int32 i (0);

	LoadDefaults (SET_SERVER);
	LoadDefaults (SET_GENERAL);
	LoadDefaults (SET_WINDOW);
	LoadDefaults (SET_NOTIFY);
	LoadDefaults (SET_FONT);
	LoadDefaults (SET_COLOR);
	LoadDefaults (SET_STRINGS);
	LoadDefaults (SET_DCC);

	// initialize theme, TODO: move to separate function

	fActiveTheme->WriteLock();

	for (i = 0; i < MAX_COLORS; i++)
	{
		fActiveTheme->SetForeground (i, fColors[i]);
		fActiveTheme->SetBackground (i, fColors[C_BACKGROUND]);
	}
	for (i = C_MIRC_WHITE; i < MAX_COLORS; i++)
		fActiveTheme->SetBackground (i, fColors[i]);

	fActiveTheme->SetBackground (C_SELECTION, fColors [C_SELECTION]);
	fActiveTheme->SetBackground (C_TIMESTAMP, fColors[C_TIMESTAMP_BACKGROUND]);
	fActiveTheme->SetBackground (MAX_COLORS, fColors[C_BACKGROUND]);
	fActiveTheme->SetForeground (MAX_COLORS, fColors[C_TEXT]);
	fActiveTheme->SetFont (MAX_FONTS, fClientFont[F_TEXT]);

	for (i = 0; i < MAX_FONTS; i++)
		fActiveTheme->SetFont (i, fClientFont[i]);

	fActiveTheme->WriteUnlock();

	fSettingsLoaded = true;
	if (fDebugSettings)
		printf (":SETTINGS: done loading\n");
}

bool
VisionApp::SaveSettings (void)
{
	BAutolock saveLock (const_cast<BLocker *>(&fSettingsLock));

	if (!saveLock.IsLocked())
		return false;

	SaveAliases();

	if ((fVisionSettings->Save() == B_OK) && fDebugSettings)
	{
		printf (":SETTINGS: saved to file\n");
		return true;
	}
	return false;
}

void
VisionApp::LoadDefaults (int32 section)
{
	// sets defaults for various states in vision

	switch (section)
	{
		case SET_SERVER:
			{
				if (!fVisionSettings->HasMessage ("defaults"))
				{
					BMessage defaults (VIS_NETWORK_DEFAULTS);
					defaults.AddString ("name", "defaults");
					defaults.AddString ("nick", "vision");
					defaults.AddString ("nick", "vision2");
					defaults.AddString ("ident", "vision");
					defaults.AddString ("realname", "Heisenberg may have slept here");
					fVisionSettings->AddMessage ("defaults", &defaults);
				}
			}
			break;

		case SET_GENERAL:
			{
				if (!fVisionSettings->HasBool ("versionParanoid"))
					fVisionSettings->AddBool ("versionParanoid", false);

				if (!fVisionSettings->HasBool ("timestamp"))
					fVisionSettings->AddBool ("timestamp", false);

				if (!fVisionSettings->HasString ("timestamp_format"))
					fVisionSettings->AddString ("timestamp_format", "[%H:%M]");

				if (!fVisionSettings->HasBool ("log_enabled"))
					fVisionSettings->AddBool ("log_enabled", false);

				if (!fVisionSettings->HasBool ("log_filetimestamp"))
					fVisionSettings->AddBool ("log_filetimestamp", false);

				if (!fVisionSettings->HasBool ("stripcolors"))
					fVisionSettings->AddBool ("stripcolors", true);

				if (!fVisionSettings->HasBool ("Newbie Spam Mode"))
					fVisionSettings->AddBool("Newbie Spam Mode", true);

				if (!fVisionSettings->HasBool ("queryOnMsg"))
					fVisionSettings->AddBool ("queryOnMsg", false);

				if (!fVisionSettings->HasBool ("notifyExpanded"))
					fVisionSettings->AddBool("notifyExpanded", true);

				if (!fVisionSettings->HasString ("logBaseDir")) {
					BPath base;
					if (find_directory(B_USER_DATA_DIRECTORY, &base) == B_OK) {
						base.Append("logs");
						base.Append("Vision");
					}
					fVisionSettings->AddString("logBaseDir", base.Path());
				}

				if (!fVisionSettings->HasInt32 ("encoding"))
					fVisionSettings->AddInt32("encoding", B_UNICODE_CONVERSION);
			}
			break;

		case SET_WINDOW:
			{
				if (!fVisionSettings->HasBool ("catchAltW"))
					fVisionSettings->AddBool ("catchAltW", false);

				if (!fVisionSettings->HasRect ("clientWinRect"))
					fVisionSettings->AddRect ("clientWinRect", BRect (100, 100, 600, 460));

				if (!fVisionSettings->HasRect ("windowDockRect"))
					fVisionSettings->AddRect ("windowDockRect", BRect (0, 0, 0, 0));

				if (!fVisionSettings->HasRect ("NetPrefWinRect"))
					fVisionSettings->AddRect ("NetPrefWinRect", BRect (0, 0, 0, 0));

				if (!fVisionSettings->HasRect ("namesListRect"))
					fVisionSettings->AddRect ("namesListRect", BRect (0, 0, 100, 0));

				if (!fVisionSettings->HasRect ("GenPrefWinRect"))
					fVisionSettings->AddRect ("GenPrefWinRect", BRect (0,0,0,0));
			}
			break;

		case SET_NOTIFY:
			{
				if (!fVisionSettings->HasString ("alsoKnownAs"))
					fVisionSettings->AddString ("alsoKnownAs", "-9y99");
			}
			break;

		case SET_FONT:
		 {
			 font_family default_family;
			 font_style default_style;
			 float	size (0.0);
			 be_plain_font->GetFamilyAndStyle (&default_family, &default_style);
			 size = be_plain_font->Size();
			 int32 i (0);

			 for (i = 0; i < MAX_FONTS; i++)
			 {
				 BString fontStr, styleStr;
				 fontStr = "family";
				 styleStr = "style";
				 fontStr << i;
				 styleStr << i;
				 if (!fVisionSettings->HasString (fontStr.String()))
				 {
					 fVisionSettings->AddString (fontStr.String(), default_family);
					 fVisionSettings->AddString (styleStr.String(), default_style);
					 fVisionSettings->AddFloat ("size", size);
				 }
				 else
				 {
					 BString family;
					 BString style;
					 size = 0.0;

					 fVisionSettings->FindString (fontStr.String(), &family);
					 fVisionSettings->FindString (styleStr.String(), &style);
					 fVisionSettings->FindFloat ("size", i, &size);

					 ClientFontFamilyAndStyle (i, family.String(), style.String());
					 ClientFontSize (i, size);
				 }
			 }
		 }
		 break;

	 case SET_COLOR:
		 {
			 // load defaults from color array into settings file
			 for (int32 i = 0; i < MAX_COLORS; i++)
				 if (!fVisionSettings->HasData ("color", B_RGB_COLOR_TYPE, i))
				 {
					 fVisionSettings->AddData ("color", B_RGB_COLOR_TYPE, &fColors[i], sizeof (rgb_color));
				 }
				 else
				 {
					 const rgb_color *color;
					 ssize_t size (0);
					 if (fVisionSettings->FindData ("color", B_RGB_COLOR_TYPE, i, reinterpret_cast<const void **>(&color), &size) == B_OK)
						 fColors[i] = *color;
				 }
		 }
		 break;

	 case SET_STRINGS:
		 {
			 BString eventStr, commandStr;
			 int32 i(0);
			 for (i = 0; i < MAX_EVENTS; i++)
			 {
				 eventStr = "event";
				 eventStr << i;
				 if (!fVisionSettings->HasString (eventStr.String()))
					 fVisionSettings->AddString (eventStr.String(), fEvents[i]);
				 else
					 fVisionSettings->FindString (eventStr.String(), &fEvents[i]);
			 }

			 // fCommands
			 for (i = 0; i < MAX_COMMANDS; i++)
			 {
				 commandStr = "command";
				 commandStr << i;
				 if (!fVisionSettings->HasString (commandStr.String()))
					 fVisionSettings->AddString (commandStr.String(), fCommands[i]);
				 else
					 fVisionSettings->FindString (commandStr.String(), &fCommands[i]);
			 }
		 }
		 break;

	 case SET_DCC:
		 {
			 if (!fVisionSettings->HasString ("dccDefPath"))
				 fVisionSettings->AddString ("dccDefPath", "/boot/home");
			 if (!fVisionSettings->HasBool ("dccAutoAccept"))
				 fVisionSettings->AddBool ("dccAutoAccept", false);
			 if (!fVisionSettings->HasBool ("dccPrivateCheck"))
				 fVisionSettings->AddBool ("dccPrivateCheck", true);
			 if (!fVisionSettings->HasString ("dccBlockSize"))
				 fVisionSettings->AddString ("dccBlockSize", "2048");
			 if (!fVisionSettings->HasString ("dccMinPort"))
				 fVisionSettings->AddString ("dccMinPort", "40000");
			 if (!fVisionSettings->HasString ("dccMaxPort"))
				 fVisionSettings->AddString ("dccMaxPort", "45000");
		 }
		 break;
	}
}

bool
VisionApp::QuitRequested (void)
{
	fShuttingDown = true;

	if (fIdentSocket >= 0)
		close (fIdentSocket);


	thread_id tid = network_manager->Thread();
	status_t result;
	BMessenger(network_manager).SendMessage(B_QUIT_REQUESTED);
	wait_for_thread(tid, &result);

	BMessenger msgr(fClientWin);
	if (msgr.IsValid())
		msgr.SendMessage(B_QUIT_REQUESTED);

	if (fSetupWin)
		BMessenger(fSetupWin).SendMessage(B_QUIT_REQUESTED);

	if (fPrefsWin)
		BMessenger(fPrefsWin).SendMessage(B_QUIT_REQUESTED);

	if (fNetWin)
		BMessenger(fNetWin).SendMessage(B_QUIT_REQUESTED);

	// give our child threads a chance to die gracefully
	while (ThreadStates() > 1)
		snooze (100000);

	if (fSettingsLoaded)
	{
		SaveSettings();
		delete fVisionSettings;
	}

	return true;
}


void
VisionApp::AboutRequested (void)
{
	if (fAboutWin)
	{
		fAboutWin->Activate();
	}
	else
	{
		fAboutWin = new AboutWindow();
		fAboutWin->Show();
	}
}


void
VisionApp::ArgvReceived (int32 ac, char **av)
{
	for (int32 i = 1; i < ac; ++i)
	{

		if (strcmp (av[i], "-!") == 0)
		{
			fDebugRecv = true;
			fDebugSend = true;
			fDebugSettings = true;
			fNumBench = true;
			fDebugShutdown = true;
		}

		else if (strcmp (av[i], "-r") == 0)
			fDebugRecv = true;

		else if (strcmp (av[i], "-s") == 0)
			fDebugSend = true;

		else if (strcmp (av[i], "-S") == 0)
			fDebugSettings = true;

		else if (strcmp (av[i], "-u") == 0)
			fDebugShutdown = true;

		else if (strcmp (av[i], "-n") == 0)
			fNumBench = true;

		else if (strcmp (av[i], "-a") == 0)
			fDisableAutostart = true;

		else if (strcmp (av[i], "-T") == 0)
		{
#if 0
			TestScript *tscript = new TestScript();
			delete tscript;
#endif
			if (IsLaunching())
				Quit();
		}

		else if (strcmp (av[i], "--help") == 0)
		{
			printf ("Vision command line switches:\n");
			printf ("Devel:\n");
			printf ("\t-T\t\tRun TestScript() and quit\n");
			printf ("Debug:\n");
			printf ("\t-!\t\tPrint everything\n");
			printf ("\t-r\t\tPrint data received across the network\n");
			printf ("\t-s\t\tPrint data sent across the network\n");
			printf ("\t-S\t\tPrint settings debug information\n");
			printf ("\t-u\t\tPrint state debug information on shutdown\n");
			printf ("\t-a\t\tDisable auto-connect\n");
			printf ("\n");
			if (IsLaunching())
				Quit();
		}

	}
}


// check if any networks have the connect on startup flag marked...
// if they do, start them

bool
VisionApp::CheckStartupNetworks (void)
{
	bool autoStarted (false);
	if (!fDisableAutostart)
	{
		BMessage netData;
		for (int32 i = 0; (netData = GetNetwork(i)), !netData.HasBool ("error"); i++)
		{
			if (CheckNetworkValid (netData.FindString("name")) && netData.FindBool ("connectOnStartup"))
			{
				BMessage msg (M_CONNECT_NETWORK);
				msg.AddString ("network", netData.FindString ("name"));
				PostMessage (&msg);
				autoStarted = true;
			}
		}
	}
	return autoStarted;
}

bool
VisionApp::CheckNetworkValid (const char *name)
{
	BMessage netData (GetNetwork (name));
	if ( ((netData.HasString ("nick")
			&& netData.HasString ("realname")
			&& netData.HasString ("ident"))
			|| netData.FindBool ("useDefaults"))
		&& netData.HasData ("server", B_ANY_TYPE)
		&& netData.HasString ("name"))
		{
			const ServerData *data (NULL);
			ssize_t size;
			for (int32 i = 0; netData.FindData ("server", B_RAW_TYPE, i,
				reinterpret_cast<const void **>(&data), &size) == B_OK; i++)
			{
				// look for a primary server
				if (data->state == SERVER_PRIMARY)
					return true;
			}
		}
	return false;
}

void
VisionApp::ReadyToRun (void)
{
	InitSettings();

	if (!CheckStartupNetworks())
	{
		fSetupWin = new SetupWindow ();
		fSetupWin->Show();
	}

	network_manager = new NetworkManager();
	network_manager->Run();

	BMessenger msgr(network_manager);
	BMessage identMsg(M_CREATE_LISTENER);
	identMsg.AddString("port", "113");
	identMsg.AddMessenger("target", BMessenger(this));
	msgr.SendMessage(&identMsg);
}

void
VisionApp::LoadURL (const char *url)
{
	BString argument (url);

	if (argument[0] == '#')
	{
		BMessage msg(M_JOIN_CHANNEL);
		msg.AddString("channel", url);
		be_app_messenger.SendMessage(&msg);
		return;
	}

	if (argument.FindFirst ("://") == B_ERROR)
	{

		if (argument.IFindFirst ("www") == 0)
			argument.Prepend ("http://");

		else if (argument.IFindFirst ("ftp") == 0)
			argument.Prepend ("ftp://");
	}

	const char *args[] = { argument.String(), 0 };

	if (argument.IFindFirst ("file:") == 0)
	{
		// The URL is guaranteed to be at least "file:/"
		BString file(argument.String() + 5);

		args[0] = file.String();
		//printf("file: '%s'\n", file.String());
		be_roster->Launch (kTrackerSig, 1, const_cast<char **>(args));
	}
	else if (argument.IFindFirst ("mailto:") == 0)
	{
		be_roster->Launch ("text/x-email", 1, const_cast<char **>(args));
	}
	else
	{
		BString mimeType = B_URL_MIME_PREFIX;
		mimeType.Append(argument, argument.FindFirst(':'));
		//printf("mime:'%s'\n", mimeType.String());
		if (!BMimeType::IsValid(mimeType.String()))
			return;
		be_roster->Launch (mimeType.String(), 1, const_cast<char **>(args));
	}
}

void
VisionApp::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_ABOUT_CLOSE:
			{
				fAboutWin = 0;
				if (fShuttingDown)
					PostMessage (B_QUIT_REQUESTED);
			}
			break;

		case M_SETUP_SHOW:
			{
				if (fSetupWin)
					fSetupWin->Activate();
				else
				{
					fSetupWin = new SetupWindow ();
					fSetupWin->Show();
				}
			}
			break;

		case M_SETUP_CLOSE:
			{
				SaveSettings();
				fSetupWin = 0;
				if (fClientWin == NULL)
					PostMessage (B_QUIT_REQUESTED);
			}
			break;

		case M_PREFS_SHOW:
			{
				if (fPrefsWin)
					fPrefsWin->Activate();
				else
				{
					fPrefsWin = new PrefsWindow();
					fPrefsWin->Show();
				}
			}
			break;

		case M_PREFS_CLOSE:
			{
				SaveSettings();
				fPrefsWin = 0;
			}
			break;

		case M_NETWORK_SHOW:
			{
				if (fNetWin)
					fNetWin->Activate();
				else
				{
					fNetWin = new NetworkWindow();
					fNetWin->Show();

				}
			}
			break;

		case M_NETWORK_CLOSE:
			{
				SaveSettings();
				fNetWin = 0;
			}
			break;

		case M_CONNECT_NETWORK:
			{
				BRect clientWinRect (GetRect("clientWinRect"));
				BMessage netData = GetNetwork (msg->FindString ("network"));

				// sanity check
				if (netData.IsEmpty())
					break;

				if (netData.FindBool ("useDefaults"))
				{
					netData.RemoveName ("nick");
					netData.RemoveName ("realname");
					netData.RemoveName ("ident");
					BMessage netDefaults (GetNetwork ("defaults"));
					netData.AddString ("realname", netDefaults.FindString ("realname"));
					netData.AddString ("ident", netDefaults.FindString ("ident"));
					const char *nick (NULL);
					for (int32 i = 0; (nick = netDefaults.FindString ("nick", i)) != NULL; i++)
						netData.AddString ("nick", nick);
				}
				if (fClientWin == NULL)
				{
					fClientWin = new ClientWindow (clientWinRect);
					fWinThread = fClientWin->Thread();
					fClientWin->Show();
				}
				BMessage connMsg (M_MAKE_NEW_NETWORK);
				connMsg.AddMessage ("network", &netData);
				fClientWin->PostMessage (&connMsg);
			}
			break;

		case M_JOIN_CHANNEL:
			{
				if (fClientWin == NULL)
					break;

				fClientWin->PostMessage (msg);
			}
			break;

		case M_DCC_FILE_WIN:
		{
			if (fDccFileWin)
			{
				fDccFileWin->PostMessage (msg);
			}
			else
			{
				DCCConnect *view;

				msg->FindPointer ("view", reinterpret_cast<void **>(&view));
				fDccFileWin = new DCCFileWindow (view);
				fDccFileWin->Show();
			}
		}
		break;

		case M_DCC_MESSENGER:
		if (msg->IsSourceWaiting())
		{
			BMessenger msgr (fDccFileWin);
			BMessage reply;
			reply.AddMessenger ("msgr", msgr);
			msg->SendReply (&reply);
		}
		break;

		case M_DCC_FILE_WIN_DONE:
		{
			fDccFileWin = 0;
		}
		break;

		case M_DCC_COMPLETE:
		{
			Broadcast(msg);
		}
		break;

		case M_LOAD_URL:
		{
			BString url (msg->FindString("url"));
			if (url.Length() > 0)
			{
				LoadURL(url.String());
			}
		}
		break;

		case M_LISTENER_CREATED:
		{
			int32 status = -1;
			if (msg->FindInt32("status", &status) == B_OK && status == B_OK)
			{
				msg->FindInt32("connection", &fIdentSocket);
			}
		}
		break;

		case M_CONNECTION_ACCEPTED:
		{
			int32 socket = -1;
			BString address;
			BString name;
			if (msg->FindInt32("client", &socket) == B_OK)
			{
				msg->FindString("address", &address);
				msg->FindString("name", &name);
				fIdentAddresses[socket] = name.Length() > 0 ? name : address;
			}
		}
		break;

		case M_CONNECTION_DATA_RECEIVED:
		{
			int32 socket = -1;
			if (msg->FindInt32("connection", &socket) == B_OK)
			{
				BString remoteIP = fIdentAddresses[socket];
				BString ident = GetIdent (fIdentAddresses[socket]);
				BString data;

				if (ident.Length() > 0)
				{
					const char *buffer = NULL;
					ssize_t size = -1;
					msg->FindData("data", B_RAW_TYPE, reinterpret_cast<const void **>(&buffer), &size);
					data.SetTo(buffer, size);
					int32 spaceidx = data.Length() - 1;
					while (spaceidx > 0 && isspace(data[spaceidx]))
					{
						--spaceidx;
					}
					data.Truncate(spaceidx + 1);


					data.Append (" : USERID : Haiku : ");
					data.Append (ident);
					data.Append ("\r\n");
				}
				else
				{
					data.SetTo("0 , 0 : UNKNOWN : UNKNOWN-ERROR");
				}
				BMessage reply(M_SEND_CONNECTION_DATA);
				reply.AddInt32("connection", socket);
				reply.AddData("data", B_RAW_TYPE, data.String(), data.Length());
				BMessenger msgr(network_manager);
				msgr.SendMessage(&reply);

				reply.what = M_DESTROY_CONNECTION;
				msgr.SendMessage(&reply);
			}
		}
		break;

		default:
			BApplication::MessageReceived (msg);
	}
}

//////////////////////////////////////////////////////////////////////////////
/// End BApplication functions
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
/// Begin Public Functions
//////////////////////////////////////////////////////////////////////////////

ClientWindow *
VisionApp::pClientWin() const
{
	return fClientWin;
}

entry_ref
VisionApp::AppRef(void) const
{
	return fAppRef;
};

void
VisionApp::VisionVersion (int typebit, BString &result)
{
	switch (typebit)
	{
		case VERSION_VERSION:
			result = VERSION_STRING;
			break;

		case VERSION_DATE:
			result = BUILD_DATE;
			result.ReplaceAll ("_", " ");
			break;
	}
}

const char *
VisionApp::GetString (const char *stringName) const
{
	BAutolock stringLock (const_cast<BLocker *>(&fSettingsLock));

	const char *value;

	if (stringLock.IsLocked())
	{
		if (fDebugSettings)
			printf (":SETTINGS: looking up String \"%s\"... ", stringName);

		if ((fVisionSettings->FindString (stringName, &value)) == B_OK)
		{
			if (fDebugSettings)
				printf ("found; returning %s\n", value);
		}
		else
		{
			if (fDebugSettings)
				printf (" not found; returning NULL\n");
		}
	}
	return value;
}


void
VisionApp::SetString (const char *stringName, int32 index, const char *value)
{
	BAutolock stringLock (const_cast<BLocker *>(&fSettingsLock));

	if (!stringLock.IsLocked())
		return;

	if (!strcmp(stringName, "timestamp_format"))
	{
		BMessage msg (M_STATE_CHANGE);
		msg.AddBool ("string", true);
		msg.AddString ("which", stringName);
		Broadcast (&msg);
	}

	BString tmp;
	tmp = value;

	fVisionSettings->ReplaceString (stringName, index, tmp);
}

const BRect
VisionApp::GetRect (const char *settingName)
{
	BRect rect (0.0, 0.0, 0.0, 0.0);

	BAutolock rectLock (const_cast<BLocker *>(&fSettingsLock));

	if (!rectLock.IsLocked())
		return rect;

	if (fVisionSettings->HasRect (settingName))
		fVisionSettings->FindRect (settingName, &rect);

	return rect;
}

void
VisionApp::SetRect (const char *settingName, BRect value)
{
 BAutolock rectLock(const_cast<BLocker *>(&fSettingsLock));

 if (!rectLock.IsLocked())
	 return;

 fVisionSettings->ReplaceRect (settingName, value);
}


rgb_color
VisionApp::GetColor (int32 which) const
{
	rgb_color color = {0, 0, 0, 255};

	BAutolock colorLock (const_cast<BLocker *>(&fSettingsLock));

	if (!colorLock.IsLocked())
		return color;


	if (which < MAX_COLORS && which >= 0)
		color = fColors[which];

	return color;
}


void
VisionApp::SetColor (int32 which, const rgb_color color)
{

	BAutolock colorLock (const_cast<BLocker *>(&fSettingsLock));

	if (!colorLock.IsLocked())
		return;

	if (which < MAX_COLORS &&	which >= 0
	&& (fColors[which].red	 != color.red
	||	fColors[which].green != color.green
	||	fColors[which].blue	!= color.blue
	||	fColors[which].alpha != color.alpha))
	{
		fColors[which] = color;
		fVisionSettings->ReplaceData ("color", B_RGB_COLOR_TYPE, which,
			reinterpret_cast<void * const *>(&color), sizeof(rgb_color));
		fActiveTheme->WriteLock();
		if (which == C_BACKGROUND)
		{
			// update regular background color on all other text
			for (int32 i = 0; i < C_TIMESTAMP; i++)
				fActiveTheme->SetBackground (i , color);
			 fActiveTheme->SetBackground (MAX_COLORS, color);
		}

		// update timestamp bg color
		else if (which == C_TIMESTAMP_BACKGROUND)
			fActiveTheme->SetBackground (C_TIMESTAMP, color);
		else if (which == C_SELECTION)
			fActiveTheme->SetBackground (C_SELECTION, color);
		// mirc fColors need to be updated on both fore and back
		else if (which >= C_MIRC_WHITE)
		{
			fActiveTheme->SetForeground (which, color);
			fActiveTheme->SetBackground (which, color);
		}
		else
			fActiveTheme->SetForeground (which, color);
		fActiveTheme->WriteUnlock();
	}
}


/// begin font prefs ///
void
VisionApp::ClientFontFamilyAndStyle (
	int32 which,
	const char *family,
	const char *style)
{

	BAutolock fontLock (const_cast<BLocker *>(&fSettingsLock));

	if (!fontLock.IsLocked())
		return;

	if (which < MAX_FONTS && which >= 0)
	{
		BString fontStr;
		BString styleStr;
		fontStr = "family";
		fontStr << which;
		styleStr = "style";
		styleStr << which;
		fClientFont[which]->SetFamilyAndStyle (family, style);

		fActiveTheme->WriteLock();
		fActiveTheme->SetFont (which, fClientFont[which]);
		if (which == F_TEXT)
			fActiveTheme->SetFont (MAX_FONTS, fClientFont[which]);
		fActiveTheme->WriteUnlock();

		SetString (fontStr.String(), 0, family);
		SetString (styleStr.String(), 0, style);
	}
}


void
VisionApp::ClientFontSize (int32 which, float size)
{
	BAutolock fontLock (const_cast<BLocker *>(&fSettingsLock));

	if (!fontLock.IsLocked())
		return;

	if (which < MAX_FONTS && which >= 0)
	{
		fClientFont[which]->SetSize (size);

		fActiveTheme->WriteLock();
		fActiveTheme->SetFont (which, fClientFont[which]);
		fActiveTheme->WriteUnlock();

		if (fVisionSettings->ReplaceFloat ("size", which, size) != B_OK)
			printf("error, could not set font size\n");
	}
}

const BFont *
VisionApp::GetClientFont (int32 which) const
{
	BAutolock fontLock (const_cast<BLocker *>(&fSettingsLock));

	if (!fontLock.IsLocked())
		return NULL;

	return which < MAX_FONTS && which >= 0
		? fClientFont[which] : be_plain_font;
}
/// end font prefs ///


BString
VisionApp::GetEvent (int32 which) const
{
	BAutolock eventLock (const_cast<BLocker *>(&fSettingsLock));

	BString value;

	if (eventLock.IsLocked())
	{
		if (which < MAX_EVENTS && which >= 0)
		{
			value = fEvents[which];
		}
	}
	return value.String();
}


void
VisionApp::SetEvent (int32 which, const char *event)
{
	BAutolock eventLock (const_cast<BLocker *>(&fSettingsLock));

	if (!eventLock.IsLocked())
		return;

	BString eventStr;
	eventStr = "event";
	eventStr << which;

	if (which < MAX_EVENTS && which >= 0
	&&	fEvents[which].Compare (event))
	{
		fEvents[which] = event;

		SetString (eventStr.String(), 0, event);
	}
}


BString
VisionApp::GetCommand (int32 which)
{
	BAutolock commandLock (const_cast<BLocker *>(&fSettingsLock));

	BString value;

	if (commandLock.IsLocked())
	{
		if (which < MAX_COMMANDS && which >= 0)
		{
			value = fCommands[which];
		}
	}
	return value;
}


void
VisionApp::SetCommand (int32 which, const char *command)
{
	BAutolock commandLock (const_cast<BLocker *>(&fSettingsLock));

	if (!commandLock.IsLocked())
		return;

	BString commandStr;
	commandStr = "command";
	commandStr << which;

	if (which < MAX_COMMANDS && which >= 0)
	{
		fCommands[which] = command;

		SetString (commandStr.String(), 0, command);
	}
}


bool
VisionApp::GetBool (const char *settingName)
{
	BAutolock boolLock (const_cast<BLocker *>(&fSettingsLock));

	if (!boolLock.IsLocked())
		return B_ERROR;

	if (fDebugSettings)
		printf (":SETTINGS: looking up bool \"%s\"... ", settingName);

	bool value (false);

	if (fVisionSettings->FindBool (settingName, &value) == B_OK)
	{
		if (fDebugSettings)
			printf ("found; returning %s\n", (value)? "true" : "false");
	}
	else
	{
		if (fDebugSettings)
			printf (" not found; returning false\n");
	}

	return value;
}


status_t
VisionApp::SetBool (const char *settingName, bool value)
{
	BAutolock boolLock (const_cast<BLocker *>(&fSettingsLock));

	if (!boolLock.IsLocked())
		return B_ERROR;

	status_t result (B_OK);

	if ((result = fVisionSettings->ReplaceBool (settingName, value)) == B_OK)
	{
		BMessage msg (M_STATE_CHANGE);
		msg.AddBool ("bool", true);
		Broadcast (&msg);
	}

	return result;
}

int32
VisionApp::GetInt32 (const char *settingName)
{
	BAutolock intLock (const_cast<BLocker *>(&fSettingsLock));

	if (!intLock.IsLocked())
		return B_ERROR;

	return fVisionSettings->FindInt32(settingName);
}

status_t
VisionApp::SetInt32 (const char*settingName, int32 value)
{
	BAutolock intLock (const_cast<BLocker *>(&fSettingsLock));

	if (!intLock.IsLocked())
		return B_ERROR;

	status_t result = fVisionSettings->ReplaceInt32 (settingName, value);

	return result;
}

BMessage
VisionApp::GetNetwork (const char *network)
{
	BMessage msg (VIS_NETWORK_DATA);

	BAutolock netLock (const_cast<BLocker *>(&fSettingsLock));

	if (netLock.IsLocked())
	{
		if (!strcmp (network, "defaults"))
		{
			fVisionSettings->FindMessage ("defaults", &msg);
			return msg;
		}
		type_code type;
		int32 netCount (0);
		fVisionSettings->GetInfo ("network", &type, &netCount);
		for (int32 i = 0; i < netCount; i++)
		{
			 BMessage tempMsg;
			 fVisionSettings->FindMessage ("network", i, &tempMsg);
			 if (!strcmp (tempMsg.FindString ("name"), network))
			 {
				 msg = tempMsg;
				 break;
			 }
		}
	}
	return msg;
}

BMessage
VisionApp::GetNetwork (int32 index)
{
	BMessage msg (VIS_NETWORK_DATA);

	BAutolock netLock (const_cast<BLocker *>(&fSettingsLock));

	if (netLock.IsLocked())
		if (fVisionSettings->FindMessage ("network", index, &msg) != B_OK)
			msg.AddBool ("error", true);
	return msg;
}

status_t
VisionApp::SetNetwork (const char *network, BMessage *data)
{
	BAutolock netLock (const_cast<BLocker *>(&fSettingsLock));

	if (!netLock.IsLocked())
		return B_ERROR;

	if (!strcmp (network, "defaults"))
	{
		fVisionSettings->ReplaceMessage ("defaults", data);
		return B_OK;
	}

	type_code type;
	int32 netCount (0);
	fVisionSettings->GetInfo ("network", &type, &netCount);
	for (int32 i = 0; i < netCount;)
	{
		 BMessage tempMsg;
		 fVisionSettings->FindMessage ("network", i, &tempMsg);
		 if (strcmp (tempMsg.FindString ("name"), network) == 0)
		 {
			 fVisionSettings->ReplaceMessage ("network", i, data);
			 return B_OK;
		 }
		 else ++i;
	}
	fVisionSettings->AddMessage ("network", data);
	return B_OK;

}

status_t
VisionApp::RemoveNetwork (const char *network)
{
	BMessage msg (VIS_NETWORK_DATA);

	BAutolock netLock (const_cast<BLocker *>(&fSettingsLock));

	if (!netLock.IsLocked())
		return B_ERROR;

	type_code type;
	int32 netCount (0);
	fVisionSettings->GetInfo ("network", &type, &netCount);
	for (int32 i = 0; i < netCount;)
	{
		 BMessage tempMsg;
		 fVisionSettings->FindMessage ("network", i, &tempMsg);
		 if (!strcmp (tempMsg.FindString ("name"), network))
		 {
			 fVisionSettings->RemoveData ("network", i);
			 break;
		 }
		 else ++i;
	}
	return B_OK;
}

Theme *
VisionApp::ActiveTheme (void)
{
	return fActiveTheme;
}

void
VisionApp::GetThreadName (int thread_type, BString &output)
{
	// random names for the connection thread
	static const char *tnames[] = {
		/*	0 */ "gummi_bear_orgy",
		/*	1 */ "complimentary_tote_bag",
		/*	2 */ "cheating_at_solitaire",
		/*	3 */ "impatient",
		/*	4 */ "personal_info_uploader",
		/*	5 */ "keystroke_logger",
		/*	6 */ "rc5_cracker",
		/*	7 */ "seti_at_home",
		/*	8 */ "stare_girl",
		/*	9 */ "baron_arnold",
		/* 10 */ "nsa_pingflood",
		/* 11 */ "random_death_threat_emailer",
		/* 12 */ "marketing_research_assistant",
		/* 13 */ "beos_stock_monitor",
		/* 14 */ "stack_underflow",
		/* 15 */ "the_matrix_has_you",
		/* 16 */ "follow_the_white_rabbit",
		/* 17 */ "psycho psycho_killer killer",
		/* 18 */ "a_s_l_check_msg_on_join",
		/* 19 */ "wudan_training", // http://us.imdb.com/Title?0190332
		/* 20 */ "peer_reset_thread",
		/* 21 */ "chocoak_is_my_hero",
		/* 22 */ "blossom",	 // commander and the leader
		/* 23 */ "bubbles",	 // the joy and the laughter
		/* 24 */ "buttercup", // shes the toughest fighter
													// Powerpuffs save the day!
		/* 25 */ "youlooklikeyouneedamonkey",
		/* 26 */ "wegotdeathstar",		// we got def star! we got def star!
		/* 27 */ "whatcha_gonna_do",	// uh oh!
		/* 28 */ "lookit! a ball!",
		/* 29 */ "5038",
		/* 30 */ "talk_to_the_hand",
		/* 31 */ "manah, manah!",
		/* 32 */ "magic 8-nipple",
		/* 33 */ "threat_mode",
		/* 34 */ "dark_and_mysterious",
		/* 35 */ "I AM A GOLDEN GOD!",
		/* 36 */ "catrec",
		/* 37 */ "bork! bork! bork!",
		/* 38 */ "geisha_slut_villainess",
		/* 39 */ "ball_gravity_control",
		/* 40 */ "exploding_cow",
		/* 41 */ "naked scottish weathergirls",
		/* 42 */ "gateway game^Wthread",
		/* 43 */ "hello kitty",
		/* 44 */ "please_fondle_my_buttocks",
		/* 45 */ "the_game's_afoot_watson!",
		/* 46 */ "Stop making that big face!",
		/* 47 */ "hush",
		/* 48 */ "Doctor Nick M.Ed.",
		/* 49 */ "will_code_for_food",
		/* 50 */ "Dig Me Out",
		/* 51 */ "Little Babies",
		/* 52 */ "daydreaming",
		/* 53 */ "BEWARE OF THE DUCK!" // this one's for you freston :-)
	};

	int rnd (rand() % 54);

	switch (thread_type)
	{
		case THREAD_S:
			output = "s>";
			break;

		case THREAD_L:
			output = "l>";
			break;
	}

	output += tnames[rnd];
}

void
VisionApp::BenchOut (const char *ts)
{
	// note: this doesn't waste time figuring out whole seconds
	// output won't look right with more than 0.999s differences.

	int32 bench0;
	bench0 = fBench2 - fBench1;
	bench0 = bench0 / 100;

	printf ("%s: 0.%04" B_PRId32 "s\n", ts, bench0);
}

void
VisionApp::Broadcast (BMessage *msg)
{
	for (int32 i = 0; i < CountWindows(); ++i)
	{
		BMessenger msgr (WindowAt (i));
		msgr.SendMessage (msg);
	}
}

void
VisionApp::Broadcast (BMessage *, const char *, bool)
{
//	TODO: implement or remove this
//	Lock();
//
//	for (int32 i = 0; i < CountWindows(); ++i)
//	{
//		ServerWindow *serverWindow (dynamic_cast<ServerWindow *>(WindowAt (i)));
//
//		if (serverWindow
//		&&	serverWindow->Id().ICompare (serverName) == 0)
//			if (active)
//				serverWindow->PostActive (msg);
//			else
//				serverWindow->RepliedBroadcast (msg);
//	}
//
//	Unlock();
}

void
VisionApp::AddIdent (const char *server, const char *serverIdent)
{
	fIdents.AddString (server, serverIdent);
}

void
VisionApp::RemoveIdent (const char *server)
{
	fIdents.RemoveName (server);
}

BString
VisionApp::GetIdent (const char *server)
{
	BString ident;
	if (fIdents.HasString (server))
		ident = fIdents.FindString (server);

	return ident;
}

void
VisionApp::AddNotifyNick (const char *network, const char *nick)
{
	// in case user has deleted the network in question, unlikely but better safe than sorry
	BMessage netMsg (GetNetwork (network));
	if (!netMsg.HasString("name"))
		return;

	type_code type;
	int32 attrCount;

	// make sure this nick hasn't already been added
	netMsg.GetInfo ("notify", &type, &attrCount);
	for (int32 i = 0; i < attrCount; i++)
	{
		if (!strcmp(netMsg.FindString ("notify", i), nick))
			return;
	}

	netMsg.AddString ("notify", nick);
	SetNetwork (network, &netMsg);
}

void
VisionApp::RemoveNotifyNick (const char *network, const char *nick)
{
	BMessage netMsg (GetNetwork (network));

	type_code type;
	int32 attrCount, i;
	netMsg.GetInfo ("notify", &type, &attrCount);
	for (i = 0; i < attrCount; i++)
	{
		if (!strcasecmp(netMsg.FindString ("notify", i), nick))
		{
			netMsg.RemoveData ("notify", i);
			break;
		}
	}
	if (i < attrCount)
		SetNetwork (network, &netMsg);
}

void
VisionApp::AddIgnoreNick (const char *network, const char *nick, bool exclude)
{
	// in case user has deleted the network in question, unlikely but better safe than sorry
	BMessage netMsg (GetNetwork (network));
	if (!netMsg.HasString("name"))
		return;

	char optype[8];
	memset(optype, 0, sizeof(optype));
	if (exclude)
	{
		strcpy(optype, "exclude");
	}
	else
	{
		strcpy(optype, "ignore");
	}

	type_code type;
	int32 attrCount;

	// make sure this nick hasn't already been added
	netMsg.GetInfo (optype, &type, &attrCount);
	for (int32 i = 0; i < attrCount; i++)
	{
		if (!strcmp(netMsg.FindString (optype, i), nick))
			return;
	}

	netMsg.AddString (optype, nick);
	SetNetwork (network, &netMsg);
}

void
VisionApp::RemoveIgnoreNick (const char *network, const char *nick, bool exclude)
{
	BMessage netMsg (GetNetwork (network));

	char optype[8];
	memset(optype, 0, sizeof(optype));
	if (exclude)
	{
		strcpy(optype, "exclude");
	}
	else
	{
		strcpy(optype, "ignore");
	}

	type_code type;
	int32 attrCount, i;
	netMsg.GetInfo (optype, &type, &attrCount);
	for (i = 0; i < attrCount; i++)
	{
		if (!strcasecmp(netMsg.FindString (optype, i), nick))
		{
			netMsg.RemoveData (optype, i);
			break;
		}
	}
	if (i < attrCount)
		SetNetwork (network, &netMsg);
}


void
VisionApp::AcquireDCCLock (void)
{
	fDccLock.Lock();
}

void
VisionApp::ReleaseDCCLock (void)
{
	fDccLock.Unlock();
}

bigtime_t
VisionApp::VisionUptime (void)
{
	return system_time() - fStartupTime;
}

bool
VisionApp::HasAlias(const BString &cmd) const
{
	return fAliases.find(cmd) != fAliases.end();
}

BString
VisionApp::ParseAlias(const char *cmd, const BString &channel)
{
	BString command(GetWord(cmd, 1).ToUpper());
	BString newcmd = fAliases[command];
	const char *parse = newcmd.String();

	int32 varidx (0);

	newcmd.ReplaceAll("$C", channel.String());

	while (*parse != '\0')
	{
		if (*parse != '$')
		{
			++parse;
			continue;
		}
		else
		{
			++parse;
			if (isdigit(*parse))
			{
				varidx = atoi(parse);
				BString replString = "$";
				replString << varidx;
				if (*(parse + replString.Length() - 1) != '-')
				{
					newcmd.ReplaceAll(replString.String(), GetWord(cmd, varidx + 1).String());
				}
				else
				{
					replString += "-";
					newcmd.ReplaceAll(replString.String(), RestOfString(cmd, varidx + 1).String());
				}
			}
		}
	}
	return newcmd;
}

status_t
VisionApp::AddAlias(const BString &cmd, const BString &value)
{
	status_t result (B_OK);
	if (cmd != "" && value != "")
	{
		fAliases[cmd] = value;
	}
	else
	{
		result = B_ERROR;
	}

	return result;
}

void
VisionApp::RemoveAlias(const BString &cmd)
{
	map<BString, BString>::iterator it = fAliases.find(cmd);
	if (it != fAliases.end())
	{
		fAliases.erase(it);
	}
}

void
VisionApp::LoadAliases(void)
{
	BPath settingsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath) < B_OK)
		return;
	settingsPath.Append(kAliasPathName);
	if (settingsPath.InitCheck() < B_OK)
		return;
	BFile file (settingsPath.Path(), B_READ_ONLY);
	if (file.InitCheck() == B_OK)
	{
		BString data;
		char buffer[2048];
		memset(buffer, 0, sizeof(buffer));
		while (file.Read((void *)buffer, 2048) > 0)
		{
			 data += buffer;
			 memset(buffer, 0, sizeof(buffer));
		}
		file.Unset();
		while (data.Length() > 0)
		{
			BString cmd, value;
			int32 idx = data.IFindFirst("\t");
			if (idx != B_ERROR)
			{
				data.MoveInto(cmd, 0, idx);
				data.Remove(0,1);
			}
			else
			{
				break;
			}
			idx = data.IFindFirst("\n");
			if (idx != B_ERROR)
			{
				data.MoveInto(value, 0, idx);
				data.Remove(0,1);
			}
			else
			{
				break;
			}
			fAliases[cmd.ToUpper()] = value;
		}
	}
}

void
VisionApp::SaveAliases(void)
{
	BPath settingsPath;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &settingsPath) < B_OK)
		return;
	settingsPath.Append(kAliasPathName);
	if (settingsPath.InitCheck() < B_OK)
		return;
	BFile file (settingsPath.Path(), B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK)
	{
		for (map<BString, BString>::iterator it = fAliases.begin(); it != fAliases.end(); ++it)
		{
			file.Write(it->first.String(), it->first.Length());
			file.Write("\t", 1);
			file.Write(it->second.String(), it->second.Length());
			file.Write("\n", 1);
		}
		file.Unset();
	}
}

int32
VisionApp::CountAliases(void) const
{
	return fAliases.size();
}

bool
VisionApp::GetNextAlias(void **cookie, BString &name, BString &value)
{
	map<BString, BString>::const_iterator *it (NULL);
	if (*cookie == NULL)
	{
		it = new map<BString, BString>::const_iterator;
		*it = fAliases.begin();
		*cookie = it;
	}
	else
	{
		it = (map<BString, BString>::const_iterator *)*cookie;
	}
	if (*it != fAliases.end())
	{
		name = (*it)->first;
		value = (*it)->second;
		++(*it);
		return true;
	}
	else
	{
		delete it;
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////////
/// End Public Functions
//////////////////////////////////////////////////////////////////////////////

