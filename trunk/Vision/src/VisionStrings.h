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

#ifndef _VISIONSTRINGS_H_
#define _VISIONSTRINGS_H_

// channel agent

#define S_CHANNEL_INIT              "*** Now talking in "
#define S_CHANNEL_REJOIN            "*** Attempting to rejoin"
#define S_CHANNEL_RECON_REJOIN      "[@] Attempting to rejoin"
#define S_CHANNEL_GOT_KICKED        "*** You have been kicked from "
#define S_CHANNEL_GOT_KICKED2       "by"
#define S_CHANNEL_SET_MODE          " set mode "

// channel options

#define S_CHANOPTS_TITLE            " Options"
#define S_CHANOPTS_OPID1            "You are currently a channel operator. "
#define S_CHANOPTS_OPID2            "You may change any of these options."

// client agent

#define S_CLIENT_DCC_SUCCESS        "Completed "
#define S_CLIENT_DCC_FAILED         "Terminated "
#define S_CLIENT_DCC_SENDTYPE       "send of "
#define S_CLIENT_DCC_RECVTYPE       "receive of "
#define S_CLIENT_DCC_TO             " to "
#define S_CLIENT_DCC_FROM           " from "
#define S_CLIENT_DCC_SIZE_UNITS     " bytes"
#define S_CLIENT_DCC_SPEED_UNITS    " cps"

// client window

#define S_CW_SERVER_MENU            "Server"
#define S_CW_SERVER_CONNECT         "Connect to"
#define S_CW_SERVER_SETUP           "Setup"
#define S_CW_EDIT_MENU              "Edit"
#define S_CW_TOOLS_MENU             "Tools"
#define S_CW_TOOLS_CHANLIST         "Channel List"
#define S_CW_TOOLS_NOTIFYLIST       "Notify List"
#define S_CW_TOOLS_IGNORELIST       "Ignore List"
#define S_CW_TOOLS_TERMINAL         "New Terminal"
#define S_CW_WINDOW_MENU            "Window"
#define S_CW_WINDOW_PART            "Part Agent"

// client window dock

#define S_CWD_WINLIST_HEADER        "Window List"
#define S_CWD_NOTIFY_HEADER         "Notify List"

// DCC messages

#define S_DCC_SPEED                 "bps: "
#define S_DCC_ESTABLISH_ERROR       "Unable to establish connection."
#define S_DCC_ESTABLISH_SUCCEEDED   "Established connection."
#define S_DCC_CONNECT_TO_SENDER     "Connecting to sender."
#define S_DCC_RECV1                 "Receiving \""
#define S_DCC_RECV2                 "\" from "
#define S_DCC_SEND1                 "Sending \""
#define S_DCC_SEND2                 "\" to "
#define S_DCC_LOCK_ACQUIRE          "Acquiring DCC lock"
#define S_DCC_ACK_WAIT              "Waiting for acceptance"
#define S_DCC_LISTEN_CALL           "Doing listen call."
#define S_DCC_WAIT_FOR_CONNECTION   "Waiting for connection "
#define S_DCC_WRITE_ERROR           "Error writing data."
#define S_DCC_SOCKET_ERROR          "Error creating socket.\n"
#define S_DCC_BIND_ERROR            "Error binding socket.\n"
#define S_DCC_CONN_ERROR            "Error connecting socket.\n"
#define S_DCC_CHAT_LISTEN           "Accepting connection on address "
#define S_DCC_CHAT_TRY              "Trying to connect to address "
#define S_DCC_CHAT_PORT             ", port "
#define S_DCC_CHAT_CONNECTED        "Connected!\n"
#define S_DCC_CHAT_TERM             "DCC Chat Terminated.\n"

// list agent

#define S_LIST_MENU                 "Channels"
#define S_LIST_MENU_FIND            "Find"
#define S_LIST_MENU_FINDNEXT        "Find Next"
#define S_LIST_MENU_FILTER          "Filter"
#define S_LIST_COLUMN_CHAN          "Channel"
#define S_LIST_COLUMN_USER          "Users"
#define S_LIST_COLUMN_TOPIC         "Topic"
#define S_LIST_STATUS_LOADING       "Loading"
#define S_LIST_STATUS_DONE          "Done"
#define S_LIST_PROMPT_TITLE         "Find"
#define S_LIST_PROMPT_LABEL         "    Find:"

// network prefs -- server list

#define S_PREFSERVER_STATUS_COLUMN  "Status"
#define S_PREFSERVER_SERVER_COLUMN  "Server"
#define S_PREFSERVER_PORT_COLUMN    "Port"
#define S_PREFSERVER_ADD_BUTTON     "Add"
#define S_PREFSERVER_REMOVE_BUTTON  "Remove"
#define S_PREFSERVER_EDIT_BUTTON    "Edit"
#define S_PREFSERVER_DESC1          "Key: "
#define S_PREFSERVER_DESC2          "  * = primary"
#define S_PREFSERVER_DESC3          "  + = secondary (fallback)"
#define S_PREFSERVER_DESC4          "  - = disabled"
#define S_PREFSERVER_OK_BUTTON      "OK"
#define S_PREFSERVER_SEL_STRING     "Select servers for "

// network prefs -- main view

#define S_NETPREFS_NETMENU          "Networks"
#define S_NETPREFS_DEFAULTS_ITEM    "Defaults"
#define S_NETPREFS_ADD_NEW          "Add New"
#define S_NETPREFS_REMOVE           "Remove current"
#define S_NETPREFS_DUPE             "Duplicate current"
#define S_NETPREFS_NET_BOX          "Network Details"
#define S_NETPREFS_PERSONAL_BOX     "Personal Details"
#define S_NETPREFS_CONN1            "Will connect to"
#define S_NETPREFS_CONN2            "falling back to 9 others."
#define S_NETPREFS_CHANGE_SERVER    "Change servers"
#define S_NETPREFS_AUTOEXEC         "Autoexec:"
#define S_NETPREFS_LAG_CHECK        "Enable lag checking"
#define S_NETPREFS_STARTUP_CONN     "Connect to this network when Vision starts up"
#define S_NETPREFS_USE_DEFAULTS     "Use Defaults"
#define S_NETPREFS_PREFNICK         "Preferred Nicks:"
#define S_NETPREFS_ADD_BUTTON       "Add"
#define S_NETPREFS_REMOVE_BUTTON    "Remove"
#define S_NETPREFS_IDENT            "Ident: "
#define S_NETPREFS_REALNAME         "Real name: "
#define S_NETPREFS_FALLBACK1        "falling back to "
#define S_NETPREFS_FALLBACK2        " other"
#define S_NETPREFS_FALLBACK2_PLURAL "s"
#define S_NETPREFS_ADDNET_TITLE     "Add Network"
#define S_NETPREFS_DUPENET_TITLE    "Duplicate Network"
#define S_NETPREFS_NET_PROMPT       "Network Name: "
#define S_NETPREFS_ADDNICK_TITLE    "Add Nickname"
#define S_NETPREFS_ADDNICK_PROMPT   "Nickname: "

// network windows

#define S_NETWORK_WINDOW_TITLE      "Network Setup"
#define S_SERVERPREFS_TITLE         "Servers"

// command parser

#define S_PCMD_PARAMETER_ERROR      "Error: Invalid parameters"
#define S_PCMD_SEND_TITLE           "Sending a file to "
#define S_PCMD_SEND_BUTTON          "Send"
#define S_PCMD_TRY_NEW_NICK         "*** Trying new nick "
#define S_PCMD_SET_BOOL_SUCCESS     "Bool has been set"
#define S_PCMD_SET_BOOL_FAILURE     "Error setting bool"
#define S_PCMD_DNS1                 "Resolved "
#define S_PCMD_DNS2                 " to "
#define S_PCMD_DNS_ERROR            "Unable to resolve "

// ctcp parser

#define S_PCTCP_RESPONSE            " response"
#define S_PCTCP_SECOND              "second"
#define S_PCTCP_SECOND_PLURAL       "seconds"

// numerics parser

#define S_PENUM_CURMODE             "[x] your current mode is: "
#define S_PENUM_WHOWAS              "[was]"
#define S_PENUM_IDLE                "Idle: "
#define S_PENUM_SIGNON              "Signon: "
#define S_PENUM_WHOIS_CHANNELS      "[x] Channels: "
#define S_PENUM_CHANMODE            "*** Channel mode for "
#define S_PENUM_MLOCK               "*** Channel mode lock for "
#define S_PENUM_CHANCREATED         "created"
#define S_PENUM_NO_TOPIC            "[x] No topic set in "
#define S_PENUM_DISP_TOPIC          "*** Topic: "
#define S_PENUM_TOPIC_SET_BY        "*** Topic set by "
#define S_PENUM_INVITING            " has been invited to "
#define S_PENUM_NAMEREPLY           "*** Users in "
#define S_PENUM_SERVER_MOTD         "- Server Message Of The Day:"
#define S_PENUM_RECON_SUCCESS       "[@] Successful reconnect"
#define S_PENUM_NICKINUSE1          "* Nickname \""
#define S_PENUM_NICKINUSE2          "\" in use or unavailable.. trying \""
#define S_PENUM_NICKINUSE3          "[x] Nickname/Channel "
#define S_PENUM_NICKINUSE4          " is already in use or unavailable."
#define S_PENUM_ALLNICKSUSED1       "* All your pre-selected nicknames are in use."
#define S_PENUM_ALLNICKSUSED2       "* Please type /NICK <NEWNICK> to try another."
#define S_PENUM_NOTINCHANNEL        " is not in "
#define S_PENUM_ME_NOTINCHANNEL     "[x] You're not in "
#define S_PENUM_ALREADYINCHANNEL    " is already in "
#define S_PENUM_KEY_ALREADY_SET     "[x] Channel key already set in "
#define S_PENUM_UNKNOWNCHANMODE     "[x] Unknown channel mode: '"
#define S_PENUM_INVITE_ONLY         " (invite only)"
#define S_PENUM_BANNED              " (you're banned)"
#define S_PENUM_BADCHANKEY          " (bad channel key)"
#define S_PENUM_UNKNOWNMODE         "[x] Unknown MODE flag."

// events parser

#define S_PEVENTS_UMODE_CHANGE      "*** User mode changed: "
#define S_PEVENTS_INVITE1           "*** You have been invited to "
#define S_PEVENTS_INVITE2           " by "
#define S_PEVENTS_SILENCE_ADDED     "*** Hostmask added to SILENCE list: "
#define S_PEVENTS_SILENCE_REMOVED   "*** Hostmask removed from SILENCE list: "

// app prefs

#define S_PREFAPP_VERSION_PARANOID  "Show OS information in version reply"
#define S_PREFAPP_CMDW              "Cmd+W closes Vision"
#define S_PREFAPP_SHOW_TIMESTAMP    "Show timestamps in IRC window"
#define S_PREFAPP_USE_LOGGING       "Enable logging"
#define S_PREFAPP_LOG_TIMESTAMP     "Append timestamp to log filenames"
#define S_PREFAPP_STRIP_MIRC        "Strip mIRC Colors"
#define S_PREFAPP_WARN_MULTILINE    "Warn when multiline pasting"

// color prefs

#define S_PREFCOLOR_TEXT            "Text"
#define S_PREFCOLOR_BACKGROUND      "Background"
#define S_PREFCOLOR_URL             "URL"
#define S_PREFCOLOR_SERVERTEXT      "Server Text"
#define S_PREFCOLOR_NOTICE          "Notice"
#define S_PREFCOLOR_ACTION          "Action"
#define S_PREFCOLOR_QUIT            "Quit"
#define S_PREFCOLOR_ERROR           "Error"
#define S_PREFCOLOR_NICK_EDGES      "Nickname edges"
#define S_PREFCOLOR_UNICK_EDGES     "User nickname edges"
#define S_PREFCOLOR_NICK_TEXT       "Nickname text"
#define S_PREFCOLOR_JOIN            "Join"
#define S_PREFCOLOR_KICK            "Kick"
#define S_PREFCOLOR_WHOIS           "Whois"
#define S_PREFCOLOR_NAMES_NORM      "Names (Normal)"
#define S_PREFCOLOR_NAMES_OP        "Names (Op)"
#define S_PREFCOLOR_NAMES_HELP      "Names (Helper)"
#define S_PREFCOLOR_NAMES_VOICE     "Names (Voice)"
#define S_PREFCOLOR_NAMES_SEL       "Names selection"
#define S_PREFCOLOR_NAMES_BG        "Names Background"
#define S_PREFCOLOR_CTCP_REQ        "CTCP Request"
#define S_PREFCOLOR_CTCP_RPY        "CTCP Reply"
#define S_PREFCOLOR_IGNORE          "Ignore"
#define S_PREFCOLOR_INPUT_TXT       "Input text"
#define S_PREFCOLOR_INPUT_BG        "Input background"
#define S_PREFCOLOR_WLIST_NORM      "Winlist normal status"
#define S_PREFCOLOR_WLIST_TXT       "Winlist text status"
#define S_PREFCOLOR_WLIST_NICK      "Winlist nick alert status"
#define S_PREFCOLOR_WLIST_SEL       "Winlist selection status"
#define S_PREFCOLOR_WLIST_EVT       "Winlist event status"
#define S_PREFCOLOR_WLIST_BG        "Winlist background"
#define S_PREFCOLOR_WALLOPS         "Wallops"
#define S_PREFCOLOR_TIMESTAMP       "Timestamp"
#define S_PREFCOLOR_TIMESTAMP_BG    "Timestamp background"
#define S_PREFCOLOR_SELECTION       "Selection"
#define S_PREFCOLOR_MIRCWHITE       "mIRC White"
#define S_PREFCOLOR_MIRCBLACK       "mIRC Black"
#define S_PREFCOLOR_MIRCDBLUE       "mIRC Dark Blue"
#define S_PREFCOLOR_MIRCGREEN       "mIRC Green"
#define S_PREFCOLOR_MIRCRED         "mIRC Red"
#define S_PREFCOLOR_MIRCBROWN       "mIRC Brown"
#define S_PREFCOLOR_MIRCPURPLE      "mIRC Purple"
#define S_PREFCOLOR_MIRCORANGE      "mIRC Orange"      
#define S_PREFCOLOR_MIRCYELLOW      "mIRC Yellow"
#define S_PREFCOLOR_MIRCLIME        "mIRC Lime"
#define S_PREFCOLOR_MIRCTEAL        "mIRC Teal"
#define S_PREFCOLOR_MIRCAQUA        "mIRC Aqua"
#define S_PREFCOLOR_MIRCLBLUE       "mIRC Light Blue"
#define S_PREFCOLOR_MIRCPINK        "mIRC Pink"
#define S_PREFCOLOR_MIRCGREY        "mIRC Grey"
#define S_PREFCOLOR_MIRCSILVER      "mIRC Silver"
#define S_PREFCOLOR_REVERT          "Revert"

// command prefs

#define S_PREFCOMMAND_QUIT          "Quit:"
#define S_PREFCOMMAND_KICK          "Kick:"
#define S_PREFCOMMAND_IGNORE        "Ignore:"
#define S_PREFCOMMAND_UNIGNORE      "Unignore:"
#define S_PREFCOMMAND_AWAY          "Away:"
#define S_PREFCOMMAND_BACK          "Back:"
#define S_PREFCOMMAND_UPTIME        "Uptime:"

// dcc prefs

#define S_PREFDCC_BLOCK_SIZE        "DCC Block Size: "
#define S_PREFDCC_AUTOACK           "Automatically accept incoming sends"
#define S_PREFDCC_DEFPATH           "Default path: "
#define S_PREFDCC_PORTRANGE         "DCC Port Range"
#define S_PREFDCC_PORTMIN           "Min: "
#define S_PREFDCC_PORTMAX           "Max: "

// event prefs

#define S_PREFEVENT_JOIN            "Join:"
#define S_PREFEVENT_PART            "Part:"
#define S_PREFEVENT_NICK            "Nick:"
#define S_PREFEVENT_QUIT            "Quit:"
#define S_PREFEVENT_KICK            "Kick:"
#define S_PREFEVENT_TOPIC           "Topic:"
#define S_PREFEVENT_SNOTICE         "Server Notice:"
#define S_PREFEVENT_UNOTICE         "User Notice:"
#define S_PREFEVENT_NOTIFYON        "Notify On:"
#define S_PREFEVENT_NOTIFYOFF       "Notify Off:"

// font prefs

#define S_PREFFONT_TEXT             "Text"
#define S_PREFFONT_SMESSAGES        "Server Messages"
#define S_PREFFONT_URLS             "URLs"
#define S_PREFFONT_NAMESLIST        "Names list"
#define S_PREFFONT_INPUT_TEXT       "Input text"
#define S_PREFFONT_WINLIST          "Window List"
#define S_PREFFONT_CHANLIST         "Channel List"
#define S_PREFFONT_TSTAMP           "Timestamp"
#define S_PREFFONT_FONTLABEL        "Font: "
#define S_PREFFONT_SIZELABEL        "Size: "

// main prefs view

#define S_PREFGEN_APP_ITEM          "Application"
#define S_PREFGEN_COLOR_ITEM        "Colors"
#define S_PREFGEN_FONT_ITEM         "Fonts"
#define S_PREFGEN_COMMAND_ITEM      "Commands"
#define S_PREFGEN_EVENT_ITEM        "Events"
#define S_PREFGEN_DCC_ITEM          "DCC"

// preferences window

#define S_PREFSWIN_TITLE            "Preferences"

// server agent

#define S_SERVER_ATTEMPT1           "[@] Attempting to "
#define S_SERVER_ATTEMPT2           "re"
#define S_SERVER_ATTEMPT3           "connect (attempt "
#define S_SERVER_ATTEMPT4           " of "
#define S_SERVER_ATTEMPT5           "[@] Attempting a connection to "
#define S_SERVER_CONN_ERROR1        "[@] Could not create connection to address and port. Make sure your Internet connection is operational."
#define S_SERVER_CONN_ERROR2        "[@] Could not establish a connection to the server. Sorry."
#define S_SERVER_CONN_OPEN          "[@] Connection open, waiting for reply from server"
#define S_SERVER_LOCALIP_ERROR      "[@] Error getting Local IP"
#define S_SERVER_LOCALIP            "[@] Local IP: "
#define S_SERVER_PROXY_MSG          "[@] (It looks like you are behind an Internet gateway. Vision will query the IRC server upon successful connection for your gateway's Internet address. This will be used for DCC communication.)"
#define S_SERVER_HANDSHAKE          "[@] Handshaking"
#define S_SERVER_ESTABLISH          "[@] Established"
#define S_SERVER_RETRY_LIMIT        "[@] Retry limit reached; giving up. Type /reconnect if you want to give it another go."
#define S_SERVER_DISCONNECT         "[@] Disconnected from "
#define S_SERVER_DISCON_STATUS      "Disconnected"
#define S_SERVER_CONN_PROBLEM       "CONNECTION PROBLEM"
#define S_SERVER_LAG_DISABLED       "Disabled"
#define S_SERVER_DCC_CHAT_PROMPT    " wants to begin a DCC chat with you."

// server entry window

#define S_SERVERWIN_TITLE           "Add Server"
#define S_SERVERWIN_SERVER          "Server: "
#define S_SERVERWIN_PORT            "Port: "
#define S_SERVERWIN_MENU1           "Choose status"
#define S_SERVERWIN_MENU_PRI        "Primary"
#define S_SERVERWIN_MENU_SEC        "Secondary"
#define S_SERVERWIN_MENU_DIS        "Disabled"
#define S_SERVERWIN_STATE           "State: "
#define S_SERVERWIN_DONE_BUTTON     "Done"
#define S_SERVERWIN_CANCEL_BUTTON   "Cancel"
// setup window

#define S_SETUP_TITLE               "Setup Window"
#define S_SETUP_CONNECT_BUTTON      "Connect"
#define S_SETUP_NETPREFS            "Network Setup"
#define S_SETUP_GENPREFS            "Preferences"
#define S_SETUP_CHOOSENET           "Choose Network"
#define S_SETUP_CHOOSELABEL         "Network: "

// status bar

#define S_STATUS_LAG                "Lag: "
#define S_STATUS_USERS              "Users: "
#define S_STATUS_OPS                "Ops: "
#define S_STATUS_MODES              "Modes: "
#define S_STATUS_LISTCOUNT          "Count: "
#define S_STATUS_LISTSTAT           "Status: "
#define S_STATUS_LISTFILTER         "Filter: "

// window list

#define S_WINLIST_CLOSE_ITEM        "Close"	

#endif _VISIONSTRINGS_H_
