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
 */

#ifndef _CLICKVIEW_H
#define _CLICKVIEW_H

#include <Bitmap.h>
#include <Cursor.h>
#include <String.h>
#include <TranslationUtils.h>
#include <View.h>

class ClickView : public BView {
public:
	BString fLaunchUrl;
	BBitmap* fLogo;

	ClickView(const char* name, uint32 flags, const char* url)
		: BView(name, flags)
	{
		fLaunchUrl = url;
		fLogo = BTranslationUtils::GetBitmap('bits', "vision-logo");
	};

	virtual void AttachedToWindow()
	{
		SetViewCursor(new BCursor(B_CURSOR_ID_FOLLOW_LINK));
		if (fLogo != NULL) {
			SetExplicitMinSize(fLogo->Bounds().Size());
			SetExplicitMaxSize(fLogo->Bounds().Size());
		}
	}

	virtual void Draw(BRect);
	virtual void MouseDown(BPoint);
};

#endif
