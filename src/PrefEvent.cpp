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
 */

#include "PrefEvent.h"
#include "Vision.h"

#include <LayoutBuilder.h>
#include <ScrollView.h>
#include <TextControl.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefEvent"

static const char* EventControlLabels[]
	= { B_TRANSLATE("Join:"), B_TRANSLATE("Part:"), B_TRANSLATE("Nick:"), B_TRANSLATE("Quit:"),
		  B_TRANSLATE("Kick:"), B_TRANSLATE("Topic:"), B_TRANSLATE("Server notice:"),
		  B_TRANSLATE("User notice:"), B_TRANSLATE("Notify on:"), B_TRANSLATE("Notify off:"), 0 };

EventPrefsView::EventPrefsView()
	: BView("Event prefs", 0)
{
	AdoptSystemColors();

	BView* bgView(new BView("", 0));
	bgView->AdoptSystemColors();
	fEvents = new BTextControl*[MAX_EVENTS];

	for (int32 i = 0; i < MAX_EVENTS; ++i) {
		fEvents[i] = new BTextControl(
			"commands", EventControlLabels[i], vision_app->GetEvent(i).String(), NULL);
		BMessage* msg(new BMessage(M_EVENT_MODIFIED));
		msg->AddInt32("which", i);
		fEvents[i]->SetModificationMessage(msg);
	}

	// clang-format off
	BLayoutBuilder::Group<>(bgView, B_VERTICAL, 0)
		.AddGrid(0.0, 0.0)
			.Add(fEvents[0]->CreateLabelLayoutItem(), 0, 0)
			.Add(fEvents[0]->CreateTextViewLayoutItem(), 1, 0)
			.Add(fEvents[1]->CreateLabelLayoutItem(), 0, 1)
			.Add(fEvents[1]->CreateTextViewLayoutItem(), 1, 1)
			.Add(fEvents[2]->CreateLabelLayoutItem(), 0, 2)
			.Add(fEvents[2]->CreateTextViewLayoutItem(), 1, 2)
			.Add(fEvents[3]->CreateLabelLayoutItem(), 0, 3)
			.Add(fEvents[3]->CreateTextViewLayoutItem(), 1, 3)
			.Add(fEvents[4]->CreateLabelLayoutItem(), 0, 4)
			.Add(fEvents[4]->CreateTextViewLayoutItem(), 1, 4)
			.Add(fEvents[5]->CreateLabelLayoutItem(), 0, 5)
			.Add(fEvents[5]->CreateTextViewLayoutItem(), 1, 5)
			.Add(fEvents[6]->CreateLabelLayoutItem(), 0, 6)
			.Add(fEvents[6]->CreateTextViewLayoutItem(), 1, 6)
			.Add(fEvents[7]->CreateLabelLayoutItem(), 0, 7)
			.Add(fEvents[7]->CreateTextViewLayoutItem(), 1, 7)
			.Add(fEvents[8]->CreateLabelLayoutItem(), 0, 8)
			.Add(fEvents[8]->CreateTextViewLayoutItem(), 1, 8)
			.Add(fEvents[9]->CreateLabelLayoutItem(), 0, 9)
			.Add(fEvents[9]->CreateTextViewLayoutItem(), 1, 9);
	/*
		fScroller = new BScrollView("command fScroller", bgView,
						0, false, true, B_NO_BORDER);
		BScrollBar* bar(fScroller->ScrollBar(B_VERTICAL));

		fMaxheight = bgView->Bounds().Height();
		fProportionheight = fEvents[MAX_EVENTS - 1]->Frame().bottom + 10.0;
		bar->SetRange(0.0, (fProportionheight - fScroller->Bounds().Height()));
		bar->SetProportion(fScroller->Bounds().Height() / fProportionheight);
	*/
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING,
			B_USE_DEFAULT_SPACING)
		//		.Add(fScroller);
		.Add(bgView);
	// clang-format on
}

EventPrefsView::~EventPrefsView()
{
	delete[] fEvents;
}

void
EventPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	for (int32 i = 0; i < MAX_EVENTS; i++)
		fEvents[i]->SetTarget(this);
	/*
		BScrollBar* bar(fScroller->ScrollBar(B_VERTICAL));
		if (bar) bar->SetSteps(3.0, 5.0); */
}

void
EventPrefsView::AllAttached()
{
	BView::AllAttached();
}

void
EventPrefsView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
	/*	BScrollBar* bar(fScroller->ScrollBar(B_VERTICAL));
		if (!bar) return;
		float min, max, scrollheight(fScroller->Bounds().Height());

		bar->GetRange(&min, &max);
		if (scrollheight < fProportionheight) {
			if (max != fMaxheight) bar->SetRange(0.0, fProportionheight - scrollheight);
			bar->SetProportion(scrollheight / fProportionheight);
		} else {
			bar->SetProportion(1.0);
			bar->SetRange(0.0, 0.0);
		} */
}

void
EventPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_EVENT_MODIFIED:
		{
			int32 which;

			msg->FindInt32("which", &which);
			vision_app->SetEvent(which, fEvents[which]->TextView()->Text());
		} break;

		default:
			BView::MessageReceived(msg);
			break;
	}
}
