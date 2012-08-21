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
 */

#include "SettingsFile.h"
#include "Vision.h" // for debug output

#include <Path.h>
#include <File.h>
#include <Directory.h>
#include <Roster.h>
#include <Application.h>

#include <string.h>
#include <stdlib.h>
#include <fs_attr.h>
#include <stdio.h>
#include <errno.h>

SettingsFile::SettingsFile(char const*lname,char const*bname,directory_which d) {
	check=B_OK;
	if (d!=(directory_which)-1) { // -1 means "absolute path"
		if ((check=find_directory(d,&path))!=B_OK) return;
	} else {
		if ((check=path.SetTo("/"))!=B_OK) return;
	}
	if (bname==NULL) { // no base name, try to figure it out from the signature
		app_info ai;
		char*sig=ai.signature;
		if ((check=be_app->GetAppInfo(&ai))!=B_OK) return;
		int plen=strlen("application/x-vnd.");
		if (strncmp(sig,"application/x-vnd.",plen)) {
			plen=strlen("application/");
			if (strncmp(sig,"application/",plen)) {
				check=B_BAD_VALUE;
				return; // the signature is really broken. bail out.
			}
		}
		sig+=plen;
		bool founddot=false;
		char*sep;
		while ((sep=strchr(sig,'.'))!=NULL) { // replace each '.' by a '/' in the signature to build a relative path
			*sep='/';
			founddot=true;
		}
		if (!founddot&&((sep=strchr(sig,'-'))!=NULL)) { // no '.' was found. replace the first '-' by a '/', if there's a '-'
			*sep='/';
		}
		if ((check=path.Append(sig))!=B_OK) { path.Unset();return; }
	} else {
		if ((check=path.Append(bname))!=B_OK) { path.Unset();return; }
	}

	if (lname==NULL) {
		if ((check=path.Append("Settings"))!=B_OK) { path.Unset();return; }
	} else {
		if ((check=path.Append(lname))!=B_OK) { path.Unset();return; }
	}
}

status_t SettingsFile::InitCheck() const {
	return check;
}

status_t SettingsFile::Load() {
	status_t ret;
	BFile file(path.Path(),B_READ_ONLY);
	ret=file.InitCheck();
	if (ret!=B_OK) {
		return ret;
	}
	ret=file.Lock();
	if (ret!=B_OK) {
		return ret;
	}
	ret=Unflatten(&file);
	if (ret!=B_OK) {
		file.Unlock();
		MakeEmpty();
		return ret;
	}

	if (vision_app->fDebugSettings)
		PrintToStream();
/*
	ret=file.RewindAttrs();
	if (ret!=B_OK) {
		file.Unlock();
		MakeEmpty();
		return ret;
	}
	char attr_name[B_ATTR_NAME_LENGTH];
	while ((ret=file.GetNextAttrName(attr_name))!=B_ENTRY_NOT_FOUND) { // walk all the attributes of the settings file
		if (ret!=B_OK) {
			file.Unlock();
			return ret;
		}
			// found an attribute
		attr_info ai;
		ret=file.GetAttrInfo(attr_name,&ai);
		if (ret!=B_OK) {
			file.Unlock();
			return ret;
		}
		switch (ai.type) {
			case B_CHAR_TYPE :
			case B_STRING_TYPE :
			case B_BOOL_TYPE :
			case B_INT8_TYPE :
			case B_INT16_TYPE :
			case B_INT32_TYPE :
			case B_INT64_TYPE :
			case B_UINT8_TYPE :
			case B_UINT16_TYPE :
			case B_UINT32_TYPE :
			case B_UINT64_TYPE :
			case B_FLOAT_TYPE :
			case B_DOUBLE_TYPE :
			case B_OFF_T_TYPE :
			case B_SIZE_T_TYPE :
			case B_SSIZE_T_TYPE :
			case B_POINT_TYPE :
			case B_RECT_TYPE :
			case B_RGB_COLOR_TYPE :
			case B_TIME_TYPE :
			case B_MIME_TYPE : {
				char*partial_name=strdup(attr_name);
				if (partial_name==NULL) {
					file.Unlock();
					return B_NO_MEMORY;
				}
				ret=_ExtractAttribute(this,&file,attr_name,partial_name,&ai);
				free(partial_name);
				if (ret!=B_OK) {
					file.Unlock();
					return ret;
				}
				break;
			}
		}
	}
*/
	file.Unlock();
	return B_OK;
}

status_t SettingsFile::_ExtractAttribute(BMessage*m,BFile*f,const char*full_name,char*partial_name,attr_info*ai) {
	status_t ret;
	char*end=strchr(partial_name,':');
	if (end==NULL) { // found a leaf
		if (!m->HasData(partial_name,ai->type)) { // the name does not exist in the message - ignore it
			return B_OK;
		}
		void* buffer=malloc(ai->size);
		if (buffer==NULL) { // cannot allocate space to hold the data
			return B_NO_MEMORY;
		}
		if (f->ReadAttr(full_name,ai->type,0,buffer,ai->size)!=ai->size) { // cannot read the data
			free(buffer);
			return B_IO_ERROR;
		}
		ret=m->ReplaceData(partial_name,ai->type,buffer,ai->size);
		if (ret!=B_OK) { // cannot replace the data
			free(buffer);
			return ret;
		}
		free(buffer);
		return B_OK;
	}
	if (end[1]!=':') { // found an un-numbered sub-message
		*(end++)='\0'; // zero-terminate the name, point to the rest of the sub-string
		if (!m->HasMessage(partial_name)) { // archived message does not contain that entry. go away.
			return B_OK;
		}
		BMessage subm;
		ret=m->FindMessage(partial_name,&subm); // extract the sub-message
		if (ret!=B_OK) {
			return ret;
		}
		ret=_ExtractAttribute(&subm,f,full_name,end,ai); // keep processing
		if (ret!=B_OK) {
			return ret;
		}
		ret=m->ReplaceMessage(partial_name,&subm); // replace the sub-message
		if (ret!=B_OK) {
			return ret;
		}
		return B_OK;
	} else { // found a numbered entry
		char* endptr;
		errno=0;
		*end='\0'; // zero-terminate the name
		int32 r=strtol(end+2,&endptr,10); // get the entry number
		if (errno!=0) {
			return B_OK;
		}
		if (r>=1000000000) { // sanity-check.
			return B_OK;
		}
		if (*endptr==':') { // this is a numbered message
			if (!m->HasMessage(partial_name,r)) { // archived message does not contain that entry, go away
				return B_OK;
			}
			BMessage subm;
			ret=m->FindMessage(partial_name,r,&subm); // extract the sub-message
			if (ret!=B_OK) {
				return ret;
			}
			ret=_ExtractAttribute(&subm,f,full_name,endptr+1,ai); // recurse
			if (ret!=B_OK) {
				return ret;
			}
			ret=m->ReplaceMessage(partial_name,r,&subm); // replace the sub-message
			if (ret!=B_OK) {
				return ret;
			}
			return B_OK;
		} else if (*endptr=='\0') { // this is a numbered leaf
			if (!m->HasData(partial_name,ai->type,r)) { // archived message does not contain this leaf
				return B_OK;
			}
			void* buffer=malloc(ai->size);
			if (buffer==NULL) {
				return B_NO_MEMORY;
			}
			if (f->ReadAttr(full_name,ai->type,0,buffer,ai->size)!=ai->size) { // extract the attribute data
				free(buffer);
				return B_IO_ERROR;
			}
			ret=m->ReplaceData(partial_name,ai->type,r,buffer,ai->size); // and replace it in the message
			if (ret!=B_OK) {
				free(buffer);
				return ret;
			}
			free(buffer);
			return B_OK;
		}
	}
	return B_OK;
}

status_t SettingsFile::Save() const {
	status_t ret;
	BFile file(path.Path(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
	ret=file.InitCheck();
	if (ret != B_OK) { // try to create the parent directory if creating the file fails the first time
		BPath parent;
		ret=path.GetParent(&parent);
		if (ret!=B_OK) {
			return ret;
		}
		ret=create_directory(parent.Path(),0777);
		if (ret!=B_OK) {
			return ret;
		}
		ret=file.SetTo(path.Path(),B_READ_WRITE|B_CREATE_FILE|B_ERASE_FILE);
	}
	if (ret!=B_OK) {
		return ret;
	}
	ret=file.Lock(); // lock the file to do atomic attribute transactions on it.
	if (ret!=B_OK) {
		return ret;
	}

	if (vision_app->fDebugSettings)
		PrintToStream();

	ret=Flatten(&file);
	if (ret!=B_OK) {
		file.Unlock();
		return ret;
	}
/*
	ret=_StoreAttributes(this,&file);
	if (ret!=B_OK) {
		file.Unlock();
		return ret;
	}
*/
	file.Unlock();
	return B_OK;
}

status_t SettingsFile::_StoreAttributes(BMessage const*m,BFile*f,const char*basename) {
#if B_BEOS_VERSION_DANO
		const char *namefound;
#else
	char* namefound;
#endif
	type_code typefound;
	int32 countfound;
	status_t ret;
	for (int32 i=0;i<m->CountNames(B_ANY_TYPE);i++) { // walk the entries in the message
		ret=m->GetInfo(B_ANY_TYPE,i,&namefound,&typefound,&countfound);
		if (ret!=B_OK) {
			return ret;
		}
		if (strchr(namefound,':')!=NULL) { // do not process anything that contains a colon (considered a magic char)
			break;
		}
		switch (typefound) {
			case B_MESSAGE_TYPE : { // found a sub-message
				if (countfound==1) { // single sub-message
					char* lname=(char*)malloc(strlen(basename)+strlen(namefound)+2); // allocate space for the base name
					if (lname==NULL) {
						return B_NO_MEMORY;
					}
					sprintf(lname,"%s%s:",basename,namefound); // create the base name for the sub-message
					BMessage subm;
					ret=m->FindMessage(namefound,&subm);
					if (ret!=B_OK) {
						free(lname);
						return ret;
					}
					ret=_StoreAttributes(&subm,f,lname); // and process the sub-message with the base name
					if (ret!=B_OK) {
						free(lname);
						return ret;
					}
					free(lname);
				} else if (countfound<1000000000) { // (useless in 32-bit) sanity check
					char* lname=(char*)malloc(strlen(basename)+strlen(namefound)+11); // allocate space for the base name
					if (lname==NULL) {
						return B_NO_MEMORY;
					}
					sprintf(lname,"%" B_PRId32,countfound-1); // find the length of the biggest number for that field
					char format[12];
					sprintf(format,"%%s%%s::%%0%ldld:",strlen(lname)); // create the sprintf format
					for (int32 j=0;j<countfound;j++) {
						sprintf(lname,format,basename,namefound,j); // create the base name for the sub-message
						BMessage subm;
						ret=m->FindMessage(namefound,j,&subm);
						if (ret!=B_OK) {
							free(lname);
							return ret;
						}
						ret=_StoreAttributes(&subm,f,lname); // process the sub-message with the base name
						if (ret!=B_OK) {
							free(lname);
							return ret;
						}
					}
					free(lname);
				}

				break;
			}
			case B_CHAR_TYPE :
			case B_STRING_TYPE :
			case B_BOOL_TYPE :
			case B_INT8_TYPE :
			case B_INT16_TYPE :
			case B_INT32_TYPE :
			case B_INT64_TYPE :
			case B_UINT8_TYPE :
			case B_UINT16_TYPE :
			case B_UINT32_TYPE :
			case B_UINT64_TYPE :
			case B_FLOAT_TYPE :
			case B_DOUBLE_TYPE :
			case B_OFF_T_TYPE :
			case B_SIZE_T_TYPE :
			case B_SSIZE_T_TYPE :
			case B_POINT_TYPE :
			case B_RECT_TYPE :
			case B_RGB_COLOR_TYPE :
			case B_TIME_TYPE :
			case B_MIME_TYPE : { // found a supported type. the code is basically the same.
				if (countfound==1) {
					char* lname=(char*)malloc(strlen(basename)+strlen(namefound)+1);
					if (lname==NULL) {
						return B_NO_MEMORY;
					}
					sprintf(lname,"%s%s",basename,namefound);
					const void* datafound;
					ssize_t sizefound;
					ret=m->FindData(namefound,typefound,&datafound,&sizefound);
					if (ret!=B_OK) {
						free(lname);
						return ret;
					}
					sizefound=f->WriteAttr(lname,typefound,0,datafound,sizefound);
					if (sizefound<0) {
						free(lname);
						return sizefound;
					}
					free(lname);
				} else if (countfound<1000000000) {
					char* lname=(char*)malloc(strlen(basename)+strlen(namefound)+10);
					if (lname==NULL) {
						return B_NO_MEMORY;
					}
					sprintf(lname,"%" B_PRId32,countfound-1);
					char format[12];
					sprintf(format,"%%s%%s::%%0%ldld",strlen(lname));
					for (int32 j=0;j<countfound;j++) {
						sprintf(lname,format,basename,namefound,j);
						const void* datafound;
						ssize_t sizefound;
						ret=m->FindData(namefound,typefound,j,&datafound,&sizefound);
						if (ret!=B_OK) {
							free(lname);
							return ret;
						}
						sizefound=f->WriteAttr(lname,typefound,0,datafound,sizefound);
						if (sizefound<0) {
							free(lname);
							return sizefound;
						}
					}
					free(lname);
				}
				break;
			}
		}
	}
	return B_OK;
}

