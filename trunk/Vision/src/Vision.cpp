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
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Rene Gollent
 *                 Todd Lair
 *                 Andrew Bazan
 *                 Ted Stodgell <kart@hal-pc.org>
 */

class VisionApp * vision_app;

/*
  -- #beos was here --
  <Brazilian> And then I says to the gnu, "Is that a horn on your head or
              are you just happy to see me?"
*/

#include <Alert.h>
#include <Resources.h>
#include <Font.h>
#include <Autolock.h>
#include <Roster.h>
#include <NetworkKit.h>

#include <algorithm>
#include <stdio.h>
#include <ctype.h>

#ifdef NETSERVER_BUILD
#  include <netdb.h>
#endif

#ifdef BONE_BUILD
#  include <arpa/inet.h>
#endif

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
  numBench = false;
  ShuttingDown = false;
  
  // :TODO: wade 013101 move colors to settings class
#if 1
// default Vision color scheme
  const rgb_color myBlack             = {0,0,0, 255};
  const rgb_color myWhite             = {255, 255, 255, 255};
  const rgb_color NOTICE_COLOR        = {10,90,170, 255};
  const rgb_color ACTION_COLOR        = {128,0,128, 255};
  const rgb_color QUIT_COLOR          = {180,10,10, 255};
  const rgb_color ERROR_COLOR         = {210,5,5, 255};
  const rgb_color URL_COLOR           = {5,5,150, 255};
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
  const rgb_color WINLIST_PAGE6_COLOR = {100, 100, 100, 255};
  const rgb_color WINLIST_SEL_COLOR   = ui_color (B_PANEL_BACKGROUND_COLOR);
  const rgb_color WALLOPS_COLOR       = {10,30,170, 255};
  const rgb_color NICK_DISPLAY        = {47, 47, 47, 255};

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
  colors[C_HELPER]                    = OP_COLOR;
  colors[C_VOICE]                     = VOICE_COLOR;
  colors[C_CTCP_REQ]                  = CTCP_REQ_COLOR;
  colors[C_CTCP_RPY]                  = CTCP_RPY_COLOR;
  colors[C_IGNORE]                    = IGNORE_COLOR;
  colors[C_INPUT]                     = INPUT_COLOR;
  colors[C_INPUT_BACKGROUND]          = INPUT_BG_COLOR;
  colors[C_WINLIST_BACKGROUND]        = WINLIST_BG_COLOR;
  colors[C_WINLIST_NORMAL]            = myBlack;
  colors[C_WINLIST_NEWS]              = JOIN_COLOR;
  colors[C_WINLIST_NICK]              = QUIT_COLOR;
  colors[C_WINLIST_SELECTION]         = WINLIST_SEL_COLOR;
  colors[C_WINLIST_PAGESIX]           = WINLIST_PAGE6_COLOR;
  colors[C_WALLOPS]                   = WALLOPS_COLOR;
  colors[C_NICKDISPLAY]               = NICK_DISPLAY;
#endif

#if 0
  // alternate color scheme <kart@hal-pc.org>
  const rgb_color myBlack             = {32,   32,  64, 255};
  const rgb_color myWhite             = {224, 192, 128, 255};
  const rgb_color NOTICE_COLOR        = {102, 215, 215, 255};
  const rgb_color ACTION_COLOR        = {101, 225, 116, 255};
  const rgb_color QUIT_COLOR          = {224,  96,  64, 255};
  const rgb_color ERROR_COLOR         = {255,  92,   0, 255};
  const rgb_color URL_COLOR           = {123, 150, 255, 255};
  const rgb_color NICK_COLOR          = {102, 215, 215, 255}; // for angle brackets around nick
  const rgb_color MYNICK_COLOR        = {122, 255, 237, 255}; // when anyone says your name
  const rgb_color JOIN_COLOR          = {192, 255, 192, 255};
  const rgb_color KICK_COLOR          = {255, 213,  88, 255};
  const rgb_color WHOIS_COLOR         = {180, 224, 221, 255};
  const rgb_color OP_COLOR            = {255,  64, 128, 255};
  const rgb_color VOICE_COLOR         = {255, 213,  88, 255};
  const rgb_color CTCP_REQ_COLOR      = {213, 180, 224, 255};
  const rgb_color CTCP_RPY_COLOR      = {180, 191, 224, 255};
  const rgb_color IGNORE_COLOR        = {106, 106, 173, 255};
  const rgb_color INPUT_COLOR         = {222, 255, 222, 255};
  const rgb_color INPUT_BG_COLOR      = {  0,   0,   0, 255};
  const rgb_color WINLIST_PAGE6_COLOR = {192, 192, 255, 255};
  const rgb_color WINLIST_BG_COLOR    = {32,   32,  64, 255};
  const rgb_color WINLIST_SEL_COLOR   = { 64,  92,  64, 255};  
  const rgb_color WALLOPS_COLOR       = {255, 135,  87, 255};
  const rgb_color NICK_DISPLAY        = {224, 207, 128, 255};


  colors[C_TEXT]                      = myWhite;   // this stuff is important
  colors[C_BACKGROUND]                = myBlack;   // pay attention to myBlack and myWhite
  colors[C_NAMES]                     = myWhite;
  colors[C_NAMES_BACKGROUND]          = myBlack;
  colors[C_URL]                       = URL_COLOR;
  colors[C_SERVER]                    = myWhite;
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
  colors[C_HELPER]                    = OP_COLOR;
  colors[C_VOICE]                     = VOICE_COLOR;
  colors[C_CTCP_REQ]                  = CTCP_REQ_COLOR;
  colors[C_CTCP_RPY]                  = CTCP_RPY_COLOR;
  colors[C_IGNORE]                    = IGNORE_COLOR;
  colors[C_INPUT]                     = INPUT_COLOR;
  colors[C_INPUT_BACKGROUND]          = INPUT_BG_COLOR;
  colors[C_WINLIST_BACKGROUND]        = WINLIST_BG_COLOR;
  colors[C_WINLIST_NORMAL]            = myWhite;
  colors[C_WINLIST_NEWS]              = JOIN_COLOR;
  colors[C_WINLIST_NICK]              = QUIT_COLOR;
  colors[C_WINLIST_SELECTION]         = WINLIST_SEL_COLOR;
  colors[C_WINLIST_PAGESIX]           = WINLIST_PAGE6_COLOR;
  colors[C_WALLOPS]                   = WALLOPS_COLOR;
  colors[C_NICKDISPLAY]               = NICK_DISPLAY;
#endif

  client_font[F_TEXT]    = new BFont (be_fixed_font);
  client_font[F_SERVER]  = new BFont (be_fixed_font);
  client_font[F_URL]     = new BFont (be_fixed_font);
  client_font[F_NAMES]   = new BFont (be_plain_font);
  client_font[F_INPUT]   = new BFont (be_fixed_font);
  client_font[F_WINLIST] = new BFont (be_plain_font);
  client_font[F_LISTAGENT] = new BFont (be_plain_font);
  
  events[E_JOIN]            = "*** $N ($I@$A) has joined the channel.";
  events[E_PART]            = "*** $N has left the channel.";
  events[E_NICK]            = "*** $N is now known as $n.";
  events[E_QUIT]            = "*** $N ($I@$A) has quit IRC ($R)";
  events[E_KICK]            = "*** $N has been kicked from $C by $n ($R)";
  events[E_TOPIC]           = "*** $C Topic changed by $N: $T";
  events[E_SNOTICE]         = "-$N- $R";
  events[E_UNOTICE]         = "-$N- $R";
  events[E_NOTIFY_ON]       = "*** $N has joined IRC.";
  events[E_NOTIFY_OFF]      = "*** $N has left IRC.";

  commands[CMD_KICK]        = "Ouch!";
  commands[CMD_QUIT]        = "Vision[$V]: i've been blurred!";
  commands[CMD_IGNORE]      = "*** $N is now ignored ($i).";
  commands[CMD_UNIGNORE]    = "*** $N is no longer ignored.";
  commands[CMD_AWAY]        = "is idle: $R";
  commands[CMD_BACK]        = "has returned";
  commands[CMD_UPTIME]      = "OS Uptime [BeOS]: $U";
  
  identThread = spawn_thread (Identity, "the_spirits_within", B_LOW_PRIORITY, NULL);
  if (identThread >= B_OK)
    resume_thread (identThread);
}

void
VisionApp::ThreadStates (void)
{
  thread_id team (Team());
  int32 cookie (0);
  thread_info info;

  BString buffer;
  int32 t_count (0);

  while (get_next_thread_info (team, &cookie, &info) == B_NO_ERROR)
  {
    buffer += "thread: ";
    buffer += info.thread;
    buffer += " name:  ";
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
    ++t_count;
  }

  if (buffer.Length())
  {
    BAlert *alert (new BAlert (
      "Too many threads",
      buffer.String(),
      t_count > 1 ? "Damn" : "Cool",
      0,
      0,
      B_WIDTH_AS_USUAL,
      t_count > 1 ? B_STOP_ALERT : B_INFO_ALERT));
      alert->Go();
  }
    
}

void
VisionApp::InitSettings (void)
{
  if (debugsettings)
    printf (":SETTINGS: loading...\n");
      
  visionSettings = new SettingsFile ("VisionSettings", "Vision");
    
  if (visionSettings->InitCheck() == B_OK)
    visionSettings->Load();
  else
    printf(":ERROR: Error Loading /Vision/VisionSettings\n");
      
  LoadDefaults (SET_SERVER);
  LoadDefaults (SET_GENERAL);
  LoadDefaults (SET_WINDOW);
  LoadDefaults (SET_NOTIFY);
  
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
      }
      break;
    
    case SET_GENERAL:
      {
        if (!visionSettings->HasBool ("versionParanoid"))
          visionSettings->AddBool ("versionParanoid", false);

        if (!visionSettings->HasBool ("timestamp"))
          visionSettings->AddBool ("timestamp", false);
          
        if (!visionSettings->HasString ("timestamp_format"))
          visionSettings->AddString ("timestamp_format", "[%02d:%02d]");
        
        if (!visionSettings->HasBool ("log_enabled"))
          visionSettings->AddBool ("log_enabled", false);

        if (!visionSettings->HasBool ("log_filetimestamp"))
          visionSettings->AddBool ("log_filetimestamp", false);
          
      }
      break;
    
    case SET_WINDOW:
      {
        if (!visionSettings->HasBool ("catchAltW"))
          visionSettings->AddBool ("catchAltW", false);

        if (!visionSettings->HasRect ("clientWinRect"))
          visionSettings->AddRect ("clientWinRect", BRect (100, 100, 600, 460));
      }
      break;

    case SET_NOTIFY:
      {
        if (!visionSettings->HasString ("alsoKnownAs"))
          visionSettings->AddString ("alsoKnownAs", "-9y99");
      }
      break;
  }
}

bool
VisionApp::QuitRequested (void)
{
  ShuttingDown = true;
  
  BMessage *quitRequest (CurrentMessage());

  if ((clientWin) && (!quitRequest->HasBool ("real_thing")))
  {
    clientWin->PostMessage (B_QUIT_REQUESTED);
    return false;
  }

  if (settingsloaded)
    if ((visionSettings->Save() == B_OK) && debugsettings)
      printf (":SETTINGS: saved to file\n");

  // give our child threads a chance to die gracefully
  snooze (500000);  // 0.5 seconds
  
  status_t result;

  wait_for_thread (identThread, &result);
  //ThreadStates();
  
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
      numBench = true;
    }
    
    else if (strcmp (av[i], "-r") == 0)
      debugrecv = true;

    else if (strcmp (av[i], "-s") == 0)
      debugsend = true;
        
    else if (strcmp (av[i], "-S") == 0)
      debugsettings = true;
      
    else if (strcmp (av[i], "-n") == 0)
      numBench = true;
    
    else if (strcmp (av[i], "-T") == 0)
    {
      TestScript *tscript = new TestScript();
      delete tscript;
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
      printf ("\n");
      if (IsLaunching())
        Quit();
    }
         
  }
}


void
VisionApp::ReadyToRun (void)
{
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
        if (ShuttingDown)
          PostMessage (B_QUIT_REQUESTED);
      }    
      break;
    
    case M_SETUP_SHOW:
      {
        if (setupWin)
          setupWin->Activate();
        else
        {
          setupWin = new SetupWindow(false);
          setupWin->Show();
        }
      }      
      break;
    
    case M_SETUP_CLOSE:
      {
        setupWin = 0;
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
  return clientWin;
}

BString
VisionApp::VisionVersion (int typebit)
{
  switch (typebit)
  {
    case VERSION_VERSION:
      {
        static BString version_version (VERSION_STRING);
        return version_version;
      }
      break;
      
    case VERSION_DATE:
      {
        static BString version_builddate (BUILD_DATE);
        version_builddate.ReplaceAll ("_", " ");
        return version_builddate;
      }
      break;
  }
  
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
    ? client_font[which] : be_plain_font;
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
VisionApp::GetThreadName (int thread_type)
{
  // random names for the connection thread
  static BString tnames[] = {
    /*  0 */ "gummi_bear_orgy",
    /*  1 */ "complimentary_tote_bag",
    /*  2 */ "cheating_at_solitaire", 
    /*  3 */ "impatient",
    /*  4 */ "personal_info_uploader",
    /*  5 */ "keystroke_logger",
    /*  6 */ "rc5_cracker",
    /*  7 */ "seti_at_home",
    /*  8 */ "stare_girl",
    /*  9 */ "baron_arnold",
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
    /* 22 */ "blossom",   // commander and the leader
    /* 23 */ "bubbles",   // the joy and the laughter 
    /* 24 */ "buttercup", // shes the toughest fighter
                          // Powerpuffs save the day!
    /* 25 */ "youlooklikeyouneedamonkey",
    /* 26 */ "wegotdeathstar",    // we got def star! we got def star!
    /* 27 */ "whatcha_gonna_do",  // uh oh!
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
    /* 52 */ "daydreaming"
  };
  
  int rnd (rand() % 53);
 
  static BString output;
  
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
  
  return output.String();
}

void
VisionApp::BenchOut (const char *ts)
{
  // note: this doesn't waste time figuring out whole seconds
  // output won't look right with more than 0.999s differences.

  int32 bench0;
  bench0 = bench2 - bench1;
  bench0 = bench0 / 100;
  
  printf ("%s: 0.%04lds\n", ts, bench0);
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
//  Lock();
//
//  for (int32 i = 0; i < CountWindows(); ++i)
//  {
//    ServerWindow *serverWindow (dynamic_cast<ServerWindow *>(WindowAt (i)));
//
//    if (serverWindow
//    &&  serverWindow->Id().ICompare (serverName) == 0)
//      if (active)
//        serverWindow->PostActive (msg);
//      else
//        serverWindow->RepliedBroadcast (msg);
//  }
//
//  Unlock();
}

int32 
VisionApp::Identity (void *) 
{ 
  BNetEndpoint identPoint, *accepted; 
  BNetBuffer buffer; 
  const char *ident (NULL); 
  char received[64]; 
      
  if (identPoint.InitCheck()    == B_OK 
  &&  identPoint.Bind (113)     == B_OK) 
  { 
    identPoint.Listen (2048);
    while (!vision_app->ShuttingDown) 
    {
      accepted = identPoint.Accept (30 * 1000);
      
      if (accepted) 
      {
        BNetAddress remoteAddr (accepted->RemoteAddr()); 
        struct sockaddr_in remoteSock; 
        remoteAddr.GetAddr (remoteSock); 
        BString remoteIP (inet_ntoa (remoteSock.sin_addr)); 
        ident = vision_app->GetIdent (remoteIP.String()); 
        if (ident) 
        {
          accepted->SetTimeout(5);

          if (accepted->Receive (buffer, 64) > 0)
          {
            buffer.RemoveString (received, 64); 
            int32 len; 
 
            received[63] = 0; 
            while ((len = strlen (received)) 
            &&     isspace (received[len - 1])) 
              received[len - 1] = 0; 
 
            BNetBuffer output; 
            BString string; 
            
            string.Append (received); 
            string.Append (" : USERID : BeOS : "); 
            string.Append (ident); 
            string.Append ("\r\n"); 
                
            output.AppendString (string.String()); 
            accepted->Send (output); 
          }
        } 
        else 
        { 
          BNetBuffer output; 
          output.AppendString ("0 , 0 : UNKNOWN : UNKNOWN-ERROR"); 
          accepted->Send (output); 
        } 
              
        accepted->Close(); 
        delete accepted; 
        if (ident) 
          ident = NULL; 
      } 
    }
  }
  identPoint.Close();
  return 0; 

} 
 
void 
VisionApp::AddIdent (const char *server, const char *serverIdent) 
{ 
  identLock.Lock(); 
  idents.AddString (server, serverIdent); 
  identLock.Unlock();
} 
 
void 
VisionApp::RemoveIdent (const char *server) 
{ 
  identLock.Lock(); 
  idents.RemoveName (server); 
  identLock.Unlock(); 
} 
 
const char * 
VisionApp::GetIdent (const char *server) 
{ 
  const char *ident (NULL); 
  identLock.Lock(); 
  if (idents.HasString (server)) 
    ident = idents.FindString (server); 
  identLock.Unlock();
  
  return ident; 
} 

//////////////////////////////////////////////////////////////////////////////
/// End Public Functions
//////////////////////////////////////////////////////////////////////////////

