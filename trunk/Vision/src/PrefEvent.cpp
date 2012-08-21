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
 * Contributor(s): Rene Gollent
 */
 
#include "VTextControl.h"
#include "PrefEvent.h"
#include "Vision.h"

#include <Catalog.h>
#include <ScrollView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "EventPrefs"

static const char *EventControlLabels[] =
{
	B_TRANSLATE_MARK("Join"),
	B_TRANSLATE_MARK("Part"),
	B_TRANSLATE_MARK("Nick"),
	B_TRANSLATE_MARK("Quit"),
	B_TRANSLATE_MARK("Kick"),
	B_TRANSLATE_MARK("Topic"),
	B_TRANSLATE_MARK("Server Notice"),
	B_TRANSLATE_MARK("User Notice"),
	B_TRANSLATE_MARK("Notify On"),
	B_TRANSLATE_MARK("Notify Off"),
	0
};

EventPrefsView::EventPrefsView (BRect frame)
	: BView (frame, "Event prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS)
{
	SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	BRect bounds (Bounds());
	bounds.left += 3;
	bounds.right -= B_V_SCROLL_BAR_WIDTH + 3;
	bounds.top += 3;
	bounds.bottom -= 5;
	int32 i (0);

	float label_width (0.0);
	
	BString tempString;
	for (i = 0; EventControlLabels[i]; ++i)
	{
		tempString = B_TRANSLATE(EventControlLabels[i]);
		tempString += ": ";
		if (StringWidth (tempString) > label_width)
			label_width = StringWidth (tempString);
	}
	
	BView *bgView (new BView (bounds, "", B_FOLLOW_ALL_SIDES, B_WILL_DRAW));
	bgView->SetViewColor (ui_color (B_PANEL_BACKGROUND_COLOR));
	fEvents = new VTextControl * [MAX_EVENTS];

	for (i = 0; i < MAX_EVENTS; ++i)
	{
		tempString = B_TRANSLATE(EventControlLabels[i]);
		tempString += ": ";
		fEvents[i] = new VTextControl (
			BRect (5, be_plain_font->Size() + ((1.5 * i) * 1.5 * be_plain_font->Size()), 5 + bounds.right - be_plain_font->StringWidth("gP"),
			be_plain_font->Size() + (1.5 * (i + 1) * 1.5 * be_plain_font->Size())),
			"commands",
			tempString,
			vision_app->GetEvent (i).String(),
			NULL,
			B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP);

		fEvents[i]->SetDivider (label_width + 5);

		BMessage *msg (new BMessage (M_EVENT_MODIFIED));

		msg->AddInt32 ("which", i);
		fEvents[i]->SetModificationMessage (msg);
		bgView->AddChild (fEvents[i]);
	}
	fScroller = new BScrollView("command fScroller", bgView, B_FOLLOW_ALL_SIDES,
		0, false, true);
	BScrollBar *bar (fScroller->ScrollBar (B_VERTICAL));
	
	fMaxheight = bgView->Bounds().Height();
	fProportionheight = fEvents[MAX_EVENTS-1]->Frame().bottom + 10.0;
	bar->SetRange (0.0, (fProportionheight - fScroller->Bounds().Height()));
	bar->SetProportion (fScroller->Bounds().Height() / fProportionheight);

	AddChild (fScroller);
}

EventPrefsView::~EventPrefsView (void)
{
	delete [] fEvents;
}

void
EventPrefsView::AttachedToWindow (void)
{
	BView::AttachedToWindow ();
	for (int32 i = 0; i < MAX_EVENTS; i++)
		fEvents[i]->SetTarget (this);
		
	BScrollBar *bar (fScroller->ScrollBar (B_VERTICAL));
	if (bar)
		bar->SetSteps (3.0, 5.0);
}

void
EventPrefsView::AllAttached (void)
{
	BView::AllAttached ();
}

void
EventPrefsView::FrameResized (float width, float height)
{
	BView::FrameResized (width, height);
	BScrollBar *bar(fScroller->ScrollBar(B_VERTICAL));
	if (!bar)
		return;
	float min, max, scrollheight (fScroller->Bounds().Height());
	
	bar->GetRange (&min, &max);
	if (scrollheight < fProportionheight)
	{
		if (max != fMaxheight)
			bar->SetRange (0.0, fProportionheight - scrollheight);
		bar->SetProportion (scrollheight / fProportionheight);
	}
	else
	{
		bar->SetProportion (1.0);
		bar->SetRange (0.0, 0.0);
	}
}

void
EventPrefsView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_EVENT_MODIFIED:
		{
			int32 which;

			msg->FindInt32 ("which", &which);
			vision_app->SetEvent (
				which,
				fEvents[which]->TextView()->Text());
	 }
	 break;

	 default:	 
		 BView::MessageReceived (msg);
		 break;
	}
}
