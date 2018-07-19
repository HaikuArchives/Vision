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

#include "PrefCommand.h"
#include "Vision.h"
#include <LayoutBuilder.h>
#include <ScrollView.h>
#include <TextControl.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefCommand"

static const char* CommandControlLabels[] = {
	B_TRANSLATE("Quit:"), B_TRANSLATE("Kick:"), B_TRANSLATE("Ignore:"), B_TRANSLATE("Unignore:"),
	B_TRANSLATE("Away:"), B_TRANSLATE("Back:"), B_TRANSLATE("Uptime:"), 0};

CommandPrefsView::CommandPrefsView()
	: BView("Command prefs", 0)
{
	AdoptSystemColors();

	BView* bgView = new BView("", 0);
	bgView->AdoptSystemColors();
	fCommands = new BTextControl* [MAX_COMMANDS];

	for (int32 i = 0; i < MAX_COMMANDS; ++i) {
		fCommands[i] = new BTextControl("commands", CommandControlLabels[i], vision_app->GetCommand(i).String(), NULL);
		BMessage* msg(new BMessage(M_COMMAND_MODIFIED));
		msg->AddInt32("which", i);
		fCommands[i]->SetModificationMessage(msg);
	}

	BLayoutBuilder::Group<>(bgView, B_VERTICAL, 0)
		.AddGrid(0.0, 0.0)
			.Add(fCommands[0]->CreateLabelLayoutItem(), 0, 0)
			.Add(fCommands[0]->CreateTextViewLayoutItem(), 1, 0)
			.Add(fCommands[1]->CreateLabelLayoutItem(), 0, 1)
			.Add(fCommands[1]->CreateTextViewLayoutItem(), 1, 1)
			.Add(fCommands[2]->CreateLabelLayoutItem(), 0, 2)
			.Add(fCommands[2]->CreateTextViewLayoutItem(), 1, 2)
			.Add(fCommands[3]->CreateLabelLayoutItem(), 0, 3)
			.Add(fCommands[3]->CreateTextViewLayoutItem(), 1, 3)
			.Add(fCommands[4]->CreateLabelLayoutItem(), 0, 4)
			.Add(fCommands[4]->CreateTextViewLayoutItem(), 1, 4)
			.Add(fCommands[5]->CreateLabelLayoutItem(), 0, 5)
			.Add(fCommands[5]->CreateTextViewLayoutItem(), 1, 5)
			.Add(fCommands[6]->CreateLabelLayoutItem(), 0, 6)
			.Add(fCommands[6]->CreateTextViewLayoutItem(), 1, 6)
		.End();
	/*
	fScroller = new BScrollView("command scroller", bgView, 0, false, true, B_NO_BORDER);
	BScrollBar* bar(fScroller->ScrollBar(B_VERTICAL));

	fMaxheight = bgView->Bounds().Height();
	fProportionheight = fCommands[MAX_COMMANDS - 1]->Frame().bottom + 10.0;
	bar->SetRange(0.0, (fProportionheight - fScroller->Bounds().Height()));
	bar->SetProportion(fScroller->Bounds().Height() / fProportionheight);
	*/
	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.SetInsets(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
		.Add(bgView)
//		.Add(fScroller)
		.AddGlue();
}

CommandPrefsView::~CommandPrefsView()
{
	delete[] fCommands;
}

void CommandPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	for (int32 i = 0; i < MAX_COMMANDS; i++) fCommands[i]->SetTarget(this);
}

void CommandPrefsView::AllAttached()
{
	BView::AllAttached();
}

void CommandPrefsView::FrameResized(float width, float height)
{
	BView::FrameResized(width, height);
	BScrollBar* bar(fScroller->ScrollBar(B_VERTICAL));
	if (!bar) return;
	float min, max, scrollheight(fScroller->Bounds().Height());

	bar->GetRange(&min, &max);
	if (scrollheight < fProportionheight) {
		if (max != fMaxheight) bar->SetRange(0.0, fProportionheight - scrollheight);
		bar->SetProportion(scrollheight / fProportionheight);
	} else {
		bar->SetProportion(1.0);
		bar->SetRange(0.0, 0.0);
	}
}

void CommandPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_COMMAND_MODIFIED: {
		int32 which;

		msg->FindInt32("which", &which);
		vision_app->SetCommand(which, fCommands[which]->TextView()->Text());
	} break;

	default:
		BView::MessageReceived(msg);
	}
}
