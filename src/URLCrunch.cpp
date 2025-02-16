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
 * Contributor(s): Rene Gollent
 *                 Todd Lair
 */

#include <Autolock.h>
#include <Mime.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "URLCrunch.h"

BLocker URLCrunch::fLocker("URLCrunch taglist");
const char** URLCrunch::fTags = NULL;

URLCrunch::URLCrunch(const char* data, int32 len)
	: buffer(""),
	  current_pos(0)
{
	buffer.Append(data, len);

	const char** tags = fTags;

	// This is used to skip the tag that are not present in the buffer
	for (fCountTags = 0; tags[fCountTags]; ++fCountTags)
		;
	missingTags = new bool[fCountTags];

	for (int32 i = 0; i < fCountTags; i++)
		missingTags[i] = false;
}

URLCrunch::~URLCrunch()
{
	delete[] missingTags;
}

int32
URLCrunch::Crunch(BString* url)
{
	if (current_pos >= buffer.Length())
		return B_ERROR;

	int32 bufferLength(buffer.Length());
	int32 pos(current_pos);
	int32 url_length(0);
	int32 i(0);

	BAutolock _(fLocker);
	const char** tags = fTags;
	if (!tags)
		return B_ERROR;

	int32 found_pos = B_ERROR;

	while (true) {
		int32 marker_pos = bufferLength;

		for (int j = 0; tags[j]; ++j) {
			if (missingTags[j])
				continue;
			found_pos = buffer.IFindFirst(tags[j], pos);
			if (found_pos != B_ERROR) {
				if (found_pos < marker_pos) {
					marker_pos = found_pos;
					i = j;
				}
			} else
				missingTags[j] = true;
		}

		if (marker_pos < bufferLength) {
			url_length = marker_pos + strlen(tags[i]);
			url_length += strcspn(buffer.String() + url_length, " \t\n|\\<>\")(][}{;'*^");
			int len(strlen(tags[i]));

			if (url_length - marker_pos > len
				&& (isdigit(buffer[marker_pos + len]) || isalpha(buffer[marker_pos + len])
					|| buffer[marker_pos + len] == '/')) {
				pos = url_length + 1;
				url_length -= marker_pos;
				url->Truncate(0);
				buffer.CopyInto(*url, marker_pos, url_length);
				current_pos = pos;
				return marker_pos;
			}
			pos = marker_pos + 1;

		} else {
			current_pos = bufferLength;
			return B_ERROR;
		}
	}
}

status_t
URLCrunch::UpdateTagList()
{
	int i;

	// known hostnames
	const char* builtins[] = { "www.", "ftp.",
		"#",  // MUST stay last
		NULL };

	BAutolock _(fLocker);

	// empty old list
	for (i = 0; fTags && fTags[i]; i++) {
		free(const_cast<char*>(fTags[i]));
	}
	delete[] fTags;
	fTags = NULL;

	// build the list
	BList tags;
	BMessage types;
	BMimeType::GetInstalledTypes("application", &types);
	BString type;

	for (i = 0; types.FindString("types", i, &type) >= B_OK; i++) {
		if (type.FindFirst(B_URL_MIME_PREFIX) != 0)
			continue;
		type.RemoveFirst(B_URL_MIME_PREFIX);
		type << ":";
		// require slash-slash for those to limit wrong matches
		if (type == "https:" || type == "http:" || type == "ftp:")
			type << "//";
		if (type == "file:")
			type << "/";
		tags.AddItem(strdup(type.String()));
	}

	// add builtins at end (so http://www.foobar isn't picked up as www.foobar)
	for (i = 0; builtins[i]; i++)
		tags.AddItem(strdup(builtins[i]));

	// allocate new list
	fTags = new const char*[tags.CountItems() + 1];
	for (i = 0; i < tags.CountItems(); i++) {
		fTags[i] = (const char*)tags.ItemAt(i);
	}
	fTags[tags.CountItems()] = NULL;

	return B_OK;
}
