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
 *                 Jamie Wilkinson
 */


#include <FilePanel.h>
#include <Path.h>
#include <Roster.h>
#include <FindDirectory.h>
#include <NetworkKit.h>

#include <stdio.h>
#include <stdlib.h>
#include <map.h>
#include <ctype.h>

#include <netdb.h>

#ifdef BONE_BUILD
#  include <arpa/inet.h>
#endif


#include "Vision.h"
#include "VisionBase.h"
#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "MessageAgent.h"
#include "Names.h"
#include "StringManip.h"
#include "ClientAgent.h"
#include "ClientWindow.h"
#include "IRCView.h"
#include "WindowList.h"

bool
ClientAgent::ParseCmd (const char *data)
{
  BString firstWord (GetWord(data, 1).ToUpper());
  BMessage sendMsg (M_SERVER_SEND);

  if (firstWord == "/WALLOPS"  // we need to insert a ':' before parm2
  ||  firstWord == "/SQUIT"    // for the user
  ||  firstWord == "/PRIVMSG")
  {
    {
      BString theCmd (firstWord.RemoveAll ("/")),
              theRest (RestOfString (data, 2));

      AddSend (&sendMsg, theCmd);
      if (theRest != "-9z99")
      {
        AddSend (&sendMsg, " :");
        AddSend (&sendMsg, theRest);
      }
      AddSend (&sendMsg, endl);
    }
    return true;
  }
  

  if (firstWord == "/KILL") // we need to insert a ':' before parm3
  {                         // for the user
    {
      BString theCmd (firstWord.RemoveAll ("/")),
             theTarget (GetWord (data, 2)),
             theRest (RestOfString (data, 3));

      AddSend (&sendMsg, theCmd);
      AddSend (&sendMsg, " ");
      AddSend (&sendMsg, theTarget);
      if (theRest != "-9z99")
      {
        AddSend (&sendMsg, " :");
        AddSend (&sendMsg, theRest);
      }
      AddSend (&sendMsg, endl);
    }
    return true;
  }


  // some quick aliases for scripts, these will of course be
  // moved to an aliases section eventually

  if (firstWord == "/SOUNDPLAY"
  ||  firstWord == "/CL-AMP")
  {
    {
      app_info ai;
      be_app->GetAppInfo (&ai);

      BEntry entry (&ai.ref);
      BPath path;
      entry.GetPath (&path);
      path.GetParent (&path);
      path.Append ("scripts");
 
      if (firstWord == "/SOUNDPLAY")
        path.Append ("soundplay-hey");
      else
        path.Append ("cl-amp-clr");

      BString *theCmd (new BString (path.Path()));
      
      BMessage *execmsg (new BMessage);
      execmsg->AddPointer ("exec", theCmd);
      execmsg->AddPointer ("agent", this);

      thread_id execThread = spawn_thread (
        ExecPipe,
        "exec_thread",
        B_LOW_PRIORITY,
        execmsg);

      resume_thread (execThread);
    }
    return true;
  }
  

  if (firstWord == "/ABOUT")
  {
    vision_app->PostMessage (B_ABOUT_REQUESTED);
    return true;
  }



  if (firstWord == "/AWAY")
  {
    {
      BString theReason (RestOfString (data, 2)),
              tempString;

      if (theReason == "-9z99")
        theReason = "BRB"; // Todo: make a default away msg option

      const char *expansions[1];
      expansions[0] = theReason.String();

      tempString = ExpandKeyed (vision_app->GetCommand (CMD_AWAY).String(), "R",
        expansions);
      tempString.RemoveFirst("\n");

      AddSend (&sendMsg, "AWAY");
      AddSend (&sendMsg, " :");
      AddSend (&sendMsg, theReason.String());
      AddSend (&sendMsg, endl);

      if (id != serverName)
        ActionMessage (tempString.String(), myNick.String());
    }
    return true;
  }
  

  if (firstWord == "/BACK")
  {
    {
      AddSend (&sendMsg, "AWAY");
      AddSend (&sendMsg, endl);

      if (id != serverName)
        ActionMessage (vision_app->GetCommand (CMD_BACK).String(), myNick.String());
    }
    return true;
  }
  
  
  if (firstWord == "/CLEAR")
  {
    text->ClearView (false);
    return true;
  }


  if (firstWord == "/FCLEAR")
  {
    text->ClearView (true);
    return true;
  }


  if (firstWord == "/GOOGLE")
  {
    {
      BString buffer (RestOfString (data, 2));
      if (buffer != "-9z99")
      {
        BMessage lookup (M_LOOKUP_GOOGLE);
        lookup.AddString ("string", buffer);
        msgr.SendMessage (&lookup);
      }
      else
        vision_app->LoadURL ("http://www.google.com");
    }
    return true;
  }
  

  if (firstWord == "/CTCP")
  {
    {
      BString theTarget (GetWord (data, 2)),
              theAction (RestOfString (data, 3));

      if (theAction != "-9z99")
      {
        theAction.ToUpper();

        if (theAction.ICompare ("PING") == 0)
        {
          time_t now (time (0));
          theAction << " " << now;
        }

        CTCPAction (theTarget, theAction);

        AddSend (&sendMsg, "PRIVMSG ");
        AddSend (&sendMsg, theTarget << " :\1" << theAction << "\1");
        AddSend (&sendMsg, endl);
      }
      else
        Display ("[x] /ctcp: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/DCC")
  {
    {
      BString secondWord (GetWord (data, 2)),
              theNick (GetWord (data, 3)),
              theFile (RestOfString(data, 4));
#if 0
      if (secondWord.ICompare ("SEND") == 0
      &&  theNick != "-9z99")
      {
        BMessage *msg (new BMessage (CHOSE_FILE));
        msg->AddString ("nick", theNick.String());
        if (theFile != "-9z99")
        {
          char filePath[B_PATH_NAME_LENGTH] = "\0";
          
          if (theFile.ByteAt (0) != '/')
          {
            find_directory (B_USER_DIRECTORY, 0, false, filePath, B_PATH_NAME_LENGTH);
            filePath[strlen (filePath)] = '/';
          }
          
          strcat (filePath, theFile.LockBuffer (0));
          theFile.UnlockBuffer();

          // use BPath to resolve relative pathnames, above code forces it
          // to use /boot/home as a working dir as opposed to the app path
          BPath sendPath (filePath, NULL, true);

          // the BFile is used to verify if the file exists
          // based off the documentation get_ref_for_path *should*
          // return something other than B_OK if the file doesn't exist
          // but that doesn't seem to be working correctly

          BFile sendFile (sendPath.Path(), B_READ_ONLY);

          // if the file exists, sendMsg, otherwise drop to the file panel

          if (sendFile.InitCheck() == B_OK)
          {
            sendFile.Unset();
            entry_ref ref;
            get_ref_for_path(sendPath.Path(), &ref);
            msg->AddRef("refs", &ref);
            sMsgr.SendMessage(msg);
            return true;
          }
        }
        BFilePanel *myPanel (new BFilePanel);
        BString myTitle ("Sending a file to ");

        myTitle.Append (theNick);
        myPanel->Window()->SetTitle (myTitle.String());

        myPanel->SetMessage (msg);

        myPanel->SetButtonLabel (B_DEFAULT_BUTTON, "Send");
        myPanel->SetTarget (sMsgr);
        myPanel->Show();
      }
#endif
      if (secondWord.ICompare ("CHAT") == 0
      &&       theNick != "-9z99")
      {
        if (theNick.ICompare(myNick) == 0)
          return false;
        BString thePort (GetWord (data, 4));
        BMessage msg (M_CHAT_ACTION);
        msg.AddString ("nick", theNick.String());
        if (thePort != "-9z99")
          msg.AddString ("port", thePort.String());
        sMsgr.SendMessage (&msg);
      }
    }
    return true;
  }

  if (firstWord == "/DOP" || firstWord == "/DEOP")
  {
    {
      BString theNick (RestOfString (data, 2));
      int32 current (2),
            last (2);
      if (theNick != "-9z99")
      {
        BString command ("MODE ");
        command += id;
        command += " -";
        while (GetWord(data, current) != "-9z99")
        {
          AddSend (&sendMsg, command.String());
          for (; GetWord(data, current) != "-9z99" && (current - last != 4); current++)
            AddSend (&sendMsg, "o");
          AddSend (&sendMsg, " ");
          for (; last < current; last++)
          {
            BString curNick (GetWord(data, last));
            if (curNick != "-9z99")
            {
              AddSend (&sendMsg, curNick);
              AddSend (&sendMsg, " ");
            }
          }
          AddSend (&sendMsg, endl);
          sendMsg.MakeEmpty();
        }
      }
      else
        Display ("[x] /deop: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/DESCRIBE")
  {
    {
      BString theTarget (GetWord (data, 2)),
              theAction (RestOfString (data, 3));

      if (theAction != "-9z99")
      {
        AddSend (&sendMsg, "PRIVMSG ");
        AddSend (&sendMsg, theTarget);
        AddSend (&sendMsg, " :\1ACTION ");
        AddSend (&sendMsg, theAction);
        AddSend (&sendMsg, "\1");
        AddSend (&sendMsg, endl);

        BString theActionMessage ("[ACTION]-> ");
        theActionMessage << theTarget << " -> " << theAction << "\n";

        Display (theActionMessage.String(), 0);
      }
    }
    return true;
  }


  if (firstWord == "/DNS")
  {
    {
      BString parms (GetWord(data, 2));

      ChannelAgent *channelagent;
      MessageAgent *messageagent;

      if ((channelagent = dynamic_cast<ChannelAgent *>(this)))
      {
        int32 count (channelagent->namesList->CountItems());

        for (int32 i = 0; i < count; ++i)
        {
          NameItem *item ((NameItem *)(channelagent->namesList->ItemAt (i)));

          if (!item->Name().ICompare (parms.String(), strlen (parms.String()))) //nick
          {
            AddSend (&sendMsg, "USERHOST ");
            AddSend (&sendMsg, item->Name().String());
            AddSend (&sendMsg, endl);
            return true;
          }
        }
      }
      else if ((messageagent = dynamic_cast<MessageAgent *>(this)))
      {
        BString eid (id);
        eid.RemoveLast (" [DCC]");
        if (!ICompare(eid, parms) || !ICompare(myNick, parms))
        {
          AddSend (&sendMsg, "USERHOST ");
          AddSend (&sendMsg, parms.String());
          AddSend (&sendMsg, endl);
          return true;
        }
      }

      if (parms != "-9z99")
      {
        BMessage *lookupmsg (new BMessage);
        lookupmsg->AddString ("lookup", parms.String());
        lookupmsg->AddPointer ("agent", this);

        thread_id lookupThread = spawn_thread (
          DNSLookup,
          "dns_lookup",
          B_LOW_PRIORITY,
          lookupmsg);

        resume_thread (lookupThread);
      }
      else
        Display ("[x] /dns: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }
  
  if (firstWord == "/PEXEC" || firstWord == "/RRUN") // piped exec
  {
    {
      BString *theCmd (new BString (RestOfString (data, 2)));

      if (*theCmd != "-9z99")
      {
        BMessage *msg (new BMessage);
        msg->AddPointer ("exec", theCmd);
        msg->AddPointer ("agent", this);

        thread_id execThread = spawn_thread (
          ExecPipe,
          "exec_thread",
          B_LOW_PRIORITY,
          msg);

        resume_thread (execThread);
      }
      else
      {
        Display ("[x] /pexec: Error: Invalid parameters\n", &errorColor);
        delete theCmd;
      }
    }
    return true;
  }


  #if 0
  if (firstWord == "/EXCLUDE")
  {
    {
      BString second (GetWord (data, 2)),
              rest (RestOfString (data, 3));

      if (rest != "-9z99" && rest != "-9z99")
      {
        BMessage msg (M_EXCLUDE_COMMAND);
        msg.AddString ("second", second.String());
        msg.AddString ("cmd", rest.String());
        msg.AddString ("server", serverName.String());
        msg.AddRect ("frame", Frame());
        vision_app->PostMessage (&msg);
      }
    }
    return true;
  }
  #endif
  

  if (firstWord == "/EXIT")
  {
    vision_app->PostMessage (B_QUIT_REQUESTED);
    return true;
  }
  

  #if 0
  if (firstWord == "/IGNORE")
  {
    {
      BString rest (RestOfString (data, 2));

      if (rest != "-9z99")
      {
        BMessage msg (M_IGNORE_COMMAND);
        msg.AddString ("cmd", rest.String());
        msg.AddString ("server", serverName.String());
        msg.AddRect ("frame", Frame());
        vision_app->PostMessage (&msg);
      }
    }
    return true;
  }
  #endif
  

  if (firstWord == "/INVITE" || firstWord == "/I")
  {
    {
      BString theUser (GetWord (data, 2));

      if (theUser != "-9z99")
      {
        BString theChan (GetWord (data, 3));

        if (theChan == "-9z99")
          theChan = id;

        AddSend (&sendMsg, "INVITE ");
        AddSend (&sendMsg, theUser << " " << theChan);
        AddSend (&sendMsg, endl);
      }
      else
        Display ("[x] /invite: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/JOIN" || firstWord == "/J")
  {
    // Bugs: Will not handle passing more than one channel key
    {
      BString channel (GetWord (data, 2));

      if (channel != "-9z99")
      {
        if (channel[0] != '#'
        &&  channel[0] != '!'
        &&  channel[0] != '&'
        &&  channel[0] != '+')
          channel.Prepend ("#");

        AddSend (&sendMsg, "JOIN ");
        AddSend (&sendMsg, channel);

        BString key (GetWord (data, 3));
        if (key != "-9z99")
        {
          AddSend (&sendMsg, " ");
          AddSend (&sendMsg, key);
        }

        AddSend (&sendMsg, endl);
        
        
        if (key != "-9z99")
        {
          // used for keeping track of channel keys on u2 ircds
          // (not included as part of the mode reply on join)
          ServerAgent *fatherServer (vision_app->pClientWin()->GetTopServer (agentWinItem));
          if (fatherServer != NULL)
          {
            if (fatherServer->IRCDType() == IRCD_UNDERNET)
            {
              vision_app->pClientWin()->joinStrings.Append (",");
              vision_app->pClientWin()->joinStrings.Append (data);
            }
          }
        }
      }
      else
        Display ("[x] /join: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/KICK" || firstWord == "/K")
  {
    {
      BString theNick (GetWord (data, 2));

      if (theNick != "-9z99")
      {
        BString theReason (RestOfString (data, 3));

        if (theReason == "-9z99")
        {
          // No expansions
          theReason = vision_app->GetCommand (CMD_KICK);
        }

        AddSend (&sendMsg, "KICK ");
        AddSend (&sendMsg, id);
        AddSend (&sendMsg, " ");
        AddSend (&sendMsg, theNick);
        AddSend (&sendMsg, " :");
        AddSend (&sendMsg, theReason);
        AddSend (&sendMsg, endl);
      }
      else
        Display ("[x] /kick: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/LIST")
  {
    {
      BString theArgs (RestOfString (data, 2));
      
      BMessage msg (M_LIST_COMMAND);
      msg.AddString ("cmd", data);
      sMsgr.SendMessage (&msg);
    }
    return true;
  }


  if (firstWord == "/M")
  {
    {
      BString theMode (RestOfString (data, 2));

      AddSend (&sendMsg, "MODE ");

      if (id == serverName)
        AddSend (&sendMsg, myNick);
      else if (id[0] == '#' || id[0] == '!' || id[0] == '&' || id[0] == '+')
        AddSend (&sendMsg, id);
      else
        AddSend (&sendMsg, myNick);
 
      if (theMode != "-9z99")
      {
        AddSend (&sendMsg, " ");
        AddSend (&sendMsg, theMode);
      }

      AddSend (&sendMsg, endl);
    }
    return true;
  }


  if (firstWord == "/ME")
  {
    {
      BString theAction (RestOfString (data, 2));

      if (theAction != "-9z99")
        ActionMessage (theAction.String(), myNick.String());
      else
        Display ("[x] /me: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }
  

  if (firstWord == "/MODE")
  {
    {
      BString theMode (RestOfString (data, 3)),
              theTarget (GetWord (data, 2));

      if (theTarget != "-9z99")
      {
        AddSend (&sendMsg, "MODE ");

        if (theMode == "-9z99")
          AddSend (&sendMsg, theTarget);
        else
          AddSend (&sendMsg, theTarget << " " << theMode);

        AddSend (&sendMsg, endl);
      }
      else
        Display ("[x] /nick: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/MSG")
  {
    {
      BString theRest (RestOfString (data, 3));
      BString theNick (GetWord (data, 2));

      if (theRest != "-9z99")
      {
        if (vision_app->GetBool ("queryOnMsg"))
        {
          BMessage msg (M_OPEN_MSGAGENT);
          BMessage buffer (M_SUBMIT);

          buffer.AddString ("input", theRest.String());
          msg.AddMessage ("msg", &buffer);
          msg.AddString ("nick", theNick.String());
          sMsgr.SendMessage (&msg);
        }
        else
        {
          BString tempString;
          tempString << "[M]-> " << theNick << " > " << theRest << "\n";
          Display (tempString.String(), 0);

          AddSend (&sendMsg, "PRIVMSG ");
          AddSend (&sendMsg, theNick);
          AddSend (&sendMsg, " :");
          AddSend (&sendMsg, theRest);
          AddSend (&sendMsg, endl);
        }
      }
    }
    return true;
  }

  if (firstWord == "/NEWSERVER")
  {
    {
      BString newServer (GetWord (data, 2)),
              newPort (GetWord (data, 3));
    
      if (newPort == "-9z99")
        newPort = "6667";    

      if (newServer != "-9z99")
      {
        BMessage newserver (M_MAKE_NEW_SERVER);
        newserver.AddString ("hostname", newServer);
        newserver.AddString ("port", newPort);
        newserver.AddString ("autoexec", "");
        newserver.AddBool   ("enidentd", true);
        vision_app->pClientWin()->PostMessage (&newserver);
      }
      else
        Display ("[x] /newserver: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }

  if (firstWord == "/NICK")
  {
    {
      BString newNick (GetWord (data, 2));

      if (newNick != "-9z99")
      {
        BString tempString ("*** Trying new nick ");

        tempString << newNick << ".\n";
        Display (tempString.String(), 0);

        AddSend (&sendMsg, "NICK ");
        AddSend (&sendMsg, newNick);
        AddSend (&sendMsg, endl);
      }
      else
        Display ("[x] /nick: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }

  if (firstWord == "/NOTICE")
  {
    {
      BString theTarget (GetWord (data, 2)),
              theMsg (RestOfString (data, 3));

      if (theMsg != "-9z99")
      {
        AddSend (&sendMsg, "NOTICE ");
        AddSend (&sendMsg, theTarget);
        AddSend (&sendMsg, " :");
        AddSend (&sendMsg, theMsg);
        AddSend (&sendMsg, endl);

        BString tempString ("[N]-> ");
        tempString += theTarget;
        tempString += " -> ";
        tempString += theMsg;
        tempString += '\n';
        Display (tempString.String(), 0);
      }
      else
        Display ("[x] /notice: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  #if 0
  if (firstWord == "/NOTIFY")
  {
    {
      BString rest (RestOfString (data, 2));

      if (rest != "-9z99")
      {
        BMessage msg (M_NOTIFY_COMMAND);
        msg.AddString ("cmd", rest.String());
        msg.AddBool ("add", true);
        msg.AddString ("server", serverName.String());
        msg.AddRect ("frame", Frame());
        vision_app->PostMessage (&msg);
      }
    }
    return true;
  }
  #endif
  
  if (firstWord == "/OP")
  {
    {
      BString theNick (RestOfString (data, 2));
      int32 current (2),
            last (2);
      if (theNick != "-9z99")
      {
        BString command ("MODE ");
        command += id;
        command += " +";
        while (GetWord(data, current) != "-9z99")
        {
          AddSend (&sendMsg, command.String());
          for (; GetWord(data, current) != "-9z99" && (current - last != 4); current++)
            AddSend (&sendMsg, "o");
          AddSend (&sendMsg, " ");
          for (; last < current; last++)
          {
            BString curNick (GetWord(data, last));
            if (curNick != "-9z99")
            {
              AddSend (&sendMsg, curNick);
              AddSend (&sendMsg, " ");
            }
          }
          AddSend (&sendMsg, endl);
          sendMsg.MakeEmpty();
        }
      }
      else
        Display ("[x] /deop: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }
  
  if (firstWord == "/PART")
  {
    {
      if ((dynamic_cast<ChannelAgent *>(this)) || (dynamic_cast<MessageAgent *>(this)))
      {
        BMessage msg (M_CLIENT_QUIT);
        msg.AddBool ("vision:part", true);
        msgr.SendMessage (&msg);
      }
    }
    return true;
  }

  if (firstWord == "/PING")
  {
    {
      BString theNick (GetWord (data, 2));

      if (theNick != "-9z99")
      {
        long theTime (time (0));
        BString tempString ("/CTCP ");
        tempString << theNick << " PING " << theTime;
        ParseCmd (tempString.String());
      }
    }
    return true;
  }

  if (firstWord == "/QUERY" || firstWord == "/Q")
  {
    {
      BString theNick (GetWord (data, 2)),
              theMsg (RestOfString (data, 3));
        
      if (theNick != "-9z99")
      {
        BMessage msg (M_OPEN_MSGAGENT);
        msg.AddString ("nick", theNick.String());

        if (theMsg != "-9z99")
        {
          BMessage buffer (M_SUBMIT);
          buffer.AddString ("input", theMsg.String());
          msg.AddMessage ("msg", &buffer);
        }
        
        sMsgr.SendMessage (&msg);
      }
    }
    return true;
  }

  if (firstWord == "/QUI" || firstWord == "/QUIT")
  {
    {
      BString theRest (RestOfString (data, 2)),
              buffer;
              
      if (theRest != "-9z99")
      {
        buffer += "QUIT :";
        buffer += theRest; 
      }

      BMessage msg (M_CLIENT_QUIT);
      msg.AddString ("vision:quit", buffer.String());
      if (sMsgr.IsValid())
        sMsgr.SendMessage (&msg);
    }
    return true;
  }

  if (firstWord == "/RAW" || firstWord == "/QUOTE")
  {
    {
      BString theRaw (RestOfString (data, 2));
      
      if (theRaw != "-9z99")
      {
        AddSend (&sendMsg, theRaw);
        AddSend (&sendMsg, endl);

        BString tempString ("[R]-> ");
        tempString << theRaw << '\n';

        Display (tempString.String(), 0);
      }
      else
        Display ("[x] /raw: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }


  if (firstWord == "/RECONNECT")
  {
    sMsgr.SendMessage (M_SLASH_RECONNECT);
    return true;
  }


  if (firstWord == "/SETBOOL")
  {
    {
      BString var (GetWord (data, 2)),
              value (GetWord (data, 3));
      bool newvalue (false),
           caught (false);
      value.ToLower();

      if (value != "-9z99")
      {
        if (value == "true")
        {
          caught = true;
          newvalue = true;
        }
        else if (value == "false")
        {
          caught = true;
          newvalue = false;
        }
      }

      if (!caught || value == "-9z99")
      {
        Display ("[x] /setbool: Error: Invalid parameters\n", &errorColor);
      }
      else
      {
        status_t returned (vision_app->SetBool (var.String(), newvalue));
        if (returned == B_OK)
          Display ("[x] /setbool: Bool has been set\n", &errorColor);
        else
          Display ("[x] /setbool: Error setting bool\n", &errorColor);
      }
    }
    return true;
  }
  
  if (firstWord == "/SLEEP")
  {
    {
      BString rest (RestOfString (data, 2));

      if (rest != "-9z99")
      {
        // this basically locks up the window its called from,
        // but I can't think of a better way with our current
        // commands implementation
        int32 sleeptime = atoi(rest.String());
        snooze (sleeptime * 1000 * 100); // deciseconds? 10 = one second
      }
    }
    return true;
  }

  if (firstWord == "/TOPIC" || firstWord == "/T")
  {
    {
      BString theChan (id),
              theTopic (RestOfString (data, 2));

      AddSend (&sendMsg, "TOPIC ");

      if (theTopic == "-9z99")
        AddSend (&sendMsg, theChan);
      else
        AddSend (&sendMsg, theChan << " :" << theTopic);
      
      AddSend (&sendMsg, endl);
    }
    return true;
  }

  #if 0
  if (firstWord == "/UNIGNORE")
  {
    {
      BString rest (RestOfString (data, 2));

      if (rest != "-9z99")
      {
        BMessage msg (M_UNIGNORE_COMMAND);
        msg.AddString ("cmd", rest.String());
        msg.AddString ("server", serverName.String());
        msg.AddRect ("frame", Frame());
        vision_app->PostMessage (&msg);
      }
    }
    return true;
  }
  #endif


  #if 0
  if (firstWord == "/UNNOTIFY")
  {
    {
      BString rest (RestOfString (data, 2));

      if (rest != "-9z99")
      {
        BMessage msg (M_NOTIFY_COMMAND);
        msg.AddString ("cmd", rest.String());
        msg.AddBool ("add", false);
        msg.AddRect ("frame", Frame());
        msg.AddString ("server", serverName.String());
        vision_app->PostMessage (&msg);
      }
    }
    return true;
  }
  #endif


  if (firstWord == "/UPTIME")
  {
    {
      BString parms (GetWord(data, 2)),
              uptime (DurationString (system_time())),
              expandedString;
              
      const char *expansions[1];
      expansions[0] = uptime.String();
      expandedString = ExpandKeyed (vision_app->GetCommand (CMD_UPTIME).String(), "U",
        expansions);
      expandedString.RemoveFirst("\n");

      if ((id != serverName) && (parms == "-9z99"))
      {
        AddSend (&sendMsg, "PRIVMSG ");
        AddSend (&sendMsg, id);
        AddSend (&sendMsg, " :");
        AddSend (&sendMsg, expandedString.String());
        AddSend (&sendMsg, endl);
        
        ChannelMessage (expandedString.String(), myNick.String());
      }
      else if ((parms == "-l") || (id == serverName)) // echo locally
      {
        BString tempString;
        tempString << "Uptime: " << expandedString << "\n";
        Display (tempString.String(), &whoisColor);
      }
    }
    return true;
  }


  if (firstWord == "/VERSION"
  ||  firstWord == "/TIME")
  {
    {
      BString theCmd (firstWord.RemoveFirst ("/")),
              theNick (GetWord (data, 2));
      theCmd.ToUpper();

      // the "." check is because the user might specify a server name

      if (theNick != "-9z99" && theNick.FindFirst(".") < 0)
      {
        BString tempString ("/CTCP ");
        tempString << theNick << " " << theCmd;
        ParseCmd (tempString.String());
      }
      else
      {
        AddSend (&sendMsg, theCmd);

        if (theNick != "-9z99")
        {
          AddSend (&sendMsg, " ");
          AddSend (&sendMsg, theNick);
        }

        AddSend (&sendMsg, endl);
      }
    }
    return true;
  }


  if (firstWord == "/VISIT")
  {
    {
      BString buffer (GetWord (data, 2));

      if (buffer != "-9z99")
        vision_app->LoadURL (buffer.String());
      else
        Display ("[x] /visit: Error: Invalid parameters\n", &errorColor);
    }
    return true;
  }

  if (firstWord == "/WEBSTER" || firstWord =="/DICTIONARY")
  {
    {
      BString buffer (RestOfString (data, 2));
    
      if (buffer != "-9z99")
      {
        BMessage lookup (M_LOOKUP_WEBSTER);
        lookup.AddString ("string", buffer);
        msgr.SendMessage (&lookup);
      }
      else
        vision_app->LoadURL ("http://www.dictionary.com");
    }
    return true;
  }

  if (firstWord != "" && firstWord[0] == '/')
  // != "" is required to prevent a nasty crash with firstWord[0]
  {
    {
      BString theCmd (firstWord.RemoveAll ("/")),
              theRest (RestOfString (data, 2));


      if (theCmd == "W")
        theCmd = "WHOIS";

      AddSend (&sendMsg, theCmd);

      if (theRest != "-9z99")
      {
        AddSend (&sendMsg, " ");
        AddSend (&sendMsg, theRest);
      }
      
      AddSend (&sendMsg, endl);
    }
    return true;
  }

  return false;  // we couldn't handle this message

}


int32
ClientAgent::ExecPipe (void *arg)
{
  BMessage *msg (reinterpret_cast<BMessage *>(arg));
  BString *exec;
  ClientAgent *agent;

  if ((msg->FindPointer ("exec", reinterpret_cast<void **>(&exec)) != B_OK)
  ||  (msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
  {
    printf (":ERROR: couldn't find valid data in BMsg to ExecPipe() -- bailing\n");
    return B_ERROR;
  }

  delete msg;

  FILE *fp = popen (exec->String(), "r");

  char read[768]; // should be long enough for any line...

  while (fgets(read, 768, fp))
  {
    read[strlen(read)-1] = '\0'; // strip termination

    // ship off to agent
    agent->LockLooper();
    agent->Submit (read);
    agent->UnlockLooper();
  }

  pclose(fp);
  delete exec;

  return B_OK;
}

int32
ClientAgent::DNSLookup (void *arg)
{

  BMessage *msg (reinterpret_cast<BMessage *>(arg));
  const char *lookup;
  ClientAgent *agent;

  if ((msg->FindString ("lookup", &lookup) != B_OK)
  ||  (msg->FindPointer ("agent", reinterpret_cast<void **>(&agent)) != B_OK))
  {
    printf (":ERROR: couldn't find valid data in BMsg to DNSLookup() -- bailing\n");
    return B_ERROR;
  } 

  BString resolve (lookup),
          output ("[x] ");

  if (isalpha (resolve[0]))
  {
    hostent *hp = gethostbyname (resolve.String());

    if (hp)
    {
      // ip address is in hp->h_addr_list[0];
      char addr_buf[16];

      in_addr *addr = (in_addr *)hp->h_addr_list[0];
      strcpy(addr_buf, inet_ntoa(*addr));
      output += "Resolved ";
      output += resolve.String();
      output += " to ";
      output += addr_buf;
    }
    else
    {
      output += "Unable to resolve ";
      output += resolve.String();
    }
  }
  else
  {
    ulong addr = inet_addr (resolve.String());
    hostent *hp = gethostbyaddr ((const char *)&addr, 4, AF_INET);
    
    if (hp)
    {
      output += "Resolved ";
      output += resolve.String();
      output += " to ";
      output += hp->h_name;
    }
    else
    {
      output += "Unable to resolve ";
      output += resolve.String();
    }
  }

  output += "\n";

  delete msg;

  BMessage dnsMsg (M_DISPLAY);
  agent->PackDisplay (&dnsMsg, output.String(), C_WHOIS);
  agent->msgr.SendMessage (&dnsMsg);

  return B_OK;
}
