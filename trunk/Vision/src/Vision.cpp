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
#include <MenuItem.h>
#include <Autolock.h>
#include <Roster.h>
#include <Beep.h>
#include <UTF8.h>

#include <algorithm>
#include <stdio.h>
#include <ctype.h>

#ifdef NETSERVER_BUILD
#  include <netdb.h>
#  include <socket.h>
#endif

#ifdef BONE_BUILD
#  include <arpa/inet.h>
#  include <sys/socket.h>
#endif

#include "AboutWindow.h"
#include "Vision.h"
#include "ClientWindow.h"
#include "DCCConnect.h"
#include "DCCFileWindow.h"
#include "NetworkWindow.h"
#include "SettingsFile.h"
#include "SetupWindow.h"
#include "PrefsWindow.h"
#include "Theme.h"
#include "WindowList.h"
#include "TestScript.h"

// sound event name definitions
const char *kSoundEventNames[] = { "Vision Nick Notification", 0 };

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
      fAboutWin (0),
      fSetupWin (0),
      fClientWin (0),
      fPrefsWin (0),
      fNetWin (0),
      fDccFileWin (0),
      fIdentThread (-1),
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
  fStartupTime = system_time();
}

VisionApp::~VisionApp (void)
{
  SaveSettings();
  delete fVisionSettings;
  int32 i (0);
  for (; i < MAX_FONTS; i++)
    delete fClientFont[i];

  delete fActiveTheme;
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
VisionApp::InitDefaults (void)
{
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
  const rgb_color MIRC_BLUE           = { 0, 0, 127, 255};
  const rgb_color MIRC_GREEN          = { 0, 127, 0, 255};
  const rgb_color MIRC_RED            = { 127, 0, 0, 255};
  const rgb_color MIRC_BROWN          = { 224, 192, 128, 255};
  const rgb_color MIRC_PURPLE         = { 127, 0, 127, 255};
  const rgb_color MIRC_ORANGE         = { 192, 127, 0, 255};
  const rgb_color MIRC_YELLOW         = { 255, 255, 0, 255};
  const rgb_color MIRC_LIME           = { 0, 255, 0, 255};
  const rgb_color MIRC_TEAL           = { 0, 127, 127, 255};
  const rgb_color MIRC_AQUA           = { 0, 255, 255, 255};
  const rgb_color MIRC_LT_BLUE        = { 0, 0, 255, 255};
  const rgb_color MIRC_PINK           = { 255, 127, 127, 255};
  const rgb_color MIRC_GREY           = { 127, 127, 127, 255};
  const rgb_color MIRC_SILVER         = { 192, 192, 192, 255};

  fColors[C_TEXT]                      = myBlack;
  fColors[C_BACKGROUND]                = myWhite;
  fColors[C_NAMES]                     = myBlack;
  fColors[C_NAMES_BACKGROUND]          = myWhite;
  fColors[C_NAMES_SELECTION]           = WINLIST_SEL_COLOR;
  fColors[C_URL]                       = URL_COLOR;
  fColors[C_SERVER]                    = myBlack;
  fColors[C_NOTICE]                    = NOTICE_COLOR;
  fColors[C_ACTION]                    = ACTION_COLOR;
  fColors[C_QUIT]                      = QUIT_COLOR;
  fColors[C_ERROR]                     = ERROR_COLOR;
  fColors[C_NICK]                      = NICK_COLOR;
  fColors[C_MYNICK]                    = MYNICK_COLOR;
  fColors[C_JOIN]                      = JOIN_COLOR;
  fColors[C_KICK]                      = KICK_COLOR;
  fColors[C_WHOIS]                     = WHOIS_COLOR;
  fColors[C_OP]                        = OP_COLOR;
  fColors[C_HELPER]                    = OP_COLOR;
  fColors[C_VOICE]                     = VOICE_COLOR;
  fColors[C_CTCP_REQ]                  = CTCP_REQ_COLOR;
  fColors[C_CTCP_RPY]                  = CTCP_RPY_COLOR;
  fColors[C_IGNORE]                    = IGNORE_COLOR;
  fColors[C_INPUT]                     = INPUT_COLOR;
  fColors[C_INPUT_BACKGROUND]          = INPUT_BG_COLOR;
  fColors[C_WINLIST_BACKGROUND]        = WINLIST_BG_COLOR;
  fColors[C_WINLIST_NORMAL]            = myBlack;
  fColors[C_WINLIST_NEWS]              = JOIN_COLOR;
  fColors[C_WINLIST_NICK]              = QUIT_COLOR;
  fColors[C_WINLIST_SELECTION]         = WINLIST_SEL_COLOR;
  fColors[C_WINLIST_PAGESIX]           = WINLIST_PAGE6_COLOR;
  fColors[C_WALLOPS]                   = WALLOPS_COLOR;
  fColors[C_NICKDISPLAY]               = NICK_DISPLAY;
  fColors[C_TIMESTAMP]                 = myBlack;
  fColors[C_TIMESTAMP_BACKGROUND]      = myWhite;
  fColors[C_SELECTION]                 = myBlack;
  fColors[C_MIRC_WHITE]                = myWhite;
  fColors[C_MIRC_BLACK]                = myBlack;
  fColors[C_MIRC_BLUE]                 = MIRC_BLUE;
  fColors[C_MIRC_GREEN]                = MIRC_GREEN;
  fColors[C_MIRC_RED]                  = MIRC_RED;
  fColors[C_MIRC_BROWN]                = MIRC_BROWN;
  fColors[C_MIRC_PURPLE]               = MIRC_PURPLE;
  fColors[C_MIRC_ORANGE]               = MIRC_ORANGE;
  fColors[C_MIRC_YELLOW]               = MIRC_YELLOW;
  fColors[C_MIRC_LIME]                 = MIRC_LIME;
  fColors[C_MIRC_TEAL]                 = MIRC_TEAL;
  fColors[C_MIRC_AQUA]                 = MIRC_AQUA;
  fColors[C_MIRC_LT_BLUE]              = MIRC_LT_BLUE;
  fColors[C_MIRC_PINK]                 = MIRC_PINK;
  fColors[C_MIRC_GREY]                 = MIRC_GREY;
  fColors[C_MIRC_SILVER]               = MIRC_SILVER;
  fColors[C_NOTIFY_ON]                 = JOIN_COLOR;
  fColors[C_NOTIFY_OFF]                = myBlack;
  fColors[C_NOTIFYLIST_BACKGROUND]     = WINLIST_BG_COLOR;
  fColors[C_NOTIFYLIST_SELECTION]      = WINLIST_SEL_COLOR;
  
  fClientFont[F_TEXT]    = new BFont (be_plain_font);
  fClientFont[F_SERVER]  = new BFont (be_plain_font);
  fClientFont[F_URL]     = new BFont (be_plain_font);
  fClientFont[F_NAMES]   = new BFont (be_plain_font);
  fClientFont[F_INPUT]   = new BFont (be_plain_font);
  fClientFont[F_WINLIST] = new BFont (be_plain_font);
  fClientFont[F_LISTAGENT] = new BFont (be_plain_font);
  fClientFont[F_TIMESTAMP] = new BFont (be_plain_font);
  
  fEvents[E_JOIN]            = "*** $N ($I@$A) has joined the channel.";
  fEvents[E_PART]            = "*** $N has left the channel.";
  fEvents[E_NICK]            = "*** $N is now known as $n.";
  fEvents[E_QUIT]            = "*** $N ($I@$A) has quit IRC ($R)";
  fEvents[E_KICK]            = "*** $N has been kicked from $C by $n ($R)";
  fEvents[E_TOPIC]           = "*** $C Topic changed by $N: $T";
  fEvents[E_SNOTICE]         = "-$N- $R";
  fEvents[E_UNOTICE]         = "-$N- $R";
  fEvents[E_NOTIFY_ON]       = "*** $N has joined IRC.";
  fEvents[E_NOTIFY_OFF]      = "*** $N has left IRC.";

  fCommands[CMD_KICK]        = "Ouch!";
  fCommands[CMD_QUIT]        = "Vision[$V]: i've been blurred!";
  fCommands[CMD_IGNORE]      = "*** $N is now ignored ($i).";
  fCommands[CMD_UNIGNORE]    = "*** $N is no longer ignored.";
  fCommands[CMD_AWAY]        = "is idle: $R";
  fCommands[CMD_BACK]        = "has returned";
  fCommands[CMD_UPTIME]      = "OS Uptime [BeOS]: $U";
  
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
    fVisionSettings->Load();
  else
    printf(":ERROR: Error Loading /Vision/VisionSettings\n");

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
        
        if (!fVisionSettings->HasString ("logBaseDir"))
          fVisionSettings->AddString("logBaseDir", "logs");
        
        if (!fVisionSettings->HasInt32 ("encoding"))
          fVisionSettings->AddInt32("encoding", B_ISO1_CONVERSION);
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
       float  size (0.0);
       be_plain_font->GetFamilyAndStyle (&default_family, &default_style);
       size = be_plain_font->Size();
       int32 i (0);
       
       for (i = 0; i < MAX_FONTS; i++)
       {
         if (!fVisionSettings->HasString ("family", i))
         {
           fVisionSettings->AddString ("family", default_family);
           fVisionSettings->AddString ("style", default_style);
           fVisionSettings->AddFloat ("size", size);
         }
         else
         {
           BString family;
           BString style;
           size = 0.0;
                  
           fVisionSettings->FindString ("family", i, &family);
           fVisionSettings->FindString ("style", i, &style);
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
       // fEvents
       if (!fVisionSettings->HasString ("event"))
       {
         for (int32 i = 0; i < MAX_EVENTS; i++)
           fVisionSettings->AddString ("event", fEvents[i]);
       }
       else
       {
         for (int32 i = 0; i < MAX_EVENTS; i++)
           fVisionSettings->FindString ("event", i, &fEvents[i]);
       }
       
       // fCommands
       if (!fVisionSettings->HasString ("command"))
       {
         for (int32 i = 0; i < MAX_COMMANDS; i++)
           fVisionSettings->AddString ("command", fCommands[i]);
       }
       else
       {
         for (int32 i = 0; i < MAX_COMMANDS; i++)
           fVisionSettings->FindString ("command", i, &fCommands[i]);
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
#ifdef BONE_BUILD
    close (fIdentSocket);
#elif NETSERVER_BUILD
    closesocket (fIdentSocket);
#endif
  
  BMessenger msgr(fClientWin);
  if (msgr.IsValid())
    msgr.SendMessage(B_QUIT_REQUESTED);
  
  status_t result;
  wait_for_thread (fWinThread, &result);

  // give our child threads a chance to die gracefully
  snooze (500000);  // 0.5 seconds

  //ThreadStates();
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
    }
    
    else if (strcmp (av[i], "-r") == 0)
      fDebugRecv = true;

    else if (strcmp (av[i], "-s") == 0)
      fDebugSend = true;
        
    else if (strcmp (av[i], "-S") == 0)
      fDebugSettings = true;
      
    else if (strcmp (av[i], "-n") == 0)
      fNumBench = true;
    
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


// check if any networks have the connect on startup flag marked...
// if they do, start them

bool
VisionApp::CheckStartupNetworks (void)
{
  bool autoStarted (false);
#if 1
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
#endif
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
      return true;
  return false;
}

void
VisionApp::ReadyToRun (void)
{
  InitSettings();

  fIdentThread = spawn_thread (Identity, "the_spirits_within", B_LOW_PRIORITY, NULL);
  if (fIdentThread >= B_OK)
    resume_thread (fIdentThread);

  if (!CheckStartupNetworks())
  {  
    fSetupWin = new SetupWindow ();
    fSetupWin->Show();
  }
}

void
VisionApp::LoadURL (const char *url)
{
  BString argument (url);
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

    // todo: Should probably see if the file exists before going through
    //       all this, but, oh well... ;)
	file.Prepend("/boot/beos/system/Tracker ");
	file += " &";									// just in case

    system(file.String());
  }
  else if (argument.IFindFirst ("mailto:") == 0)
  {
  	be_roster->Launch ("text/x-email", 1, const_cast<char **>(args));
  }
  else
  {
  	be_roster->Launch ("text/html", 1, const_cast<char **>(args));
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
  
  if (!stringLock.IsLocked())
    return "";
  
  if (fDebugSettings)
    printf (":SETTINGS: looking up String \"%s\"... ", stringName);
    
  const char *value;
  
  if (fVisionSettings->FindString (stringName, &value) == B_OK)
    if (fDebugSettings)
      printf ("found; returning %s\n", value);
  else
    if (fDebugSettings)
      printf (" not found; returning NULL\n");
      
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
  
  fVisionSettings->ReplaceString (stringName, index, value);
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
    
  if (which < MAX_COLORS &&  which >= 0
  && (fColors[which].red   != color.red
  ||  fColors[which].green != color.green
  ||  fColors[which].blue  != color.blue
  ||  fColors[which].alpha != color.alpha))
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
    fClientFont[which]->SetFamilyAndStyle (family, style);

    fActiveTheme->WriteLock();
    fActiveTheme->SetFont (which, fClientFont[which]);
    if (which == F_TEXT)
      fActiveTheme->SetFont (MAX_FONTS, fClientFont[which]);
    fActiveTheme->WriteUnlock();
    
    SetString ("family", which, family);
    SetString ("style", which, style);
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
  
  if (!eventLock.IsLocked())
    return "";
  
  BString buffer;

  if (which < MAX_EVENTS && which >= 0)
    buffer = fEvents[which];

  return buffer;
}


void
VisionApp::SetEvent (int32 which, const char *event)
{
  BAutolock eventLock (const_cast<BLocker *>(&fSettingsLock));
  
  if (!eventLock.IsLocked())
    return;

  if (which < MAX_EVENTS && which >= 0
  &&  fEvents[which].Compare (event))
  {
    fEvents[which] = event;

    fVisionSettings->ReplaceString ("event", which, event);
  }
}


BString
VisionApp::GetCommand (int32 which)
{
  BAutolock commandLock (const_cast<BLocker *>(&fSettingsLock));
  
  if (!commandLock.IsLocked())
    return "";

  BString buffer;

  if (which < MAX_COMMANDS && which >= 0)
    buffer = fCommands[which];

  return buffer;
}


void
VisionApp::SetCommand (int32 which, const char *command)
{
  BAutolock commandLock (const_cast<BLocker *>(&fSettingsLock));
  
  if (!commandLock.IsLocked())
    return;

  if (which < MAX_EVENTS && which >= 0)
  {
    fCommands[which] = command;
    
    fVisionSettings->ReplaceString ("command", which, command);
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
    if (fDebugSettings)
      printf ("found; returning %s\n", (value)? "true" : "false");
  else
    if (fDebugSettings)
      printf (" not found; returning false\n");
      
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
  
  printf ("%s: 0.%04lds\n", ts, bench0);
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
//  TODO: implement or remove this
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
  int32 identSock (0), accepted (0);
  BString ident; 
  char received[64];
 
  struct sockaddr_in localAddr;
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons (113);
  localAddr.sin_addr.s_addr = INADDR_ANY;
      
  if ((identSock = socket (AF_INET, SOCK_STREAM, 0)) >= 0 
  &&  bind (identSock, (struct sockaddr *)&localAddr, sizeof (localAddr)) == 0) 
  {
      vision_app->fIdentSocket = identSock;

#ifdef BONE_BUILD
    struct linger lng = { 0, 0 }; 
    setsockopt (identSock, SOL_SOCKET, SO_LINGER, &lng, sizeof (linger));
#endif
    listen (identSock, 1);

    while (!vision_app->fShuttingDown) 
    {
      struct fd_set rset, eset;
      struct sockaddr_in remoteSock;
      int size (sizeof (sockaddr_in));
      struct timeval tv = { 10, 0};
      FD_ZERO (&rset);
      FD_ZERO (&eset);
      FD_SET (identSock, &rset);
      FD_SET (identSock, &eset);

      if (select (identSock + 1, &rset, 0, &eset, NULL) < 0 || FD_ISSET (identSock, &eset))
         break;
      else if (FD_ISSET (identSock, &rset))
      {
        accepted = accept (identSock, (struct sockaddr *)&remoteSock, &size);
        if (accepted >= 0)
        {
          FD_ZERO (&rset);
          FD_ZERO (&eset);
        
          BString remoteIP (inet_ntoa (remoteSock.sin_addr));
          ident = vision_app->GetIdent (remoteIP.String());

          if (ident.Length() > 0) 
          {
            memset (received, 0, 64);
            FD_SET (accepted, &rset);
            FD_SET (accepted, &eset);
            if (select (accepted + 1, &rset, 0, &eset, &tv) > 0
              && FD_ISSET (accepted, &rset) && !FD_ISSET (accepted, &eset))
            {
            
              recv (accepted, received, 64, 0);
              int32 len (0); 
     
              received[63] = 0; 
              while ((len = strlen (received)) 
              &&     isspace (received[len - 1])) 
                received[len - 1] = 0;
              
              BString string; 
              
              string.Append (received); 
              string.Append (" : USERID : BeOS : "); 
              string.Append (ident); 
              string.Append ("\r\n"); 
                
              send (accepted, string.String(), string.Length(), 0); 
            }
          } 
          else 
          { 
            BString string ("0 , 0 : UNKNOWN : UNKNOWN-ERROR");
            send (accepted, string.String(), string.Length(), 0);
          } 
#ifdef NETSERVER_BUILD
          closesocket (accepted);
#elif BONE_BUILD
          close (accepted);
#endif              
        }
      }
    }
  }

#ifdef BONE_BUILD
  close (identSock);
#elif NETSERVER_BUILD
  closesocket (identSock);
#endif
  return 0; 
} 
 
void 
VisionApp::AddIdent (const char *server, const char *serverIdent) 
{ 
  fIdentLock.Lock(); 
  fIdents.AddString (server, serverIdent); 
  fIdentLock.Unlock();
} 
 
void 
VisionApp::RemoveIdent (const char *server) 
{ 
  fIdentLock.Lock(); 
  fIdents.RemoveName (server); 
  fIdentLock.Unlock(); 
} 
 
BString 
VisionApp::GetIdent (const char *server)
{
  BString ident;
  fIdentLock.Lock(); 
  if (fIdents.HasString (server)) 
    ident = fIdents.FindString (server); 
  fIdentLock.Unlock();
  
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

//////////////////////////////////////////////////////////////////////////////
/// End Public Functions
//////////////////////////////////////////////////////////////////////////////

