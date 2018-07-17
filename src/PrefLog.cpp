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

#include "NumericFilter.h"
#include "PrefLog.h"
#include "Vision.h"

#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Entry.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <TextControl.h>

#include <ctype.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefLog"

LogPrefsView::LogPrefsView()
	: BView("DCC prefs", B_WILL_DRAW | B_FRAME_EVENTS)
{
	AdoptSystemColors();

	fLogBaseDir = new BTextControl(NULL, B_TRANSLATE("Log base path:"),
								   vision_app->GetString("logBaseDir"),
								   new BMessage(M_PREFLOG_LOGPATH_CHANGED));

	fLogStampFormat = new BTextControl(NULL, B_TRANSLATE("Timestamp format:"),
									   vision_app->GetString("timestamp_format"),
									   new BMessage(M_PREFLOG_TS_FORMAT_CHANGED));

	BMessage msg(M_PREFLOG_CHECKBOX_CHANGED);

	fClearLogs = new BButton("Clear Logs", B_TRANSLATE("Delete Log Files"),
							 new BMessage(M_PREFLOG_DELETE_LOGS));
	if (_CheckIfEmpty()) fClearLogs->SetEnabled(false);

	msg.AddString("setting", "timestamp");
	fTimeStamp =
		new BCheckBox("timestamp", B_TRANSLATE("Show timestamps in IRC window"), new BMessage(msg));
	fTimeStamp->SetValue((vision_app->GetBool("timestamp")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "log_enabled");
	fLogEnabled =
		new BCheckBox("fLogEnabled", B_TRANSLATE("Enable logging"), new BMessage(msg));
	fLogEnabled->SetValue((vision_app->GetBool("log_enabled")) ? B_CONTROL_ON : B_CONTROL_OFF);

	msg.ReplaceString("setting", "log_filetimestamp");
	fLogFileTimestamp = new BCheckBox("fLogFileTimestamp", B_TRANSLATE("Append timestamp to log filenames"),
									  new BMessage(msg));
	fLogFileTimestamp->SetValue(vision_app->GetBool(("log_filetimestamp")) ? B_CONTROL_ON :
																			 B_CONTROL_OFF);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_WINDOW_SPACING)
			.Add(fLogBaseDir)
			.AddGroup(B_HORIZONTAL) // TODO improve this it doesn't look good
				.Add(fLogStampFormat)
//				.AddGlue()
				.Add(fClearLogs)
			.End()
			.Add(fTimeStamp)
			.Add(fLogEnabled)
			.Add(fLogFileTimestamp)
		.End();
}

LogPrefsView::~LogPrefsView()
{
}

void LogPrefsView::AttachedToWindow()
{
	fLogBaseDir->SetTarget(this);
	fLogStampFormat->SetTarget(this);
	fClearLogs->SetTarget(this);
	fTimeStamp->SetTarget(this);
	fLogEnabled->SetTarget(this);
	fLogFileTimestamp->SetTarget(this);
//	ResizeTo(fLogBaseDir->Frame().Width() + 15.0, fLogFileTimestamp->Frame().bottom + 15.0);
}

void LogPrefsView::AllAttached()
{
	BView::AllAttached();
}

void LogPrefsView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
}

void LogPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_PREFLOG_CHECKBOX_CHANGED: {
		BControl* source(NULL);
		msg->FindPointer("source", reinterpret_cast<void**>(&source));
		vision_app->SetBool(msg->FindString("setting"), source->Value() == B_CONTROL_ON);
	} break;

	case M_PREFLOG_LOGPATH_CHANGED: {
		BString tempPath(fLogBaseDir->Text());
		if (tempPath.Length() == 0) {
			BAlert* emptyPathAlert(
				new BAlert(B_TRANSLATE("Error"), B_TRANSLATE("The log path you have entered is invalid."), B_TRANSLATE("OK")));
			emptyPathAlert->Go();
			break;
		} else {
			// remove trailing slash to avoid setting off BPath's normalization
			if (tempPath[tempPath.Length() - 1] == '/') tempPath.ReplaceLast('/', '\0');

			BPath path(tempPath.String());
			if (path.InitCheck() != B_OK) {
				BAlert* badPathAlert(new BAlert(B_TRANSLATE("Error"), B_TRANSLATE("The log path you have entered is invalid."),
												B_TRANSLATE("OK")));
				badPathAlert->Go();
				break;
			}
		}
		vision_app->SetString("logBaseDir", 0, tempPath.String());
	} break;

	case M_PREFLOG_TS_FORMAT_CHANGED: {
		vision_app->SetString("timestamp_format", 0, fLogStampFormat->Text());
	} break;

	case M_PREFLOG_DELETE_LOGS: {
		BString path(vision_app->GetString("logBaseDir"));
		BEntry entry(path.String());
		_DeleteLogs(&entry);
		fClearLogs->SetEnabled(false);
	} break;

	default:
		BView::MessageReceived(msg);
		break;
	}
}

void LogPrefsView::_DeleteLogs(BEntry* dir_entry)
{
	BEntry entry;
	BDirectory dir(dir_entry);
	dir.Rewind();
	while (dir.GetNextEntry(&entry) == B_OK) {
		if (entry.IsDirectory())
			_DeleteLogs(&entry);
		else
			entry.Remove();
	}
}

bool LogPrefsView::_CheckIfEmpty()
{
	BString path(vision_app->GetString("logBaseDir"));
	BDirectory dir(path.String());
	BEntry entry;
	while (dir.GetNextEntry(&entry) == B_OK) {
		if (entry.IsDirectory()) {
			BDirectory directory(&entry);
			if (directory.CountEntries() > 0) return false;
		} else
			return false;
	}
	return true;
}
