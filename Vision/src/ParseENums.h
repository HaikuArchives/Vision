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
 *								 Bjorn Oksholen
 */

#ifndef _PARSEENUMS_H_
#define _PARSEENUMS_H_


const int ZERO												= 0;

// NUMERICS

const int RPL_WELCOME								 =	 1;
const int RPL_YOURHOST								=	 2;
const int RPL_CREATED								 =	 3;
const int RPL_MYINFO									=	 4;
const int RPL_PROTOCTL								=	 5; // conflict (most)
const int RPL_NNMAP									 =	 5; // conflict (newnet)
const int RPL_ULMAP									 =	 6;
const int RPL_ULMAPEND								=	 7;
const int RPL_U2MAP									 =	15;
const int RPL_U2MAPEND								=	17;

const int RPL_TRACELINK							 = 200;
const int RPL_TRACECONNECTING				 = 201;
const int RPL_TRACEHANDSHAKE					= 202;
const int RPL_TRACEUNKNOWN						= 203;
const int RPL_TRACEOPERATOR					 = 204;
const int RPL_TRACEUSER							 = 205;
const int RPL_TRACESERVER						 = 206;
const int RPL_TRACENEWTYPE						= 208;
const int RPL_TRACECLASS							= 209;
const int RPL_STATSLINKINFO					 = 211;
const int RPL_STATSCOMMANDS					 = 212;
const int RPL_STATSCLINE							= 213;
const int RPL_STATSNLINE							= 214;
const int RPL_STATSILINE							= 215;
const int RPL_STATSKLINE							= 216;
const int RPL_STATSQLINE							= 217;
const int RPL_STATSYLINE							= 218;
const int RPL_ENDOFSTATS							= 219;
const int RPL_STATSBLINE							= 220;
const int RPL_UMODEIS								 = 221;
const int RPL_DALSTATSE							 = 223;
const int RPL_DALSTATSF							 = 224;
const int RPL_DALSTATSZ							 = 225;
const int RPL_DALSTATSN							 = 226;
const int RPL_DALSTATSG							 = 227;
const int RPL_SERVICEINFO						 = 231;
const int RPL_ENDOFSERVICES					 = 232;
const int RPL_SERVICE								 = 233;
const int RPL_SERVLIST								= 234;
const int RPL_SERVLISTEND						 = 235;
const int RPL_STATSLLINE							= 241;
const int RPL_STATSUPTIME						 = 242;
const int RPL_STATSOLINE							= 243;
const int RPL_STATSHLINE							= 244;
const int RPL_STATSSLINE							= 245;
const int RPL_DALSTATSX							 = 246;
const int RPL_STATSXLINE							= 247;
const int RPL_STATSPLINE							= 249;
const int RPL_LUSERHIGHESTCONN				= 250;
const int RPL_LUSERCLIENT						 = 251;
const int RPL_LUSEROP								 = 252;
const int RPL_LUSERUNKNOWN						= 253;
const int RPL_LUSERCHANNELS					 = 254;
const int RPL_LUSERME								 = 255;
const int RPL_ADMINME								 = 256;
const int RPL_ADMINLOC1							 = 257;
const int RPL_ADMINLOC2							 = 258;
const int RPL_ADMINEMAIL							= 259;
const int RPL_TRACELOG								= 261;
const int RPL_ENDOFTRACE							= 262;
const int RPL_TRYAGAIN								= 263;
const int RPL_LUSERLOCAL							= 265;
const int RPL_LUSERGLOBAL						 = 266;
const int RPL_SILELIST								= 271;
const int RPL_ENDOFSILELIST					 = 272;
const int RPL_290										 = 290;
const int RPL_291										 = 291;
const int RPL_292										 = 292;

const int RPL_NONE										= 300;
const int RPL_AWAY										= 301;
const int RPL_USERHOST								= 302;
const int RPL_ISON										= 303;
const int RPL_UNAWAY									= 305;
const int RPL_NOWAWAY								 = 306;
const int RPL_WHOISIDENTIFIED				 = 307;	// conflict
const int RPL_U2USERIP								= 307;	// conflict 
const int RPL_WHOISADMIN							= 308;
const int RPL_WHOISSERVICESADMIN			= 309;
const int RPL_WHOISHELPOP						 = 310;
const int RPL_WHOISUSER							 = 311;
const int RPL_WHOISSERVER						 = 312;
const int RPL_WHOISOPERATOR					 = 313;
const int RPL_WHOWASUSER							= 314;
const int RPL_ENDOFWHO								= 315;
const int RPL_WHOISCHANOP						 = 216;
const int RPL_WHOISIDLE							 = 317;
const int RPL_ENDOFWHOIS							= 318;
const int RPL_WHOISCHANNELS					 = 319;
const int RPL_WHOISREGNICK						= 320;
const int RPL_LISTSTART							 = 321;
const int RPL_LIST										= 322;
const int RPL_LISTEND								 = 323;
const int RPL_CHANNELMODEIS					 = 324;
const int RPL_CHANNELMLOCK						= 325;
const int RPL_CHANSERVURL						 = 328;
const int RPL_CHANNELCREATED					= 329;
const int RPL_NOTOPIC								 = 331;
const int RPL_TOPIC									 = 332;
const int RPL_TOPICSET								= 333;
const int RPL_COMMANDSYNTAX					 = 334;
const int RPL_WHOISACTUALLY					 = 338;
const int RPL_INVITING								= 341;
const int RPL_SUMMONING							 = 342;
const int RPL_VERSION								 = 351;
const int RPL_WHOREPLY								= 352;
const int RPL_NAMEREPLY							 = 353;
const int RPL_KILLDONE								= 361;
const int RPL_CLOSING								 = 362;
const int RPL_CLOSEEND								= 363;
const int RPL_LINKS									 = 364;
const int RPL_ENDOFLINKS							= 365;
const int RPL_ENDOFNAMES							= 366;
const int RPL_BANLIST								 = 367;
const int RPL_ENDOFBANLIST						= 368;
const int RPL_ENDOFWHOWAS						 = 369;
const int RPL_INFO										= 371;
const int RPL_MOTD										= 372;
const int RPL_INFOSTART							 = 373;
const int RPL_ENDOFINFO							 = 374;
const int RPL_MOTDSTART							 = 375;
const int RPL_ENDOFMOTD							 = 376;
const int RPL_MOTDALT								 = 378;
const int RPL_YOUREOPER							 = 381;
const int RPL_REHASHING							 = 382;
const int RPL_MYPORTIS								= 384;
const int RPL_TIME										= 391;
const int RPL_USERSSTART							= 392;
const int RPL_USERS									 = 393;
const int RPL_ENDOFUSERS							= 394;
const int RPL_NOUSERS								 = 395;					 


const int ERR_NOSUCHNICK							= 401;
const int ERR_NOSUCHSERVER						= 402;
const int ERR_NOSUCHCHANNEL					 = 403;
const int ERR_CANNOTSENDTOCHAN				= 404;
const int ERR_TOOMANYCHANNELS				 = 405;
const int ERR_WASNOSUCHNICK					 = 406;
const int ERR_TOOMANYTARGETS					= 407;
const int ERR_NOCOLORSONCHAN					= 408;
const int ERR_NOORIGIN								= 409;
const int ERR_NORECIPIENT						 = 411;
const int ERR_NOTEXTTOSEND						= 412;
const int ERR_NOTOPLEVEL							= 413;
const int ERR_WILDTOPLEVEL						= 414;
const int ERR_UNKNOWNCOMMAND					= 421;
const int ERR_NOMOTD									= 422;
const int ERR_NOADMININFO						 = 423;
const int ERR_FILEERROR							 = 424;
const int ERR_TOOMANYAWAY						 = 429;
const int ERR_NONICKNAMEGIVEN				 = 431;
const int ERR_ERRONEOUSNICKNAME			 = 432;
const int ERR_NICKNAMEINUSE					 = 433;
const int ERR_NICKCOLLISION					 = 436;
const int ERR_RESOURCEUNAVAILABLE		 = 437;
const int ERR_NICKCHANGETOOFAST			 = 438;
const int ERR_TARGETCHANGETOOFAST		 = 439;
const int ERR_USERNOTINCHANNEL				= 441;
const int ERR_NOTONCHANNEL						= 442;
const int ERR_USERONCHANNEL					 = 443;
const int ERR_NOLOGIN								 = 444;
const int ERR_SUMMONDISABLED					= 445;
const int ERR_USERSDISABLED					 = 446;
const int ERR_NOTREGISTERED					 = 451;
const int ERR_YOUCANTDOTHAT					 = 460;
const int ERR_NEEDMOREPARMS					 = 461;
const int ERR_ALREADYREGISTERED			 = 462;
const int ERR_NOPERMFORHOST					 = 463;
const int ERR_PASSWDMISMATCH					= 464;
const int ERR_YOUREBANNEDCREEP				= 465;
const int ERR_YOUWILLBEBANNED				 = 466;
const int ERR_KEYSET									= 467;
const int ERR_CHANNELISFULL					 = 471;
const int ERR_UNKNOWNMODE						 = 472;
const int ERR_INVITEONLYCHAN					= 473;
const int ERR_BANNEDFROMCHAN					= 474;
const int ERR_BADCHANNELKEY					 = 475;
const int ERR_BADCHANMASK						 = 476;
const int ERR_NOPRIVILEGES						= 481;
const int ERR_CHANOPRIVSNEEDED				= 482;
const int ERR_CANTKILLSERVER					= 483;
const int ERR_NOOPERHOST							= 491;
const int ERR_NOSERVICEHOST					 = 492;

const int ERR_UMODEUNKNOWNFLAG				= 501;
const int ERR_USERSDONTMATCH					= 502;
const int ERR_SILELISTFULL						= 511;
const int ERR_TOOMANYWATCH						= 512;
const int ERR_TOOMANYDCC							= 514;
const int ERR_CANTINVITE							= 518;
const int ERR_LISTSYNTAX							= 521;
const int ERR_WHOSYNTAX							 = 522;
const int ERR_WHOLIMEXCEED						= 523;

const int RPL_WHOISMASK							 = 550; // Sorcery.net host masking
const int RPL_LOGON									 = 600;
const int RPL_LOGOFF									= 601;
const int RPL_WATCHOFF								= 602;
const int RPL_WATCHSTAT							 = 603;
const int RPL_NOWON									 = 604;
const int RPL_NOWOFF									= 605;
const int RPL_WATCHLIST							 = 606;
const int RPL_ENDOFWATCHLIST					= 607;
const int RPL_OPERMOTDSTART					 = 609; // PTLink
const int RPL_OPERMOTD								= 610; // possibly others
const int RPL_OPERENDOFMOTD					 = 611;
const int RPL_WHOWASIP								= 612; 
const int RPL_WHOISUSERMODESALT			 = 614; // user modes on PTLink
const int RPL_WHOISUSERMODES					= 615;
const int RPL_WHOISREALHOSTNAME			 = 616;
const int RPL_WHOISREGISTEREDBOT			= 617; // conflict (ultimate)
const int RPL_DCCALLOWCHANGE					= 617; // conflict (bahamut)
const int RPL_DCCALLOWLIST						= 618;
const int RPL_DCCALLOWEND						 = 619;
const int RPL_DCCALLOW								= 620;

#endif
