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
#include "NetworkWindow.h"
#include "SettingsFile.h"
#include "SetupWindow.h"
#include "PrefsWindow.h"
#include "Theme.h"

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
      aboutWin (0),
      setupWin (0),
      clientWin (0),
      prefsWin (0),
      netWin (0),
      identSocket (-1),
      activeTheme (new Theme ("current", MAX_COLORS + 1, MAX_COLORS + 1, MAX_FONTS + 1))
{
  // some setup
  settingsloaded = false;
  debugrecv = false;
  debugsend = false;
  debugsettings = false;
  numBench = false;
  ShuttingDown = false;
  
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
  colors[C_TIMESTAMP]                 = myBlack;
  colors[C_TIMESTAMP_BACKGROUND]      = myWhite;
  colors[C_SELECTION]                 = myBlack;
  colors[C_MIRC_WHITE]                = myWhite;
  colors[C_MIRC_BLACK]                = myBlack;
  colors[C_MIRC_BLUE]                 = MIRC_BLUE;
  colors[C_MIRC_GREEN]                = MIRC_GREEN;
  colors[C_MIRC_RED]                  = MIRC_RED;
  colors[C_MIRC_BROWN]                = MIRC_BROWN;
  colors[C_MIRC_PURPLE]               = MIRC_PURPLE;
  colors[C_MIRC_ORANGE]               = MIRC_ORANGE;
  colors[C_MIRC_YELLOW]               = MIRC_YELLOW;
  colors[C_MIRC_LIME]                 = MIRC_LIME;
  colors[C_MIRC_TEAL]                 = MIRC_TEAL;
  colors[C_MIRC_AQUA]                 = MIRC_AQUA;
  colors[C_MIRC_LT_BLUE]              = MIRC_LT_BLUE;
  colors[C_MIRC_PINK]                 = MIRC_PINK;
  colors[C_MIRC_GREY]                 = MIRC_GREY;
  colors[C_MIRC_SILVER]               = MIRC_SILVER;
  
  client_font[F_TEXT]    = new BFont (be_plain_font);
  client_font[F_SERVER]  = new BFont (be_plain_font);
  client_font[F_URL]     = new BFont (be_plain_font);
  client_font[F_NAMES]   = new BFont (be_plain_font);
  client_font[F_INPUT]   = new BFont (be_plain_font);
  client_font[F_WINLIST] = new BFont (be_plain_font);
  client_font[F_LISTAGENT] = new BFont (be_plain_font);
  client_font[F_TIMESTAMP] = new BFont (be_plain_font);
  
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
  
  if (debugsettings)
    printf (":SETTINGS: loading...\n");
      
  visionSettings = new SettingsFile ("VisionSettings", "Vision");
    
  if (visionSettings->InitCheck() == B_OK)
    visionSettings->Load();
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

  // initialize theme, TODO: move to separate function

  for (i = 0; i < MAX_COLORS; i++)
  {
    activeTheme->SetForeground (i, colors[i]);
    activeTheme->SetBackground (i, colors[C_BACKGROUND]);
  }
  for (i = C_MIRC_WHITE; i < MAX_COLORS; i++)
    activeTheme->SetBackground (i, colors[i]);
   
  activeTheme->SetBackground (C_SELECTION, colors [C_SELECTION]); 
  activeTheme->SetBackground (C_TIMESTAMP, colors[C_TIMESTAMP_BACKGROUND]);
  activeTheme->SetBackground (MAX_COLORS, colors[C_BACKGROUND]);
  activeTheme->SetForeground (MAX_COLORS, colors[C_TEXT]);
  activeTheme->SetFont (MAX_FONTS, client_font[F_TEXT]);

  for (i = 0; i < MAX_FONTS; i++)
    activeTheme->SetFont (i, client_font[i]);

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
        if (!visionSettings->HasMessage ("defaults"))
        {
          BMessage defaults (VIS_NETWORK_DEFAULTS);
          defaults.AddString ("name", "defaults");
          defaults.AddString ("nick", "vision");
          defaults.AddString ("nick", "vision2");
          defaults.AddString ("ident", "vision");
          defaults.AddString ("realname", "Heisenberg may have slept here");
          visionSettings->AddMessage ("defaults", &defaults);
        }
        // remove this eventually, stupid bug
        else
        {
          BMessage defaults (GetNetwork ("defaults"));
          if (!defaults.HasString ("name"))
            defaults.AddString ("name", "defaults");
          SetNetwork ("defaults", &defaults);
        }

      }
      break;
    
    case SET_GENERAL:
      {
        if (!visionSettings->HasBool ("versionParanoid"))
          visionSettings->AddBool ("versionParanoid", false);

        if (!visionSettings->HasBool ("timestamp"))
          visionSettings->AddBool ("timestamp", false);
          
        if (!visionSettings->HasString ("timestamp_format"))
          visionSettings->AddString ("timestamp_format", "[%H:%M]");
        else
          visionSettings->ReplaceString ("timestamp_format", "[%H:%M]");
        
        if (!visionSettings->HasBool ("log_enabled"))
          visionSettings->AddBool ("log_enabled", false);

        if (!visionSettings->HasBool ("log_filetimestamp"))
          visionSettings->AddBool ("log_filetimestamp", false);
        
        if (!visionSettings->HasBool ("stripcolors"))
          visionSettings->AddBool ("stripcolors", true);
          
        if (!visionSettings->HasBool ("Newbie Spam Mode"))
          visionSettings->AddBool("Newbie Spam Mode", true);  
      }
      break;
    
    case SET_WINDOW:
      {
        if (!visionSettings->HasBool ("catchAltW"))
          visionSettings->AddBool ("catchAltW", false);

        if (!visionSettings->HasRect ("clientWinRect"))
          visionSettings->AddRect ("clientWinRect", BRect (100, 100, 600, 460));
        
        if (!visionSettings->HasRect ("windowDockRect"))
          visionSettings->AddRect ("windowDockRect", BRect (0, 0, 0, 0));
          
        if (!visionSettings->HasRect ("NetPrefWinRect"))
          visionSettings->AddRect ("NetPrefWinRect", BRect (0, 0, 0, 0));
        
        if (!visionSettings->HasRect ("namesListRect"))
          visionSettings->AddRect ("namesListRect", BRect (0, 0, 100, 0));
        
        if (!visionSettings->HasRect ("GenPrefWinRect"))
          visionSettings->AddRect ("GenPrefWinRect", BRect (0,0,0,0));
      }
      break;

    case SET_NOTIFY:
      {
        if (!visionSettings->HasString ("alsoKnownAs"))
          visionSettings->AddString ("alsoKnownAs", "-9y99");
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
         if (!visionSettings->HasString ("family", i))
         {
           visionSettings->AddString ("family", default_family);
           visionSettings->AddString ("style", default_style);
           visionSettings->AddFloat ("size", size);
         }
         else
         {
           BString family;
           BString style;
           size = 0.0;
                  
           visionSettings->FindString ("family", i, &family);
           visionSettings->FindString ("style", i, &style);
           visionSettings->FindFloat ("size", i, &size);
           
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
         if (!visionSettings->HasData ("color", B_RGB_COLOR_TYPE, i))
         {
           visionSettings->AddData ("color", B_RGB_COLOR_TYPE, &colors[i], sizeof (rgb_color));
         }
         else
         {
           const rgb_color *color;
           ssize_t size (0);
           if (visionSettings->FindData ("color", B_RGB_COLOR_TYPE, i, reinterpret_cast<const void **>(&color), &size) == B_OK)
             colors[i] = *color;
         }
     }
     break;
   
   case SET_STRINGS:
     {
       // events
       if (!visionSettings->HasString ("event"))
       {
         for (int32 i = 0; i < MAX_EVENTS; i++)
           visionSettings->AddString ("event", events[i]);
       }
       else
       {
         for (int32 i = 0; i < MAX_EVENTS; i++)
           visionSettings->FindString ("event", i, &events[i]);
       }
       
       // commands
       if (!visionSettings->HasString ("command"))
       {
         for (int32 i = 0; i < MAX_COMMANDS; i++)
           visionSettings->AddString ("command", commands[i]);
       }
       else
       {
         for (int32 i = 0; i < MAX_COMMANDS; i++)
           visionSettings->FindString ("command", i, &commands[i]);
       }
     }
     break;
  }
}

bool
VisionApp::QuitRequested (void)
{
  ShuttingDown = true;

  if (identSocket >= 0)
#ifdef BONE_BUILD
  close (identSocket);
#elif NETSERVER_BUILD
  closesocket (identSocket);
#endif

  if(clientWin)
  {
    clientWin->PostMessage (B_QUIT_REQUESTED);
    //hang out here until ClientWindow::QuitRequested is really done
    acquire_sem_etc(shutdownSem, 1, B_RELATIVE_TIMEOUT, 500000);
  }
  
  // give our child threads a chance to die gracefully
  snooze (500000);  // 0.5 seconds

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

  identThread = spawn_thread (Identity, "the_spirits_within", B_LOW_PRIORITY, NULL);
  if (identThread >= B_OK)
    resume_thread (identThread);

  shutdownSem = create_sem(0, "ShutdownSem");
  if(shutdownSem < B_OK) //that's an error
  	vision_app->Quit();
  
  if (!CheckStartupNetworks())
  {  
    setupWin = new SetupWindow ();
    setupWin->Show();
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
  
  be_roster->Launch ("text/html", 1, const_cast<char **>(args));
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
          setupWin = new SetupWindow ();
          setupWin->Show();
        }
      }
      break;

    case M_SETUP_CLOSE:
      {
        settingsLock.Lock();
        if ((visionSettings->Save() == B_OK) && debugsettings)
          printf (":SETTINGS: saved to file\n");
        settingsLock.Unlock();
        setupWin = 0;
        if (clientWin == NULL)
          PostMessage (B_QUIT_REQUESTED);
      }
      break;
    
      
    case M_PREFS_SHOW:
      {
        if (prefsWin)
          prefsWin->Activate();
        else
        {
          prefsWin = new PrefsWindow();
          prefsWin->Show();
        }
      }      
      break;
    
    case M_PREFS_CLOSE:
      {
        settingsLock.Lock();
        if ((visionSettings->Save() == B_OK) && debugsettings)
          printf (":SETTINGS: saved to file\n");
        settingsLock.Unlock();
        prefsWin = 0;
      }
      break;
    
    case M_NETWORK_SHOW:
      {
        if (netWin)
          netWin->Activate();
        else
        {
          netWin = new NetworkWindow();
          netWin->Show();
        }
      }      
      break;

    case M_NETWORK_CLOSE:
      {
        settingsLock.Lock();
        if ((visionSettings->Save() == B_OK) && debugsettings)
          printf (":SETTINGS: saved to file\n");
        settingsLock.Unlock();
        netWin = 0;
      }
      break;
    
    case M_CONNECT_NETWORK:
      {
        BRect clientWinRect;
        visionSettings->FindRect ("clientWinRect", &clientWinRect);

        BMessage netData = GetNetwork (msg->FindString ("network"));
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
        if (clientWin == NULL)
        {
          clientWin = new ClientWindow (clientWinRect);
          clientWin->Show();
        }
        BMessage connMsg (M_MAKE_NEW_NETWORK);
        connMsg.AddMessage ("network", &netData);
        clientWin->PostMessage (&connMsg);
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
  static BString versionReply;
  switch (typebit)
  {
    case VERSION_VERSION:
      versionReply = VERSION_STRING;
      break;
      
    case VERSION_DATE:
      versionReply = BUILD_DATE;
      versionReply.ReplaceAll ("_", " ");
      break;
  }
  return versionReply;
}

const char *
VisionApp::GetString (const char *stringName) const
{
  BAutolock stringLock (const_cast<BLocker *>(&settingsLock));
  
  if (!stringLock.IsLocked())
    return "";
  
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
VisionApp::SetString (const char *stringName, int32 index, const char *value)
{
  BAutolock stringLock (const_cast<BLocker *>(&settingsLock));
  
  if (!stringLock.IsLocked())
    return B_ERROR;
  
  if (visionSettings->ReplaceString (stringName, index, value) == B_OK)
    return B_OK;
  
  return B_ERROR;  
}

const BRect
VisionApp::GetRect (const char *settingName)
{
  BRect rect (0.0, 0.0, 0.0, 0.0);
  
  if (visionSettings->HasRect (settingName))
    visionSettings->FindRect (settingName, &rect);
  
  return rect;
}

status_t
VisionApp::SetRect (const char *settingName, BRect value)
{
 BAutolock rectLock(const_cast<BLocker *>(&settingsLock));
 
 
 if (!rectLock.IsLocked())
   return B_ERROR;
  
 if (visionSettings->ReplaceRect (settingName, value) == B_OK)
 {
   visionSettings->Save();
   return B_OK;
 }
 
 return B_ERROR;
}


rgb_color
VisionApp::GetColor (int32 which) const
{
  rgb_color color = {0, 0, 0, 255};

  BAutolock colorLock (const_cast<BLocker *>(&settingsLock));
  
  if (!colorLock.IsLocked())
    return color;
    
  
  if (which < MAX_COLORS && which >= 0)
    color = colors[which];

  return color;
}


void
VisionApp::SetColor (int32 which, const rgb_color color)
{

  BAutolock colorLock (const_cast<BLocker *>(&settingsLock));
  
  if (!colorLock.IsLocked())
    return;
    
  if (which < MAX_COLORS &&  which >= 0
  && (colors[which].red   != color.red
  ||  colors[which].green != color.green
  ||  colors[which].blue  != color.blue
  ||  colors[which].alpha != color.alpha))
  {
    colors[which] = color;
    visionSettings->ReplaceData ("color", B_RGB_COLOR_TYPE, which,
      reinterpret_cast<void * const *>(&color), sizeof(rgb_color));
    
    if (which == C_BACKGROUND)
    {
      // update regular background color on all other text
      for (int32 i = 0; i < C_TIMESTAMP; i++)
        activeTheme->SetBackground (i , color);
       activeTheme->SetBackground (MAX_COLORS, color);
    }
    
    // update timestamp bg color
    else if (which == C_TIMESTAMP_BACKGROUND)
      activeTheme->SetBackground (C_TIMESTAMP, color);
    else if (which == C_SELECTION)
      activeTheme->SetBackground (C_SELECTION, color);
    // mirc colors need to be updated on both fore and back
    else if (which >= C_MIRC_WHITE)
    {
      activeTheme->SetForeground (which, color);
      activeTheme->SetBackground (which, color);
    }
    else
      activeTheme->SetForeground (which, color);

    BMessage msg (M_STATE_CHANGE);

    msg.AddInt32 ("which", which);
    msg.AddBool ("color", true);
    Broadcast (&msg);
  }
}


/// begin font prefs ///
void
VisionApp::ClientFontFamilyAndStyle (
  int32 which,
  const char *family,
  const char *style)
{

  BAutolock fontLock (const_cast<BLocker *>(&settingsLock));
  
  if (!fontLock.IsLocked())
    return;
    
  if (which < MAX_FONTS && which >= 0)
  {
    activeTheme->SetFont (which, client_font[which]);
    
    if (which == F_TEXT)
      activeTheme->SetFont (MAX_FONTS, client_font[which]);
    BMessage msg (M_STATE_CHANGE);
    
    SetString ("family", which, family);
    SetString ("style", which, style);
    
    msg.AddBool ("font", true);
    msg.AddInt32 ("which", which);
    Broadcast (&msg);
  }
}


void
VisionApp::ClientFontSize (int32 which, float size)
{
  BAutolock fontLock (const_cast<BLocker *>(&settingsLock));
  
  if (!fontLock.IsLocked())
    return;

  if (which < MAX_FONTS && which >= 0)
  {
    client_font[which]->SetSize (size);
    BMessage msg (M_STATE_CHANGE);
    
    activeTheme->SetFont (which, client_font[which]);
    
    if (visionSettings->ReplaceFloat ("size", which, size) != B_OK)
      printf("error, could not set font size\n");

    msg.AddBool ("font", true);
    msg.AddInt32 ("which", which);
    Broadcast (&msg);
  }
}


const BFont *
VisionApp::GetClientFont (int32 which) const
{
  BAutolock fontLock (const_cast<BLocker *>(&settingsLock));
  
  if (!fontLock.IsLocked())
    return NULL;

  return which < MAX_FONTS && which >= 0 
    ? client_font[which] : be_plain_font;
}
/// end font prefs ///


BString
VisionApp::GetEvent (int32 which) const
{
  BAutolock eventLock (const_cast<BLocker *>(&settingsLock));
  
  if (!eventLock.IsLocked())
    return "";
  
  BString buffer;

  if (which < MAX_EVENTS && which >= 0)
    buffer = events[which];

  return buffer;
}


void
VisionApp::SetEvent (int32 which, const char *event)
{
  BAutolock eventLock (const_cast<BLocker *>(&settingsLock));
  
  if (!eventLock.IsLocked())
    return;

  if (which < MAX_EVENTS && which >= 0
  &&  events[which].Compare (event))
  {
    events[which] = event;

    visionSettings->ReplaceString ("event", which, event);
    
    BMessage msg (M_STATE_CHANGE);
    
    msg.AddBool ("event", true);
    msg.AddInt32 ("which", which);

    Broadcast (&msg);
  }
}


BString
VisionApp::GetCommand (int32 which)
{
  BAutolock commandLock (const_cast<BLocker *>(&settingsLock));
  
  if (!commandLock.IsLocked())
    return "";

  BString buffer;

  if (which < MAX_COMMANDS && which >= 0)
    buffer = commands[which];

  return buffer;
}


void
VisionApp::SetCommand (int32 which, const char *command)
{
  BAutolock commandLock (const_cast<BLocker *>(&settingsLock));
  
  if (!commandLock.IsLocked())
    return;

  if (which < MAX_EVENTS && which >= 0)
  {
    commands[which] = command;
    
    visionSettings->ReplaceString ("command", which, command);
    
    BMessage msg (M_STATE_CHANGE);
    
    msg.AddBool ("command", true);
    msg.AddInt32 ("which", which);
    
    Broadcast (&msg);

  }
}


bool
VisionApp::GetBool (const char *settingName)
{
  BAutolock boolLock (const_cast<BLocker *>(&settingsLock));
  
  if (!boolLock.IsLocked())
    return B_ERROR;

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
  BAutolock boolLock (const_cast<BLocker *>(&settingsLock));
  
  if (!boolLock.IsLocked())
    return B_ERROR;
  
 if (visionSettings->ReplaceBool (settingName, value) == B_OK)
 {
   BMessage msg (M_STATE_CHANGE);
   msg.AddBool ("bool", true);
   Broadcast (&msg);
   return B_OK;
 }
 return B_ERROR;
}

BMessage
VisionApp::GetNetwork (const char *network)
{
  BMessage msg (VIS_NETWORK_DATA);

  BAutolock netLock (const_cast<BLocker *>(&settingsLock));
  
  if (netLock.IsLocked())
  {
    if (!strcmp (network, "defaults"))
    {
      visionSettings->FindMessage ("defaults", &msg);
      return msg;
    }
    type_code type;
    int32 count (0);
    visionSettings->GetInfo ("network", &type, &count);
    for (int32 i = 0; i < count; i++)
    {
       BMessage tempMsg;
       visionSettings->FindMessage ("network", i, &tempMsg);
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

  BAutolock netLock (const_cast<BLocker *>(&settingsLock));

  if (netLock.IsLocked())
    if (visionSettings->FindMessage ("network", index, &msg) != B_OK)
      msg.AddBool ("error", true);
  return msg;
}

status_t
VisionApp::SetNetwork (const char *network, BMessage *data)
{
  BAutolock netLock (const_cast<BLocker *>(&settingsLock));

  if (!netLock.IsLocked())
    return B_ERROR;
  
  if (!strcmp (network, "defaults"))
  {
    visionSettings->ReplaceMessage ("defaults", data);
    return B_OK;
  }

  type_code type;
  int32 count (0);
  visionSettings->GetInfo ("network", &type, &count);
  for (int32 i = 0; i < count; i++)
  {
     BMessage tempMsg;
     visionSettings->FindMessage ("network", i, &tempMsg);
     if (!strcmp (tempMsg.FindString ("name"), network))
     {
       visionSettings->ReplaceMessage ("network", i, data);
       return B_OK;
     }
  }
  visionSettings->AddMessage ("network", data);
  return B_OK;

}

status_t
VisionApp::RemoveNetwork (const char *network)
{
  BMessage msg (VIS_NETWORK_DATA);

  BAutolock netLock (const_cast<BLocker *>(&settingsLock));

  if (!netLock.IsLocked())
    return B_ERROR;

  type_code type;
  int32 count (0);
  visionSettings->GetInfo ("network", &type, &count);
  for (int32 i = 0; i < count; i++)
  {
     BMessage tempMsg;
     visionSettings->FindMessage ("network", i, &tempMsg);
     if (!strcmp (tempMsg.FindString ("name"), network))
     {
       visionSettings->RemoveData ("network", i);
       break;
     }
  }
  return B_OK;
}

Theme *
VisionApp::ActiveTheme (void)
{
  return activeTheme;
}

sem_id
VisionApp::GetShutdownSem (void)
{
  return shutdownSem;
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
  const char *ident (NULL); 
  char received[64];
 
  struct sockaddr_in localAddr;
  localAddr.sin_family = AF_INET;
  localAddr.sin_port = htons (113);
  localAddr.sin_addr.s_addr = INADDR_ANY;
      
  if ((identSock = socket (AF_INET, SOCK_STREAM, 0)) >= 0 
  &&  bind (identSock, (struct sockaddr *)&localAddr, sizeof (localAddr)) == 0) 
  {
      vision_app->identSocket = identSock;

#ifdef BONE_BUILD
    struct linger lng = { 0, 0 }; 
    setsockopt (identSock, SOL_SOCKET, SO_LINGER, &lng, sizeof (linger));
#endif
    listen (identSock, 10);

    while (!vision_app->ShuttingDown) 
    {
      struct fd_set rset, eset;
      struct sockaddr_in remoteSock;
      struct timeval tv = { 10, 0};
      int size (sizeof (sockaddr_in));
      FD_ZERO (&rset);
      FD_ZERO (&eset);
      FD_SET (identSock, &rset);
      FD_SET (identSock, &eset);

      if (select (identSock + 1, &rset, 0, &eset, &tv) < 0 || FD_ISSET (identSock, &eset))
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
#ifdef BONE_BUILD
        setsockopt (accepted, SOL_SOCKET, SO_LINGER, &lng, sizeof (linger));
#endif 
          if (ident) 
          {
            memset (received, 0, 64);
            FD_SET (accepted, &rset);
            FD_SET (accepted, &eset);
            if (select (accepted + 1, &rset, 0, &eset, &tv) > 0
              && FD_ISSET (accepted, &rset))
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
          if (ident) 
            ident = NULL; 
        }
      }
    }
  }
#ifdef BONE_BUILD
  shutdown(identSock, SHUTDOWN_BOTH);
  close (identSock);
#elif NETSERVER_BUILD
  closesocket (identSock);
#endif
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

