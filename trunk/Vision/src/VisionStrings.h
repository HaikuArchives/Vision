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
#define S_CHANNEL_RECON_REJOIN      "[@] Attempting to rejoin\n"
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

// status bar

#define S_STATUS_LAG                "Lag: "
#define S_STATUS_USERS              "Users: "
#define S_STATUS_OPS                "Ops: "
#define S_STATUS_MODES              "Modes: "
#define S_STATUS_LISTCOUNT          "Count: "
#define S_STATUS_LISTSTAT           "Status: "
#define S_STATUS_LISTFILTER         "Filter: "


#endif _VISIONSTRINGS_H_
