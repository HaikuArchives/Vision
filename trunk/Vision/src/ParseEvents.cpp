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
 
#include "Vision.h"
#include "StringManip.h"
#include "StatusView.h"
#include "ServerAgent.h"
#include "ChannelAgent.h"
#include "MessageAgent.h"
#include "ClientWindow.h"
#include "WindowList.h"

#include <stdio.h>

bool
ServerAgent::ParseEvents (const char *data)
{
  BString firstWord = GetWord(data, 1).ToUpper();
  BString secondWord = GetWord(data, 2).ToUpper();

  if (secondWord == "PRIVMSG")
  {
    BString theNick (GetNick (data)),
            ident (GetIdent (data)),
            address (GetAddress (data)),
            addy;
    
    addy += ident;
    addy += "@";
    addy += address;

    BString theTarget (GetWord (data, 3).ToUpper()),
            theTargetOrig (GetWord (data, 3)),
            theMsg (RestOfString (data, 4));
    
    ClientAgent *client (0);

    theMsg.RemoveFirst(":");

    if (theMsg[0] == '\1' && GetWord (theMsg.String(), 1) != "\1ACTION")
    {
      // CTCP Request
      ParseCTCP (theNick, theTargetOrig, theMsg);
      return true;
    }

    if (theTarget[0] == '#' || theTarget[0] == '!' || theTarget[0] == '&')
      client = Client (theTarget.String());
    else if (!(client = Client (theNick.String())))
    {
      BString msgident (GetIdent (data)),
              msgaddress (GetAddress (data));

      vision_app->pClientWin()->pWindowList()->AddAgent (
        new MessageAgent (
          *vision_app->pClientWin()->AgentRect(),
          theNick.String(),
          sid,
          serverHostName.String(),
          sMsgr,
          myNick.String(),
          addy.String()),
        sid,
        theNick.String(),
        WIN_MESSAGE_TYPE,
        false);

      client = (vision_app->pClientWin()->pWindowList()->Agent (sid, theNick.String()));
      clients.AddItem (client);
    }

    if (client)
    {
      BString msgident (GetIdent (data)),
              msgaddress (GetAddress (data));

      client->ChannelMessage (
        theMsg.String(),
        theNick.String(),
        msgident.String(),
        msgaddress.String());
    }

    return true;
  }

  if (firstWord == "NOTICE")
  {
    BString theNotice (RestOfString(data, 4));
    theNotice.RemoveFirst(":");
    
    BString tempString;
    
    const char *expansions[2];
    expansions[0] = serverHostName.String();
    expansions[1] = theNotice.String();
    tempString = ExpandKeyed (events[E_SNOTICE].String(), "NR", expansions);
    Display (tempString.String(), &noticeColor, 0, true);
    
    return true;
  }

  if (secondWord == "NOTICE")
  {
    BString theNotice (RestOfString(data, 4));
    theNotice.RemoveFirst(":");

    BString tempString;

    firstWord.RemoveFirst (":");

    if (firstWord.ICompare (serverHostName) == 0)
    {
      const char *expansions[2];
      expansions[0] = serverHostName.String();
      expansions[1] = theNotice.String();
      tempString = ExpandKeyed (events[E_SNOTICE].String(), "NR", expansions);
      Display (tempString.String(), &noticeColor, 0, true);

      return true;
    }
    else
    {
      BString theNick (GetNick (data)),
              ident (GetIdent (data)),
              address (GetAddress (data));
              
      if (theNotice[0] == '\1')
      {
        // CTCP reply
        ParseCTCPResponse (theNick, theNotice);
        return true;
      }

      const char *expansions[4];
      expansions[0] = theNick.String();
      expansions[1] = theNotice.String();
      expansions[2] = ident.String();
      expansions[3] = address.String();

      tempString = ExpandKeyed (events[E_UNOTICE].String(), "NRIA", expansions);
      BMessage display (M_DISPLAY);
      PackDisplay (&display, tempString.String(), &noticeColor, 0, true);
      PostActive (&display);
      return true;
    }
  }

  if (secondWord == "JOIN")
  {
    BString nick (GetNick (data)),
            channel (GetWord (data, 3));

    channel.RemoveFirst (":");
    ClientAgent *client (Client (channel.String()));

    if (nick == myNick)
    {
      if (!client)
      {
        vision_app->pClientWin()->pWindowList()->AddAgent (
          new ChannelAgent (
            channel.String(),
            sid,
            serverHostName.String(),
            ircdtype,
            myNick.String(),
            sMsgr,
            *vision_app->pClientWin()->AgentRect()),
          sid,
          channel.String(),
          WIN_CHANNEL_TYPE,
          true);

        clients.AddItem ((vision_app->pClientWin()->pWindowList()->Agent (sid,
                            channel.String())));
      }

      BString tempString ("MODE ");
      tempString += channel;
      SendData (tempString.String());
    }
    else if (client)
    {
      // someone else
      BString ident (GetIdent (data)),
              address (GetAddress (data)),
              tempString;
              
      const char *expansions[3];
      expansions[0] = nick.String();
      expansions[1] = ident.String();
      expansions[2] = address.String();

      tempString = ExpandKeyed (events[E_JOIN].String(), "NIA", expansions);
      
      BMessage display (M_DISPLAY);
      
      PackDisplay (
        &display,
        tempString.String(),
        &joinColor,
        0,
        true);

      bool ignored (false);

      BMessage msg (M_USER_ADD);
      msg.AddString ("nick",  nick.String());
      msg.AddBool ("ignore", ignored);
      msg.AddMessage ("display", &display);
      client->msgr.SendMessage (&msg);
    }

    return true;
  }

  if (secondWord == "PART")
  {
    BString nick (GetNick (data)),
            channel (GetWord (data, 3));

    ClientAgent *client;

    if ((client = Client (channel.String())) != 0)
    {
      BString ident (GetIdent (data)),
              address (GetAddress (data)),
              buffer;
      
      const char *expansions[3];
      expansions[0] = nick.String();
      expansions[1] = ident.String();
      expansions[2] = address.String();

      buffer = ExpandKeyed (events[E_PART].String(), "NIA", expansions);

      BMessage display (M_DISPLAY);
      PackDisplay (&display, buffer.String(), &joinColor, 0, true);

      BMessage msg (M_USER_QUIT);
      msg.AddString ("nick", nick.String());
      msg.AddMessage ("display", &display);
      client->msgr.SendMessage (&msg);
    }

    return true;
  }

  if (secondWord == "NICK")
  {
    BString oldNick (GetNick (data)),
            ident (GetIdent (data)),
            address (GetAddress (data)),
            newNick (GetWord (data, 3)),
            buffer;
        
    newNick.RemoveFirst (":");
                    
    const char *expansions[4];
    expansions[0] = oldNick.String();
    expansions[1] = newNick.String();
    expansions[2] = ident.String();
    expansions[3] = address.String();

    buffer = ExpandKeyed (events[E_NICK].String(), "NnIA", expansions);
    BMessage display (M_DISPLAY);
    PackDisplay (&display, buffer.String(), &nickColor, 0,
                  vision_app->GetBool ("timestamp"));

    BMessage msg (M_CHANGE_NICK);
    msg.AddString ("oldnick", oldNick.String());
    msg.AddString ("newnick", newNick.String());
    msg.AddString ("ident", ident.String());
    msg.AddString ("address", address.String());
    msg.AddMessage ("display", &display);

    Broadcast (&msg);

    // Gotta change the server as well!
    if (myNick.ICompare (oldNick) == 0)
    {
      myNick = newNick;
      if (!IsHidden())
        vision_app->pClientWin()->pStatusView()->SetItemValue (STATUS_NICK,
                                                               newNick.String());
    }

    return true;
  }

  if (secondWord == "QUIT")
  {
    BString theNick (GetNick (data).String()),
            theRest (RestOfString (data, 3)),
            ident (GetIdent (data)),
            address (GetAddress (data)),
            theMsg,
            firstNick;
  
    theRest.RemoveFirst (":");
   
    const char *expansions[4];
    expansions[0] = theNick.String();
    expansions[1] = theRest.String();
    expansions[2] = ident.String();
    expansions[3] = address.String();

    theMsg = ExpandKeyed (events[E_QUIT].String(), "NRIA", expansions);
    
    BMessage display (M_DISPLAY);
    PackDisplay (&display, theMsg.String(), &quitColor, 0, true);

    BMessage msg (M_USER_QUIT);
    msg.AddMessage ("display", &display);
    msg.AddString ("nick", theNick.String());

    Broadcast (&msg);

    // see if it was our first nickname. if so, change
    firstNick = lnick1;
    if (theNick == firstNick)
    {
      BString tempCmd ("/nick ");
      tempCmd << firstNick;
      ParseCmd (tempCmd.String());
    }

    return true;
  }

  if (firstWord == "PING")
  {
    BString tempString,
            theServer (GetWord(data, 2));
    
    theServer.RemoveFirst(":");

    tempString += "PONG ";
    tempString += myNick;
    tempString += " ";
    tempString += theServer;
    SendData (tempString.String());

    // some noncompliant servers dont like the above (correct) reply,
    // so we send this one, too.
    // this can't be contained in an ircdtype if because the first reply
    // is sent before we receive the server version.
    tempString = "";
    tempString += "PONG ";
    tempString += theServer;
    SendData (tempString.String());
  
    return true;
  }

  if (secondWord == "KICK")
  {
    BString kicker (GetNick (data)),
            kickee (GetWord (data, 4)),
            rest (RestOfString (data, 5)),
            channel (GetWord (data, 3));

    ClientAgent *client (Client (channel.String()));

    rest.RemoveFirst (":");

    if ((client = Client (channel.String())) != 0
    &&   kickee == myNick)
    {
      BMessage msg (M_CHANNEL_GOT_KICKED);
      msg.AddString ("channel", channel.String());
      msg.AddString ("kicker", kicker.String());
      msg.AddString ("rest", rest.String());
      client->msgr.SendMessage (&msg);
    }

    if (client && kickee != myNick)
    {
      BMessage display (M_DISPLAY);
      const char *expansions[4];
      BString buffer;

      expansions[0] = kickee.String();
      expansions[1] = channel.String();
      expansions[2] = kicker.String();
      expansions[3] = rest.String();

      buffer = ExpandKeyed (events[E_KICK].String(), "NCnR", expansions);
      PackDisplay (&display, buffer.String(), &quitColor, 0, true);

      BMessage msg (M_USER_QUIT);
      msg.AddString ("nick", kickee.String());
      msg.AddMessage ("display", &display);
      client->msgr.SendMessage (&msg);
    }

    return true;
  }
   
  if (secondWord == "TOPIC")
  {
    BString theNick (GetNick (data)),
            theChannel (GetWord (data, 3)),
            theTopic (RestOfString (data, 4));

    ClientAgent *client (Client (theChannel.String()));

    theTopic.RemoveFirst (":");

    if (client)
    {
      BString ident (GetIdent (data)),
              address (GetAddress (data)),
              buffer;

      const char *expansions[5];
      expansions[0] = theNick.String();
      expansions[1] = theTopic.String();
      expansions[2] = client->Id().String();
      expansions[3] = ident.String();
      expansions[4] = address.String();

      BMessage topic (M_CHANNEL_TOPIC);

      topic.AddString("topic", theTopic.String());

      BMessage display (M_DISPLAY);

      buffer = ExpandKeyed (events[E_TOPIC].String(), "NTCIA", expansions);
      PackDisplay (&display, buffer.String(), &whoisColor, 0, true);
      topic.AddMessage("display", &display);
      client->msgr.SendMessage (&topic);
    }

    return true;
  }

  if (secondWord == "MODE")
  {
    BString theNick (GetNick (data)),
            theChannel (GetWord (data, 3)),
            theMode (GetWord (data, 4)),
            theTarget (RestOfString (data, 5));

    ClientAgent *client (Client (theChannel.String()));

    if (client)
    {
      BMessage msg (M_CHANNEL_MODE);

      msg.AddString("nick", theNick.String());
      msg.AddString("mode", theMode.String());
      msg.AddString("target", theTarget.String());

      client->msgr.SendMessage (&msg);
    }
    else
    {
      BMessage msg (M_DISPLAY);
      BString buffer;

      theMode.RemoveFirst(":");

      buffer += "*** User mode changed: ";
      buffer += theMode;
      buffer += "\n";
      
      PackDisplay (&msg, buffer.String(), 0, 0, true);
      PostActive (&msg);
    }

    return true;
  }

  if (firstWord == "ERROR") // server error (on connect?)
  {
    BString theError (RestOfString (data, 2));

    theError.RemoveFirst (":");
    theError.Append ("\n");

    Display (theError.String(), &quitColor);
    
    if (!isConnected) // we got the error on connect
      isConnecting = false;
    
    return true;
  }

  if (secondWord == "WALLOPS")
  {
    BString theNick (GetNick (data)),
            theWall (RestOfString (data, 3)),
            tempString;

    theWall.RemoveFirst (":");
    theWall.Append ("\n");

    tempString += "!";
    tempString += theNick;
    tempString += "! ";
    tempString += theWall;
    
    Display (tempString.String(), &wallColor);
    return true;      
  }

  if (secondWord == "INVITE")
  {
    BString tempString,
            theChannel (GetWord(data, 4));

    theChannel.RemoveFirst(":");

    tempString += "*** You have been invited to ";
    tempString += theChannel;
    tempString += " by ";
    tempString += GetNick(data);
    tempString += ".\n";

    BMessage msg (M_DISPLAY);

    PackDisplay (&msg, tempString.String(), &whoisColor, 0,
                   vision_app->GetBool("timestamp"));
    PostActive (&msg);

    return true;
  }

  // ship off to parse numerics
  return ParseENums (data, secondWord.String());
  
}

