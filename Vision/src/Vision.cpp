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
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <guru@startrek.com>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 */

class VisionApp * vision_app;

#include <Alert.h>
#include <Resources.h>
#include <Font.h>
#include <Autolock.h>
#include <Roster.h>

#include <algorithm>
#include <stdio.h>

#include "AboutWindow.h"
#include "Vision.h"
#include "ClientWindow.h"
#include "SettingsFile.h"
#include "SetupWindow.h"

#include "TestScript.h"

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
      aboutWin (0)
{
  // some setup
  settingsloaded = false;
  debugrecv = false;
  debugsend = false;
  debugsettings = false;
  
  // :TODO: wade 013101 move colors to settings class
  const rgb_color myBlack             = {0,0,0, 255};
  const rgb_color myWhite             = {255, 255, 255, 255};
  const rgb_color NOTICE_COLOR        = {10,90,170, 255};
  const rgb_color ACTION_COLOR        = {225,10,10, 255};
  const rgb_color QUIT_COLOR          = {180,10,10, 255};
  const rgb_color ERROR_COLOR         = {210,5,5, 255};
  const rgb_color URL_COLOR	          = {5,5,150, 255};
  const rgb_color NICK_COLOR          = {10,10,190, 255};
  const rgb_color MYNICK_COLOR        = {200,10,20, 255};
  const rgb_color JOIN_COLOR          = {10,130,10, 255};
  const rgb_color KICK_COLOR          = {250,130,10, 255};
  const rgb_color WHOIS_COLOR         = {10,30,170, 255};
  const rgb_color OP_COLOR            = {140,10,40, 255};
  const rgb_color VOICE_COLOR         = {160, 20, 20, 255};
  const rgb_color CTCP_REQ_COLOR      = {10,10,180, 255};
  const rgb_color CTCP_RPY_COLOR      = {10,40,180, 255};
  const rgb_color IGNORE_COLOR        = {100, 100, 100, 255};
  const rgb_color INPUT_COLOR         = {0, 0, 0, 255};
  const rgb_color INPUT_BG_COLOR      = {255, 255, 255, 255};
  const rgb_color WINLIST_BG_COLOR    = {238, 242, 242, 255};
  const rgb_color WINLIST_SEL_COLOR   = ui_color (B_PANEL_BACKGROUND_COLOR);
	

	colors[C_TEXT]                      = myBlack;
	colors[C_BACKGROUND]                = myWhite;
	colors[C_NAMES]                     = myBlack;
	colors[C_NAMES_BACKGROUND]          = myWhite;
	colors[C_URL]                       = URL_COLOR;
	colors[C_SERVER]                    = myBlack;
	colors[C_NOTICE]                    = NOTICE_COLOR;
	colors[C_ACTION]                    = ACTION_COLOR;
	colors[C_QUIT]                      = QUIT_COLOR;
	colors[C_ERROR]                     = ERROR_COLOR;
	colors[C_NICK]                      = NICK_COLOR;
	colors[C_MYNICK]                    = MYNICK_COLOR;
	colors[C_JOIN]                      = JOIN_COLOR;
	colors[C_KICK]                      = KICK_COLOR;
	colors[C_WHOIS]                     = WHOIS_COLOR;
	colors[C_OP]                        = OP_COLOR;
	colors[C_HELPER]					 = OP_COLOR;
	colors[C_VOICE]                     = VOICE_COLOR;
	colors[C_CTCP_REQ]                  = CTCP_REQ_COLOR;
	colors[C_CTCP_RPY]                  = CTCP_RPY_COLOR;
	colors[C_IGNORE]                    = IGNORE_COLOR;
	colors[C_INPUT]                     = INPUT_COLOR;
	colors[C_INPUT_BACKGROUND]          = INPUT_BG_COLOR;
	colors[C_WINLIST_BACKGROUND]        = WINLIST_BG_COLOR;
	colors[C_WINLIST_TEXT]              = myBlack;
	colors[C_WINLIST_NEWS]              = JOIN_COLOR;
	colors[C_WINLIST_NICK]              = QUIT_COLOR;
	colors[C_WINLIST_SELECTION]         = WINLIST_SEL_COLOR;
	
	client_font[F_TEXT]    = new BFont (be_fixed_font);
	client_font[F_SERVER]  = new BFont (be_fixed_font);
	client_font[F_URL]     = new BFont (be_fixed_font);
	client_font[F_NAMES]   = new BFont (be_plain_font);
	client_font[F_INPUT]   = new BFont (be_fixed_font);
	client_font[F_WINLIST] = new BFont (be_plain_font);
	
	events[E_JOIN]							= "*** $N ($I@$A) has joined the channel.";
	events[E_PART]							= "*** $N has left the channel.";
	events[E_NICK]							= "*** $N is now known as $n.";
	events[E_QUIT]							= "*** $N ($I@$A) has quit IRC ($R)";
	events[E_KICK]							= "*** $N has been kicked from $C by $n ($R)";
	events[E_TOPIC]							= "*** $C Topic changed by $N: $T";
	events[E_SNOTICE]						= "$R";
	events[E_UNOTICE]						= "-$N- $R";
	events[E_NOTIFY_ON]						= "*** $N has joined IRC.";
	events[E_NOTIFY_OFF]					= "*** $N has left IRC.";

	commands[CMD_KICK]					= "Ouch!";
	commands[CMD_QUIT]					= "Vision[$V]: i've been blurred!";
	commands[CMD_IGNORE]				= "*** $N is now ignored ($i).";
	commands[CMD_UNIGNORE]				= "*** $N is no longer ignored.";
	commands[CMD_AWAY]					= "is idle: $R";
	commands[CMD_BACK]					= "has returned";
	commands[CMD_UPTIME]				= "System was booted $U ago.";
}

void
VisionApp::InitSettings(void)
{
    if (debugsettings)
      printf (":SETTINGS: loading...\n");
      
	visionSettings = new SettingsFile ("VisionSettings", "Vision");
	// :TODO: Add default settings here

    
	if (visionSettings->InitCheck() == B_OK)
		visionSettings->Load();	
	else
      printf(":ERROR: Error Loading /Vision/VisionSettings\n");
      
    LoadDefaults (SET_SERVER);
    LoadDefaults (SET_GENERAL);
    LoadDefaults (SET_WINDOW);
 
    settingsloaded = true;
    if (debugsettings)
      printf (":SETTINGS: done loading\n");
      

}

void
VisionApp::LoadDefaults (int32 section)
{
  // sets defaults for various states in vision

  switch (section)
  {
    case SET_SERVER:
    {
      if (!visionSettings->HasString ("nickname1"))
        visionSettings->AddString ("nickname1", "vision");

      if (!visionSettings->HasString ("nickname2"))
        visionSettings->AddString ("nickname2", "vision2");

      if (!visionSettings->HasString ("realname"))
        visionSettings->AddString ("realname", "Heisenberg may have slept here");

      if (!visionSettings->HasString ("username"))
        visionSettings->AddString ("username", "vision");
    
      break;
    }
    
    case SET_GENERAL:
    {
      if (!visionSettings->HasBool ("versionParanoid"))
        visionSettings->AddBool ("versionParanoid", false);

      if (!visionSettings->HasBool ("timestamp"))
        visionSettings->AddBool ("timestamp", false);

      break;
    }
    
    case SET_WINDOW:
    {
      if (!visionSettings->HasBool ("catchAltW"))
        visionSettings->AddBool ("catchAltW", false);

      if (!visionSettings->HasRect ("clientWinRect"))
        visionSettings->AddRect ("clientWinRect", BRect (100, 100, 600, 460));
    
      break;
    }
  }
}

bool
VisionApp::QuitRequested (void)
{

  BMessage *quitRequest (CurrentMessage());

  if ((clientWin) && (!quitRequest->HasBool ("real_thing")))
  {
    clientWin->PostMessage (B_QUIT_REQUESTED);
    return false;
  }

  if (settingsloaded)
    if ((visionSettings->Save() == B_OK) && debugsettings)
      printf (":SETTINGS: saved to file\n");
    
  return true;
}


void
VisionApp::AboutRequested (void)
{
  if (aboutWin)
  {
    aboutWin->Activate();
  }
  else
  {
    aboutWin = new AboutWindow();
    aboutWin->Show();
  }
}


void
VisionApp::ArgvReceived (int32 ac, char **av)
{
  for (int32 i = 1; i < ac; ++i)
  {
    
    if (strcmp (av[i], "-!") == 0)
    {
      debugrecv = true;
      debugsend = true;
      debugsettings = true;
    }
    
    else if (strcmp (av[i], "-r") == 0)
      debugrecv = true;

    else if (strcmp (av[i], "-s") == 0)
      debugsend = true;
        
    else if (strcmp (av[i], "-S") == 0)
      debugsettings = true;
        
    else if (strcmp (av[i], "--help") == 0)
    {
      printf ("Vision command line switches:\n");
      printf ("Debug:\n");
      printf ("\t-!\t\tPrint everything\n");
      printf ("\t-r\t\tPrint data received across the network\n");
      printf ("\t-s\t\tPrint data sent across the network\n");
      printf ("\t-S\t\tPrint settings debug information\n");
      printf ("\n");
      if (IsLaunching())
        Quit();
    }
         
  }
}


void
VisionApp::ReadyToRun (void)
{
  printf ("# START # TestScript::RunTestScripts()\n"); 
  TestScript *tscript = new TestScript(); 
  printf ("# END   # TestScript::RunTestScripts()\n"); 
  
  InitSettings();

  BRect clientWinRect;
  visionSettings->FindRect ("clientWinRect", &clientWinRect);
  
  clientWin = new ClientWindow (clientWinRect);
  setupWin = new SetupWindow (true);
  clientWin->Show();
}

void
VisionApp::LoadURL (const char *url)
{
  const char *arguments[] = {url, 0};
  be_roster->Launch ("text/html", 1, const_cast<char **>(arguments));
}


void
VisionApp::MessageReceived (BMessage *msg)
{
  switch (msg->what)
  {
    case M_ABOUT_CLOSE:
    {
      aboutWin = 0;
      break;
    }
    
    case M_SETUP_SHOW:
    {
      if (setupWin)
      {
        setupWin->Activate();
      }
      else
      {
        setupWin = new SetupWindow(false);
        setupWin->Show();
      }
      
      break;
    }
    
    case M_SETUP_CLOSE:
    {
      setupWin = 0;
      break;
    }

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
  return clientWin;
}

BString
VisionApp::VisionVersion (void)
{
  BString output (VERSION_STRING);
  return output;
}

const char *
VisionApp::GetString (const char *stringName) const
{
  if (debugsettings)
    printf (":SETTINGS: looking up String \"%s\"... ", stringName);
    
  const char *value;
  
  if (visionSettings->FindString (stringName, &value) == B_OK)
    if (debugsettings)
      printf ("found; returning %s\n", value);
  else
    if (debugsettings)
      printf (" not found; returning NULL\n");
      
  return value;
}


status_t
VisionApp::SetString (const char *stringName, const char *value)
{
  BString temp (value);
  if (visionSettings->ReplaceString (stringName, temp) == B_OK)
    return B_OK;
  
  return B_ERROR;  
}

status_t
VisionApp::SetRect (const char *settingName, BRect value)
{
 if (visionSettings->ReplaceRect (settingName, value) == B_OK)
   return B_OK;
 
 return B_ERROR;
}


rgb_color
VisionApp::GetColor (int32 which) const
{
  rgb_color color = {0, 0, 0, 255};

  if (which < MAX_COLORS && which >= 0)
    color = colors[which];

  return color;
}


void
VisionApp::SetColor (int32 which, const rgb_color color)
{
  if (which < MAX_COLORS &&  which >= 0
  && (colors[which].red   != color.red
  ||  colors[which].green != color.green
  ||  colors[which].blue  != color.blue
  ||  colors[which].alpha != color.alpha))
  {
    colors[which] = color;

    BMessage msg (M_STATE_CHANGE);

    msg.AddInt32 ("which", which);
    msg.AddData (
      "color",
      B_RGB_COLOR_TYPE,
      colors + which,
      sizeof (rgb_color));
  }
}


/// begin font prefs ///
void
VisionApp::ClientFontFamilyAndStyle (
  int32 which,
  const char *family,
  const char *style)
{
  if (which < MAX_FONTS && which >= 0)
  {
    client_font[which]->SetFamilyAndStyle (family, style);

    BMessage msg (M_STATE_CHANGE);

    msg.AddInt32 ("which", which);
    msg.AddPointer ("font", client_font[which]);
    //Broadcast (&msg);
  }
}


void
VisionApp::ClientFontSize (int32 which, float size)
{
  if (which < MAX_FONTS && which >= 0)
  {
    client_font[which]->SetSize (size);

    BMessage msg (M_STATE_CHANGE);

    msg.AddInt32 ("which", which);
    msg.AddPointer ("font", client_font[which]);
    //Broadcast (&msg);
  }
}


const BFont *
VisionApp::GetClientFont (int32 which) const
{
  return which < MAX_FONTS && which >= 0 
    ? client_font[which] : new BFont (be_plain_font);
}
/// end font prefs ///


BString
VisionApp::GetEvent (int32 which) const
{
  BString buffer;

  if (which < MAX_EVENTS && which >= 0)
    buffer = events[which];

  return buffer;
}


void
VisionApp::SetEvent (int32 which, const char *event)
{
  if (which < MAX_EVENTS && which >= 0
  &&  events[which].Compare (event))
  {
    events[which] = event;

    BMessage msg (M_STATE_CHANGE);

    msg.AddInt32 ("which", which);
    msg.AddString ("event", events[which].String());

    Broadcast (&msg);
  }
}


BString
VisionApp::GetCommand (int32 which)
{
  BAutolock GetCommandLock (this);
  BString buffer;

  if (which < MAX_COMMANDS && which >= 0 && GetCommandLock.IsLocked())
    buffer = commands[which];

  return buffer;
}


void
VisionApp::SetCommand (int32 which, const char *command)
{
  BAutolock SetCommandLock (this);

  if (which < MAX_EVENTS && which >= 0 && SetCommandLock.IsLocked())
    commands[which] = command;
}


bool
VisionApp::GetBool (const char *settingName)
{
  if (debugsettings)
    printf (":SETTINGS: looking up bool \"%s\"... ", settingName);
    
  bool value (false);
  
  if (visionSettings->FindBool (settingName, &value) == B_OK)
    if (debugsettings)
      printf ("found; returning %s\n", (value)? "true" : "false");
  else
    if (debugsettings)
      printf (" not found; returning false\n");
      
  return value;
}


status_t
VisionApp::SetBool (const char *settingName, bool value)
{
 if (visionSettings->ReplaceBool (settingName, value) == B_OK)
   return B_OK;
 
 return B_ERROR;
}


const char *
VisionApp::GetThreadName (void)
{
  // random names for the connection thread
  BString buffer;
  BString tnames[31];
    
  tnames[0]  = "gummi_bear_orgy"; // pictures will be taken and uploaded
                                  // via the personal_info_uploader thread.
  tnames[1]  = "complimentary_tote_bag"; // mmmm... free stuff
  tnames[2]  = "cheating_at_solitaire"; 
  tnames[3]  = "impatient";
  tnames[4]  = "personal_info_uploader";
  tnames[5]  = "keystroke_logger"; 
  tnames[6]  = "rc5_cracker";
  tnames[7]  = "seti_at_home"; 
  tnames[8]  = "stare_girl"; // can you out stare stare girl?
  tnames[9]  = "baron_arnold"; // ba owns j00
  tnames[10] = "nsa_pingflood";
  tnames[11] = "random_death_threat_emailer";
  tnames[12] = "marketing_research_assistant";
  tnames[13] = "beos_stock_monitor";
  tnames[14] = "stack_underflow";
  tnames[15] = "the_matrix_has_you";
  tnames[15] = "follow_the_white_rabbit";
  tnames[16] = "psycho psycho_killer killer";
  tnames[17] = "a_s_l_check_msg_on_join";
  tnames[18] = "wudan_training"; // http://us.imdb.com/Title?0190332
  tnames[19] = "peer_reset_thread";
  tnames[20] = "chocoak_is_my_hero";
  tnames[21] = "blossom";   // commander and the leader
  tnames[22] = "bubbles";   // the joy and the laughter 
  tnames[23] = "buttercup"; // shes the toughest fighter
                            // Powerpuffs save the day!
  tnames[24] = "youlooklikeyouneedamonkey";
  tnames[25] = "wegotdeathstar";   // we got def star! we got def star!
  tnames[26] = "whatcha_gonna_do";  // uh oh!
  tnames[27] = "lookit! a ball!";
  tnames[28] = "5038";
  tnames[29] = "talk_to_the_hand";
  tnames[30] = "manah, manah!";
  tnames[31] = "magic 8-nipple";
  
  int rnd (rand() % 31);
  buffer = tnames[rnd];
    
  return buffer.String();
}

void
VisionApp::Broadcast (BMessage *msg)
{
  Lock();

  for (int32 i = 0; i < CountWindows(); ++i)
    WindowAt (i)->PostMessage (msg);

  Unlock();
}

void
VisionApp::Broadcast (BMessage *msg, const char *serverName, bool active)
{
//	Lock();
//
//	for (int32 i = 0; i < CountWindows(); ++i)
//	{
//		ServerWindow *serverWindow (dynamic_cast<ServerWindow *>(WindowAt (i)));
//
//		if (serverWindow
//		&&  serverWindow->Id().ICompare (serverName) == 0)
//			if (active)
//				serverWindow->PostActive (msg);
//			else
//				serverWindow->RepliedBroadcast (msg);
//	}
//
//	Unlock();
}


//////////////////////////////////////////////////////////////////////////////
/// End Public Functions
//////////////////////////////////////////////////////////////////////////////
