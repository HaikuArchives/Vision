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
 */

#ifndef _VISIONBASE_H_
#define _VISIONBASE_H_

/// base includes and defines ///
#ifdef GNOME_BUILD
#  include "gnome/GraphicsDefs.h"
#elif BEOS_BUILD
#  include <GraphicsDefs.h>
#endif

#define C_TEXT											0
#define C_BACKGROUND									1
#define C_NAMES											2
#define C_NAMES_BACKGROUND								3
#define C_URL											4
#define C_SERVER										5
#define C_NOTICE										6
#define C_ACTION										7
#define C_QUIT											8
#define C_ERROR											9
#define C_NICK											10
#define C_MYNICK										11
#define C_JOIN											12
#define C_KICK											13
#define C_WHOIS											14
#define C_OP											15
#define C_HELPER										16
#define C_VOICE											17
#define C_CTCP_REQ										18
#define C_CTCP_RPY										19
#define C_IGNORE										20
#define C_INPUT_BACKGROUND								21
#define C_INPUT											22
#define C_WINLIST_BACKGROUND                            23
#define C_WINLIST_TEXT                                  24
#define C_WINLIST_NEWS                                  25
#define C_WINLIST_NICK                                  26
#define C_WINLIST_SELECTION                             27
#define C_WINLIST_VOID                                  28
#define C_NAMES_SELECTION                               29
#define C_WALLOPS                                       30
#define C_NICKDISPLAY                                   31

#define MAX_COLORS										32


#define F_TEXT												0
#define F_SERVER											1
#define F_URL												2
#define F_NAMES												3
#define F_INPUT												4
#define F_WINLIST                                           5

#define MAX_FONTS											6

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
#define STATUS_OP_BIT                                   0x0001
#define STATUS_HELPER_BIT                               0x0002
#define STATUS_VOICE_BIT                                0x0004
#define STATUS_NORMAL_BIT                               0x0008
#define STATUS_IGNORE_BIT                               0x0010


// tells the WindowList how to color the WindowListItem
#define WIN_VOID_BIT                                0x0001
#define WIN_NORMAL_BIT                              0x0010
#define WIN_NEWS_BIT								0x0100
#define WIN_NICK_BIT							 	0x1000

// tells the WindowList how to format the WindowListItem
// (eg: WIN_CHANNEL_TYPE = indent 2 spaces)
#define WIN_SERVER_TYPE                                 0x0001
#define WIN_CHANNEL_TYPE                                0x0010
#define WIN_MESSAGE_TYPE                                0x0100


const int32 ID_SERVER = -47;
const int32 ID_NOTCHILD = -1;



const int32 BIG_ENOUGH_FOR_A_REALLY_FAST_ETHERNET	= 1024 * 16;

const uint32 M_ACTIVATION_CHANGE 					= 0x1011;
const uint32 M_NEW_CLIENT							= 0x1012;
const uint32 M_QUIT_CLIENT							= 0x1013;
const uint32 M_NEWS_CLIENT							= 0x1014;
const uint32 M_NICK_CLIENT							= 0x1015;
const uint32 M_ID_CHANGE							= 0x1016;

const uint32 M_NOTIFY_SELECT						= 0x1017;
const uint32 M_NOTIFY_PULSE							= 0x1018;
const uint32 M_NOTIFY_END							= 0x1019;
const uint32 M_NOTIFY_START							= 0x1020;
const uint32 M_NOTIFY_USER							= 0x1021;
const uint32 M_NOTIFY_COMMAND						= 0x1022;
const uint32 M_NOTIFY_WINDOW						= 0x1023;
const uint32 M_NOTIFY_SHUTDOWN						= 0x1024;

const uint32 M_LIST_BEGIN							= 0x1025;
const uint32 M_LIST_EVENT							= 0x1026;
const uint32 M_LIST_DONE							= 0x1027;
const uint32 M_LIST_COMMAND							= 0x1028;
const uint32 M_LIST_SHUTDOWN						= 0x1029;

const uint32 M_IS_IGNORED							= 0x1030;
const uint32 M_IGNORE_COMMAND						= 0x1031;
const uint32 M_IGNORE_SHUTDOWN						= 0x1032;
const uint32 M_UNIGNORE_COMMAND  					= 0x1033;
const uint32 M_EXCLUDE_COMMAND						= 0x1034;
const uint32 M_IGNORE_WINDOW						= 0x1035;

const uint32 M_STATE_CHANGE							= 0x1036;
const uint32 M_SERVER_STARTUP						= 0x1037;
const uint32 M_SERVER_CONNECTED						= 0x1038;

const uint32 M_SEND_TO_AGENT                        = 0x2000;

const uint32 M_PREVIOUS_CLIENT						= 0x1300;
const uint32 M_NEXT_CLIENT							= 0x1301;
const uint32 M_PREVIOUS_INPUT						= 0x1302;
const uint32 M_NEXT_INPUT							= 0x1303;
const uint32 M_SUBMIT								= 0x1304;
const uint32 M_DISPLAY								= 0x1305;
const uint32 M_SUBMIT_RAW							= 0x1306;
const uint32 M_CHANNEL_MSG							= 0x1307;
const uint32 M_CHANGE_NICK							= 0x1308;
const uint32 M_CHANNEL_MODES						= 0x1309;
const uint32 M_LAG_CHANGED							= 0x1310;
const uint32 M_CLIENT_QUIT							= 0x1311;

const uint32 M_SERVER_SEND							= 0x1701;
const uint32 M_SERVER_SHUTDOWN						= 0x1702;
const uint32 M_CLIENT_SHUTDOWN						= 0x1703;
const uint32 M_IGNORED_PRIVMSG						= 0x1704;
const uint32 M_OPEN_MSGAGENT   						= 0x1705;
const uint32 M_SLASH_RECONNECT						= 0x1708;
const uint32 M_REJOIN                               = 0x1709;

const uint32 M_USER_QUIT							= 0x1600;
const uint32 M_USER_ADD								= 0x1601;
const uint32 M_CHANNEL_NAMES						= 0x1602;
const uint32 M_CHANNEL_TOPIC						= 0x1603;
const uint32 M_CHANNEL_MODE							= 0x1604;
const uint32 M_INPUT_FOCUS                          = 0x1605;
const uint32 M_CHANNEL_GOT_KICKED					= 0x1606;

#endif
