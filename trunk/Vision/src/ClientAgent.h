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
 *                 Jamie Wilkinson
 */

#ifndef _CLIENTAGENT_H_
#define _CLIENTAGENT_H_

#include <View.h>
#include <Messenger.h>

class BScrollView;
class VTextControl;

class ClientInputFilter;
class IRCView;
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

	virtual void				MessageReceived (BMessage *);
	virtual void			AttachedToWindow (void);
	virtual void			AllAttached (void);
	virtual void            Show (void);

	float                       ScrollPos(void);
	void                        SetScrollPos(float);
    void                        ScrollRange(float *, float *);
    
    bool						ParseCmd (const char *);
    virtual void				TabExpansion (void);
	static int32				DNSLookup (void *);
	static int32				ExecPipe (void *);

    virtual void                DroppedFile (BMessage *);
    
    const BString					&Id (void) const;
    int32                           Sid (void) const;
    
    BMessenger              msgr;
    
	void								ChannelMessage (
											const char *,
											const char * = 0,
											const char * = 0,
											const char * = 0);
	void								ActionMessage (
											const char *,
											const char *);

	void								CTCPAction (BString theTarget, BString
											theMsg);

    WindowListItem                 *agentWinItem;
    
    
  private:
    void                        Init (void);
                            
  protected:
    HistoryMenu                 *history;
    IRCView                     *text;
    BScrollView                 *textScroll;
    VTextControl                *input;
    
    //AgentSettings					*settings;

	static const char				*endl;


	friend							ClientInputFilter;
	

	virtual void					Display (
											const char *,
											const rgb_color *,
											const BFont * = 0,
											bool = false);

	virtual void					Submit (const char *, bool = true, bool = true);
	
	static int32					TimedSubmit (void *);
	void								PackDisplay (BMessage *,
											const char *,
											const rgb_color * = 0,
											const BFont * = 0,
											bool = false);

	int32								FirstKnownAs (
											const BString &,
											BString &,
											bool *);
	int32								FirstSingleKnownAs (
											const BString &,
											const BString &);

	virtual void					Parser (const char *);
	virtual bool					SlashParser (const char *);
	//virtual void					StateChange (BMessage *);
    
	void							AddSend (BMessage *, const char *);
	void							AddSend (BMessage *, const BString &);
	void							AddSend (BMessage *, int32);

	BString							id;
	const int32						sid;
	const BString					serverName;
	BString							myNick;
	
	rgb_color						textColor,
										ctcpReqColor,
										nickColor,
										quitColor,
										errorColor,
										joinColor,
										whoisColor,
										myNickColor,
										actionColor,
										opColor,
										inputbgColor,
										inputColor;

	BFont								myFont,
										serverFont,
										inputFont;
	bool								timeStampState,
										canNotify,
										scrolling,
										isLogging;
	    BRect                       	frame;
	BMessenger						sMsgr;

};

const uint32 M_LOOKUP_WEBSTER           = 'calw';
const uint32 M_LOOKUP_GOOGLE            = 'calg';

const uint32 M_MOVE_UP                  = 'camu';
const uint32 M_MOVE_DOWN                = 'camd';

#endif
