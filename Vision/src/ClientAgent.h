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

#ifndef _CLIENTAGENT_H_
#define _CLIENTAGENT_H_

#ifdef GNOME_BUILD
#  include "gnome/View.h"
#  include "gnome/Messenger.h"
#elif BEOS_BUILD
#  include <View.h>
#  include <Messenger.h>
#endif

class BScrollView;
class VTextControl;

class BPopUpMenu;
class ClientAgentInputFilter;
class ClientAgentLogger;
class RunView;
class HistoryMenu;
class AgentSettings;
class WindowListItem;

class ClientAgent : public BView
{
  public:
//                                ClientAgent (BRect,
//                                             const char *,
//                                             uint32,
//                                             uint32);

                                // used by ServerAgent
                                ClientAgent (
                                  const char *,         // id_  (window name)
                                  int32,                // sid_ (server id)
                                  const char *,         // serverName_
                                  const char *,         // myNick_
                                  BRect);                // frame
                                ClientAgent (
                                  const char *,         // id_  (window name)
                                  int32,                // sid_ (server id)
                                  const char *,         // serverName_
                                  const char *,         // myNick_
                                  const BMessenger &,   // sMsgr pointer
                                  BRect);                // frame
                                  
    virtual                     ~ClientAgent (void);

    virtual void                MessageReceived (BMessage *);
    virtual void                AttachedToWindow (void);
    virtual void                AllAttached (void);
    virtual void                Show (void);

    void                        AddMenuItems (BPopUpMenu *);

    float                       ScrollPos(void);
    void                        SetScrollPos(float);
    void                        ScrollRange(float *, float *);
    void                        SetServerName(const char *);
    
    bool                        ParseCmd (const char *);
    virtual void                TabExpansion (void);
    static int32                DNSLookup (void *);
    static int32                ExecPipe (void *);

    virtual void                DroppedFile (BMessage *);
    
    const BString               &Id (void) const;
    int32                       Sid (void) const;
    
    BMessenger              msgr;
    
    void                        ChannelMessage (
                                  const char *,
                                  const char * = 0,
                                  const char * = 0,
                                  const char * = 0);
    void                        ActionMessage (
                                  const char *,
                                  const char *);

    void                        CTCPAction (BString theTarget, BString theMsg);

    WindowListItem                 *agentWinItem;
    
    
  private:
    void                        Init (void);
                            
  protected:
    HistoryMenu                 *history;
    RunView                     *text;
    BScrollView                 *textScroll;
    VTextControl                *input;

    static const char               *endl;


    friend                          ClientAgentInputFilter;
 
    virtual void                    Display (
                                      const char *,
                                      uint32 = 0,
                                      uint32 = 0,
                                      uint32 = 0);
                                      
    virtual void                    Submit (const char *, bool = true, bool = true);

    static int32                    TimedSubmit (void *);
    static void                     PackDisplay (BMessage *,
                                      const char *,
                                      const uint32 = 0,
                                      const uint32 = 0,
                                      const uint32 = 0);

    int32                           FirstKnownAs (
                                      const BString &,
                                      BString &,
                                      bool *);
    int32                           FirstSingleKnownAs (
                                      const BString &,
                                      const BString &);

    virtual void                    Parser (const char *);
    virtual bool                    SlashParser (const char *);
    //virtual void                  StateChange (BMessage *);
    
    void                            AddSend (BMessage *, const char *);
    void                            AddSend (BMessage *, const BString &);
    void                            AddSend (BMessage *, int32);

    BString                         id;
    const int32                     sid;
    BString                         serverName;
    BString                         myNick,
                                      myLag;

    bool                             timeStampState,
                                       canNotify,
                                       scrolling,
                                       isLogging;
                                       
    BRect                            frame;
    BMessenger                       sMsgr;
    ClientAgentLogger                *logger;

};

const uint32 M_LOOKUP_WEBSTER         = 'calw';
const uint32 M_LOOKUP_GOOGLE          = 'calg';

/// IRCDS
/// an effort to properly support conflicting numeric meanings

const int IRCD_STANDARD               =  1;
const int IRCD_HYBRID                 =  2;  // "hybrid"    
const int IRCD_ULTIMATE               =  3;  // "UltimateIRCd"
const int IRCD_COMSTUD                =  4;  // "comstud"
const int IRCD_UNDERNET               =  5;  // "u2."
const int IRCD_BAHAMUT                =  6;  // "bahamut"
const int IRCD_PTLINK                 =  7;  // "PTlink"
const int IRCD_CONFERENCEROOM         =  8;  // "CR"
const int IRCD_NEWNET                 =  9;  // "nn-"


#endif
