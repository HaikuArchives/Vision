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

#ifndef _VISIONMESSAGES_H_
#define _VISIONMESSAGES_H_

const uint32 M_MENU_NUKE							= 0x1010;
const uint32 M_ACTIVATION_CHANGE 					= 0x1011;
const uint32 M_NEW_CLIENT							= 0x1012;
const uint32 M_QUIT_CLIENT							= 0x1013;
const uint32 M_NEWS_CLIENT							= 0x1014;
const uint32 M_NICK_CLIENT							= 0x1015;
const uint32 M_ID_CHANGE							= 0x1016;

const uint32 M_NOTIFYLIST_ADD				 		= 0x1017;
const uint32 M_NOTIFYLIST_REMOVE					= 0x1018;
const uint32 M_NOTIFYLIST_CHANGE					= 0x1019;
const uint32 M_NOTIFYLIST_UPDATE					= 0x1020;
const uint32 M_NOTIFYLIST_RESIZE					= 0x1021;
const uint32 M_WINLIST_NOTIFY_BLINKER				= 0x1022;
// reserved											= 0x1023;
// reserved											= 0x1024;
		
const uint32 M_LIST_BEGIN							= 0x1025;
const uint32 M_LIST_EVENT							= 0x1026;
const uint32 M_LIST_DONE							= 0x1027;
const uint32 M_LIST_COMMAND							= 0x1028;
const uint32 M_LIST_SHUTDOWN						= 0x1029;
const uint32 M_LIST_FIND							= 0x1030;
const uint32 M_LIST_FAGAIN							= 0x1031;
const uint32 M_LIST_FILTER							= 0x1032;
const uint32 M_LIST_INVOKE							= 0x1033;
const uint32 M_LIST_UPDATE							= 0x1034;

const uint32 M_IS_IGNORED							= 0x1035;
const uint32 M_IGNORE_ADD							= 0x1036;
const uint32 M_IGNORE_LIST							= 0x1037;
const uint32 M_IGNORE_REMOVE						= 0x1038;
const uint32 M_EXCLUDE_ADD							= 0x1039;
const uint32 M_EXCLUDE_REMOVE						= 0x1040;

const uint32 M_STATE_CHANGE							= 0x1041;
const uint32 M_SERVER_STARTUP						= 0x1042;
const uint32 M_SERVER_CONNECTED						= 0x1043;
const uint32 M_RESIZE_VIEW							= 0x1044;
const uint32 M_ABOUT_SCROLL							= 0x1045;

const uint32 M_PREVIOUS_INPUT						= 0x1046;
const uint32 M_NEXT_INPUT							= 0x1047;

const uint32 M_UP_CLIENT							= 0x1300;
const uint32 M_DOWN_CLIENT							= 0x1301;
const uint32 M_SMART_UP_CLIENT						= 0x1302;
const uint32 M_SMART_DOWN_CLIENT					= 0x1303;
const uint32 M_NETWORK_CLIENT						= 0x1304;
const uint32 M_PREVIOUS_CLIENT						= 0x1305;
const uint32 M_COLLAPSE_NETWORK						= 0x1306;
const uint32 M_EXPAND_NETWORK						= 0x1307;
const uint32 M_NETWORK_UP							= 0x1308;
const uint32 M_NETWORK_DOWN							= 0x1309;
const uint32 M_SUBMIT								= 0x1310;
const uint32 M_DISPLAY								= 0x1311;
const uint32 M_SUBMIT_INPUT							= 0x1312;
const uint32 M_CHANNEL_MSG							= 0x1313;
const uint32 M_CHANGE_NICK							= 0x1314;
const uint32 M_CHANNEL_MODES						= 0x1315;
const uint32 M_LAG_CHANGED							= 0x1316;
const uint32 M_CLIENT_QUIT							= 0x1317;
const uint32 M_JOIN_CHANNEL							= 0x1318;

const uint32 M_MSG_WHOIS							= 0x1400;

const uint32 M_USER_QUIT							= 0x1600;
const uint32 M_USER_ADD								= 0x1601;
const uint32 M_CHANNEL_NAMES						= 0x1602;
const uint32 M_CHANNEL_TOPIC						= 0x1603;
const uint32 M_CHANNEL_MODE							= 0x1604;
const uint32 M_INPUT_FOCUS							= 0x1605;
const uint32 M_CHANNEL_GOT_KICKED					= 0x1606;
const uint32 M_LOOKUP_WEBSTER						= 0x1607;
const uint32 M_LOOKUP_GOOGLE						= 0x1608;
const uint32 M_LOOKUP_ACRONYM						= 0x1609;
const uint32 M_LOOKUP_BROWSER						= 0x1610;
const uint32 M_CHANNEL_OPTIONS_SHOW					= 0x1611;
const uint32 M_CHANNEL_OPTIONS_CLOSE				= 0x1612;


const uint32 M_SERVER_SEND							= 0x1701;
const uint32 M_SERVER_SHUTDOWN						= 0x1702;
const uint32 M_CLIENT_SHUTDOWN						= 0x1703;
const uint32 M_IGNORED_PRIVMSG						= 0x1704;
const uint32 M_OPEN_MSGAGENT	 					= 0x1705;
const uint32 M_SLASH_RECONNECT						= 0x1708;
const uint32 M_REJOIN								= 0x1709;
const uint32 M_GET_ESTABLISH_DATA					= 0x1710;
const uint32 M_SET_ENDPOINT							= 0x1711;
const uint32 M_GET_RECONNECT_STATUS					= 0x1712;
const uint32 M_NOT_CONNECTING						= 0x1713;
const uint32 M_CONNECTED							= 0x1714;
const uint32 M_INC_RECONNECT						= 0x1715;
const uint32 M_DISPLAY_ALL							= 0x1716;
const uint32 M_SET_IP								= 0x1717;
const uint32 M_GET_IP								= 0x1718;
const uint32 M_SERVER_DISCONNECT					= 0x1719;
const uint32 M_PARSE_LINE							= 0x1720;
const uint32 M_LAG_CHECK							= 0x1721;
const uint32 M_REJOIN_ALL							= 0x1722;
const uint32 M_SEND_RAW								= 0x1723;
const uint32 M_CHAT_ACCEPT							= 0x1724;
const uint32 M_CHAT_ACTION							= 0x1725;
const uint32 M_REGISTER_LOGGER						= 0x1726;
const uint32 M_UNREGISTER_LOGGER					= 0x1727;
const uint32 M_CLIENT_LOG							= 0x1728;
const uint32 M_SERVER_THREAD_VALID					= 0x1729;
const uint32 M_GET_SENDER_DATA						= 0x1730;

const uint32 M_DCC_COMPLETE							= 0x1800;
const uint32 M_DCC_FILE_WIN_DONE					= 0x1801;
const uint32 M_DCC_FILE_WIN							= 0x1802;
const uint32 M_ADD_RESUME_DATA						= 0x1803;
const uint32 M_DCC_MESSENGER						= 0x1804;
const uint32 M_DCC_ACCEPT							= 0x1805;
const uint32 M_CHOSE_FILE							= 0x1806;
const uint32 M_ABOUT_CLOSE							= 0x1807; 
const uint32 M_DCC_STOP_BUTTON						= 0x1808;
const uint32 M_DCC_UPDATE_STATUS					= 0x1809;
const uint32 M_DCC_GET_CONNECT_DATA					= 0x1810;
const uint32 M_DCC_GET_RESUME_POS					= 0x1811;
const uint32 M_DCC_UPDATE_TRANSFERRED				= 0x1812;
const uint32 M_DCC_UPDATE_AVERAGE					= 0x1813;
const uint32 M_DCC_FINISH							= 0x1814;

const uint32 M_THEME_FOREGROUND_CHANGE				= 0x1900;
const uint32 M_THEME_BACKGROUND_CHANGE				= 0x1901;
const uint32 M_THEME_FONT_CHANGE					= 0x1902;

const uint32 M_CW_UPDATE_STATUS						= 0x2000;
const uint32 M_OBITUARY								= 0x2001;
const uint32 M_CW_ALTW								= 0x2002;
const uint32 M_CW_ALTW_RESET						= 0x2003;
const uint32 M_CW_ALTP								= 0x2004;
const uint32 M_OPEN_TERM							= 0x2005;
const uint32 M_MAKE_NEW_NETWORK						= 0x2006;
const uint32 M_STATUS_CLEAR							= 0x2007;
const uint32 M_STATUS_ADDITEMS						= 0x2008;

// names view

const uint32 M_NAMES_POPUP_MODE						= 0x2100;
const uint32 M_NAMES_POPUP_CTCP						= 0x2101;
const uint32 M_NAMES_POPUP_WHOIS					= 0x2102;
const uint32 M_NAMES_POPUP_KICK						= 0x2103;
const uint32 M_NAMES_POPUP_DCCCHAT					= 0x2104;
const uint32 M_NAMES_POPUP_DCCSEND					= 0x2105;
const uint32 M_NAMES_POPUP_NOTIFY					= 0x2106;

// network prefs

const uint32 M_SERVER_ADD_ITEM						= 0x2200;
const uint32 M_SERVER_EDIT_ITEM						= 0x2201;
const uint32 M_SERVER_REMOVE_ITEM					= 0x2202;
const uint32 M_SERVER_RECV_DATA						= 0x2203;
const uint32 M_SERVER_ITEM_SELECTED					= 0x2204;
const uint32 M_SERVER_DATA_CHANGED					= 0x2205;
const uint32 M_SERVER_NAME_CHANGED					= 0x2206;
const uint32 M_SERVER_PORT_CHANGED					= 0x2207;
const uint32 M_SERVER_DONE							= 0x2208;
const uint32 M_SERVER_CANCEL						= 0x2209;
const uint32 M_SERVER_STATE							= 0x2210;
const uint32 M_NETWORK_DEFAULTS						= 0x2211;
const uint32 M_CHOOSE_NETWORK						= 0x2212;
const uint32 M_ADD_NEW_NETWORK						= 0x2213;
const uint32 M_DUPE_CURRENT_NETWORK					= 0x2214;
const uint32 M_REMOVE_CURRENT_NETWORK				= 0x2215;
const uint32 M_SERVER_DIALOG						= 0x2216;
const uint32 M_NET_CHECK_LAG						= 0x2217;
const uint32 M_CONNECT_ON_STARTUP					= 0x2218;
const uint32 M_REMOVE_NICK							= 0x2219;
const uint32 M_ADD_NICK								= 0x2220;
const uint32 M_NICK_ADDED							= 0x2221;
const uint32 M_NICK_UP								= 0x2222;
const uint32 M_NICK_DOWN							= 0x2223;
const uint32 M_NICK_SELECTED						= 0x2224;
const uint32 M_USE_NICK_DEFAULTS					= 0x2225;
const uint32 M_SETUP_CHOOSE_NETWORK					= 0x2226;
const uint32 M_SETUP_CLOSE							= 0x2227;
const uint32 M_SETUP_SHOW							= 0x2228;
const uint32 M_NETWORK_SHOW							= 0x2229;
const uint32 M_NETWORK_CLOSE						= 0x2230;
const uint32 M_PREFS_SHOW							= 0x2231;
const uint32 M_PREFS_CLOSE							= 0x2232;
const uint32 M_CONNECT_NETWORK						= 0x2233;
const uint32 M_SERVER_USEPASS						= 0x2234;
const uint32 M_NETPREFS_TEXT_INVOKE					= 0x2235;

// general prefs

const uint32 M_APPWINDOWPREFS_SETTING_CHANGED		= 0x2300;
const uint32 M_APPWINDOWPREFS_ENCODING_CHANGED		= 0x2301;
const uint32 M_REVERT_COLOR_SELECTIONS				= 0x2302;
const uint32 M_COMMAND_MODIFIED						= 0x2303;
const uint32 M_BLOCK_SIZE_CHANGED					= 0x2304;
const uint32 M_DEFAULT_PATH_CHANGED					= 0x2305;
const uint32 M_AUTO_ACCEPT_CHANGED					= 0x2306;
const uint32 M_DCC_MIN_PORT_CHANGED					= 0x2307;
const uint32 M_DCC_MAX_PORT_CHANGED					= 0x2308;
const uint32 M_EVENT_MODIFIED						= 0x2309;
const uint32 M_FONT_ELEMENT_CHANGE					= 0x2310;
const uint32 M_FONT_CHANGE							= 0x2311;
const uint32 M_FONT_SIZE_CHANGE						= 0x2312;
const uint32 M_GENERALPREFS_SELECTION_CHANGED		= 0x2313;
const uint32 M_PREFLOG_CHECKBOX_CHANGED				= 0x2314;
const uint32 M_PREFLOG_LOGPATH_CHANGED				= 0x2315;
const uint32 M_PREFLOG_TS_FORMAT_CHANGED			= 0x2316;
const uint32 M_DCC_PRIVATE_CHANGED					= 0x2317;

// misc

const uint32 M_PROMPT_DONE							= 0x2400;
const uint32 M_PROMPT_CANCEL						= 0x2401;
const uint32 M_OFFVIEW_SELECTION					= 0x2405;
const uint32 M_LOAD_URL								= 0x2406;

// network manager
const uint32 M_CREATE_CONNECTION					= 0x2500;
const uint32 M_CREATE_LISTENER						= 0x2501;
const uint32 M_DESTROY_CONNECTION					= 0x2502;
const uint32 M_SEND_CONNECTION_DATA					= 0x2503;
const uint32 M_CONNECTION_DATA_RECEIVED				= 0x2504;
const uint32 M_CONNECTION_DISCONNECT				= 0x2505;
const uint32 M_CONNECTION_CREATED					= 0x2506;
const uint32 M_LISTENER_CREATED						= 0x2507;
const uint32 M_CONNECTION_ACCEPTED					= 0x2508;

#endif // _VISIONMESSAGES_H_
