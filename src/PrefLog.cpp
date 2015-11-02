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
#include "VTextControl.h"

#include <Alert.h>
#include <Button.h>
#include <CheckBox.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Entry.h>
#include <Path.h>

#include <ctype.h>
#include <stdlib.h>

LogPrefsView::LogPrefsView(BRect frame)
	: BView(frame, "DCC prefs", B_FOLLOW_NONE, B_WILL_DRAW | B_FRAME_EVENTS)
{
	AdoptSystemColors();

	BRect checkboxRect(Bounds());
	BRect trackingBoundsRect(0.0, 0.0, 0, 0);
	float maxWidth(0), maxHeight(0);

	fLogBaseDir = new VTextControl(BRect(0, 0, 0, 0), NULL, S_PREFLOG_LOGPATH,
								   vision_app->GetString("logBaseDir"),
								   new BMessage(M_PREFLOG_LOGPATH_CHANGED));
	fLogBaseDir->ResizeToPreferred();
	fLogBaseDir->ResizeToPreferred();
	fLogBaseDir->SetDivider(fLogBaseDir->StringWidth(S_PREFLOG_LOGPATH) + 5);
	fLogBaseDir->ResizeTo(Bounds().Width() - 15, fLogBaseDir->Bounds().Height());
	fLogBaseDir->MoveTo(be_plain_font->StringWidth("S"), be_plain_font->Size());
	AddChild(fLogBaseDir);
	checkboxRect = fLogBaseDir->Bounds();

	fLogStampFormat = new VTextControl(BRect(0, 0, 0, 0), NULL, S_PREFLOG_TS_FORMAT,
									   vision_app->GetString("timestamp_format"),
									   new BMessage(M_PREFLOG_TS_FORMAT_CHANGED));
	fLogBaseDir->ResizeToPreferred();
	fLogStampFormat->SetDivider(fLogBaseDir->StringWidth(S_PREFLOG_TS_FORMAT) + 5);
	fLogStampFormat->MoveTo(be_plain_font->StringWidth("S"),
							fLogBaseDir->Frame().bottom + be_plain_font->Size());
	AddChild(fLogStampFormat);
	checkboxRect.top += fLogStampFormat->Bounds().Height() * 1.2;
	BMessage msg(M_PREFLOG_CHECKBOX_CHANGED);

	fClearLogs = new BButton(BRect(0, 0, 0, 0), "Clear Logs", S_PREFLOG_DELETE_LOGS,
							 new BMessage(M_PREFLOG_DELETE_LOGS));
	fClearLogs->ResizeToPreferred();
	fClearLogs->MoveTo(fLogBaseDir->Bounds().Width() + 200,
					   fLogBaseDir->Frame().bottom + be_plain_font->Size());
	if (_CheckIfEmpty()) fClearLogs->SetEnabled(false);
	AddChild(fClearLogs);

	checkboxRect.top += be_plain_font->Size();
	checkboxRect.bottom = checkboxRect.top;
	msg.AddString("setting", "timestamp");
	fTimeStamp =
		new BCheckBox(checkboxRect, "timestamp", S_PREFLOG_SHOW_TIMESTAMP, new BMessage(msg));
	fTimeStamp->SetValue((vision_app->GetBool("timestamp")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fTimeStamp->MoveBy(be_plain_font->StringWidth("S"), 0);
	fTimeStamp->ResizeToPreferred();
	trackingBoundsRect = fTimeStamp->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5;
	AddChild(fTimeStamp);

	checkboxRect.top += fTimeStamp->Bounds().Height() * 1.2;
	msg.ReplaceString("setting", "log_enabled");
	fLogEnabled =
		new BCheckBox(checkboxRect, "fLogEnabled", S_PREFLOG_USE_LOGGING, new BMessage(msg));
	fLogEnabled->SetValue((vision_app->GetBool("log_enabled")) ? B_CONTROL_ON : B_CONTROL_OFF);
	fLogEnabled->MoveBy(be_plain_font->StringWidth("S"), 0);
	fLogEnabled->ResizeToPreferred();
	trackingBoundsRect = fLogEnabled->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxHeight += trackingBoundsRect.Height() * 1.5;
	AddChild(fLogEnabled);

	checkboxRect.top += fLogEnabled->Bounds().Height() * 1.2;
	msg.ReplaceString("setting", "log_filetimestamp");
	fLogFileTimestamp = new BCheckBox(checkboxRect, "fLogFileTimestamp", S_PREFLOG_LOG_TIMESTAMP,
									  new BMessage(msg));
	fLogFileTimestamp->SetValue(vision_app->GetBool(("log_filetimestamp")) ? B_CONTROL_ON :
																			 B_CONTROL_OFF);
	fLogFileTimestamp->MoveBy(be_plain_font->StringWidth("S"), 0);
	fLogFileTimestamp->ResizeToPreferred();
	trackingBoundsRect = fLogFileTimestamp->Bounds();
	maxWidth = (maxWidth < trackingBoundsRect.Width()) ? trackingBoundsRect.Width() : maxWidth;
	maxWidth *= 1.2;
	maxHeight += trackingBoundsRect.Height() * 1.5;
	AddChild(fLogFileTimestamp);
}

LogPrefsView::~LogPrefsView(void)
{
}

void LogPrefsView::AttachedToWindow(void)
{
	fLogBaseDir->SetTarget(this);
	fLogBaseDir->ResizeTo(Bounds().Width() - 15, fLogBaseDir->Bounds().Height());
	fLogStampFormat->SetTarget(this);
	fLogStampFormat->ResizeTo(Bounds().Width() / 2.0, fLogStampFormat->Bounds().Height());
	fClearLogs->SetTarget(this);
	fTimeStamp->SetTarget(this);
	fTimeStamp->MoveTo(be_plain_font->StringWidth("S"),
					   fLogStampFormat->Frame().bottom + fLogStampFormat->Bounds().Height());
	fLogEnabled->SetTarget(this);
	fLogEnabled->MoveBy(0, fTimeStamp->Frame().bottom - fLogEnabled->Frame().bottom +
							   fLogEnabled->Bounds().Height() * 1.2);
	fLogFileTimestamp->SetTarget(this);
	fLogFileTimestamp->MoveBy(0, fLogEnabled->Frame().bottom - fLogFileTimestamp->Frame().bottom +
									 fLogFileTimestamp->Bounds().Height() * 1.2);
	ResizeTo(fLogBaseDir->Frame().Width() + 15.0, fLogFileTimestamp->Frame().bottom + 15.0);
}

void LogPrefsView::AllAttached(void)
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
				new BAlert(S_PREFLOG_ALERT_TITLE, S_PREFLOG_ALERT_TEXT, S_PREFLOG_ALERT_BUTTON));
			emptyPathAlert->Go();
			break;
		} else {
			// remove trailing slash to avoid setting off BPath's normalization
			if (tempPath[tempPath.Length() - 1] == '/') tempPath.ReplaceLast('/', '\0');

			BPath path(tempPath.String());
			if (path.InitCheck() != B_OK) {
				BAlert* badPathAlert(new BAlert(S_PREFLOG_ALERT_TITLE, S_PREFLOG_ALERT_TEXT,
												S_PREFLOG_ALERT_BUTTON));
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
