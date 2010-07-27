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
 */

#ifndef _VISIONBASE_H_
#define _VISIONBASE_H_

#include "VisionMessages.h"

/// base includes and defines ///
#include <GraphicsDefs.h>

#include <Debug.h>
#include <Locale.h>
#include <String.h>

// TODO: resort these so they make more sense -- will break settings compat though

#define C_TEXT											0
#define C_BACKGROUND									1
#define C_URL											2
#define C_SERVER										3
#define C_NOTICE										4
#define C_ACTION										5
#define C_QUIT											6
#define C_ERROR											7
#define C_NICK											8
#define C_MYNICK										9
#define C_NICKDISPLAY																	 10
#define C_JOIN												11
#define C_KICK											12
#define C_WHOIS											13
#define C_NAMES											14
#define C_OP											15
#define C_HELPER										16
#define C_VOICE											17
#define C_NAMES_SELECTION															 18
#define C_NAMES_BACKGROUND								19
#define C_CTCP_REQ										20
#define C_CTCP_RPY										21
#define C_IGNORE										22
#define C_INPUT											23
#define C_INPUT_BACKGROUND								24
#define C_WINLIST_NORMAL																25
#define C_WINLIST_NEWS																	26
#define C_WINLIST_NICK																	27
#define C_WINLIST_SELECTION														 28
#define C_WINLIST_PAGESIX															 29
#define C_WINLIST_BACKGROUND														30
#define C_WALLOPS																			 31
#define C_TIMESTAMP																		 32
#define C_TIMESTAMP_BACKGROUND													33
#define C_SELECTION																		 34

// mirc color codes 
#define C_MIRC_WHITE									35 
#define C_MIRC_BLACK									36
#define C_MIRC_BLUE										37
#define C_MIRC_GREEN									38
#define C_MIRC_RED										39
#define C_MIRC_BROWN									40
#define C_MIRC_PURPLE									41
#define C_MIRC_ORANGE									42
#define C_MIRC_YELLOW									43
#define C_MIRC_LIME										44
#define C_MIRC_TEAL										45
#define C_MIRC_AQUA										46
#define C_MIRC_LT_BLUE									47
#define C_MIRC_PINK										48
#define C_MIRC_GREY										49
#define C_MIRC_SILVER									50

#define C_NOTIFY_ON																		 51
#define C_NOTIFY_OFF																		52
#define C_NOTIFYLIST_BACKGROUND												 53
#define C_NOTIFYLIST_SELECTION													54

#define MAX_COLORS										55


#define F_TEXT												0
#define F_SERVER											1
#define F_URL												2
#define F_NAMES												3
#define F_INPUT												4
#define F_WINLIST																					 5
#define F_LISTAGENT																				 6
#define F_TIMESTAMP																				 7

#define MAX_FONTS											8

#define E_JOIN												0
#define E_PART												1
#define E_NICK												2
#define E_QUIT												3
#define E_KICK												4
#define E_TOPIC												5
#define E_SNOTICE											6
#define E_UNOTICE											7
#define E_NOTIFY_ON											8
#define E_NOTIFY_OFF										9

#define MAX_EVENTS											10

#define CMD_QUIT											0
#define CMD_KICK											1
#define CMD_IGNORE											2
#define CMD_UNIGNORE										3
#define CMD_AWAY											4
#define CMD_BACK											5
#define CMD_UPTIME											6

#define MAX_COMMANDS										7

// tells the NamesView how to color the nicks
#define STATUS_FOUNDER_BIT															0x0001
#define STATUS_PROTECTED_BIT														0x0002
#define STATUS_OP_BIT																	 0x0004
#define STATUS_HELPER_BIT															 0x0008
#define STATUS_VOICE_BIT																0x0010
#define STATUS_NORMAL_BIT															 0x0020
#define STATUS_IGNORE_BIT															 0x0040


// tells the WindowList how to color the WindowListItem
#define WIN_NORMAL_BIT															0x0001
#define WIN_PAGESIX_BIT														 0x0010
#define WIN_NEWS_BIT								0x0100
#define WIN_NICK_BIT							 	0x1000

// tells the WindowList how to format and sort the WindowListItem
// (eg: WIN_CHANNEL_TYPE = indent 2 spaces)
#define WIN_SERVER_TYPE																 0x0001
#define WIN_CHANNEL_TYPE																0x0010
#define WIN_MESSAGE_TYPE																0x0100
#define WIN_LIST_TYPE																	 0x0200

/// IRCDS
/// an effort to properly support conflicting numeric meanings

const int IRCD_STANDARD							 =	1;
const int IRCD_HYBRID								 =	2;	// "hybrid"		
const int IRCD_ULTIMATE							 =	3;	// "UltimateIRCd"
const int IRCD_COMSTUD								=	4;	// "comstud"
const int IRCD_UNDERNET							 =	5;	// "u2."
const int IRCD_BAHAMUT								=	6;	// "bahamut"
const int IRCD_PTLINK								 =	7;	// "PTlink"
const int IRCD_CONFERENCEROOM				 =	8;	// "CR"
const int IRCD_NEWNET								 =	9;	// "nn-"


const int32 ID_SERVER = -47;
const int32 ID_NOTCHILD = -1;


const uint32 SERVER_PRIMARY					 =	0;
const uint32 SERVER_SECONDARY				 =	1;
const uint32 SERVER_DISABLED					=	2;


const int32 BIG_ENOUGH_FOR_A_REALLY_FAST_ETHERNET	= 1024 * 16;

// Sound event identifiers
enum SoundEvent
{
	seNickMentioned = 0,
	seSoundEventsNumber
};

extern const char *kSoundEventNames[];

// network data structure:
struct ServerData
{
	char serverName[255];
	uint32 port;
	uint32 state;
	char password[255];
};

#endif
