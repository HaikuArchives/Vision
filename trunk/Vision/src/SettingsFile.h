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
 */

#ifndef _SETTINGSFILE_H_
#define _SETTINGSFILE_H_

#include <Message.h>
#include <FindDirectory.h>
#include <Path.h>

class BFile;

struct attr_info;

class SettingsFile : public BMessage {
public :
	SettingsFile(char const*leafname=NULL,char const*basename=NULL,directory_which dir=B_USER_SETTINGS_DIRECTORY);

	status_t InitCheck() const;
	
	status_t Load();
	status_t Save() const;

private:
	static status_t _StoreAttributes(BMessage const*m,BFile*f,const char*basename="");
	static status_t _ExtractAttribute(BMessage*m,BFile*f,const char*full_name,char*partial_name,attr_info*ai);

	status_t check;
	BPath path;
};

#endif

