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
				   Alan Ellis
 */

#include "PrefGeneral.h"
#include "PrefAliases.h"
#include "PrefApp.h"
#include "PrefColor.h"
#include "PrefDCC.h"
#include "PrefFont.h"
#include "PrefCommand.h"
#include "PrefEvent.h"
#include "PrefLog.h"

#include <stdio.h>

#include <Box.h>
#include <CardLayout.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefGeneral"

GeneralPrefsView::GeneralPrefsView(const char* title)
	: BView(title, 0), fPreviousSelection(0)
{
	AdoptSystemColors();

	fPrefsItems[piWindow] = new AppWindowPrefsView();
	fPrefsItems[piColor] = new ColorPrefsView();
	fPrefsItems[piAlias] = new AliasesPrefsView();
	fPrefsItems[piFonts] = new FontPrefsView();
	fPrefsItems[piCommands] = new CommandPrefsView();
	fPrefsItems[piEvents] = new EventPrefsView();
	fPrefsItems[piDCC] = new DCCPrefsView();
	fPrefsItems[piLog] = new LogPrefsView();

	fPrefsBox = new BBox("prefsBox");
	fPrefsContainerBox = new BView("prefsContainerBox", 0);
	fPrefsContainerBox->SetLayout(new BCardLayout());
	fPrefsBox->AddChild(fPrefsContainerBox);


	for (int32 i = 0; i < piEND; i++)
		((BCardLayout*) fPrefsContainerBox->GetLayout())->AddView(fPrefsItems[i]);

	fPrefsListView = new BListView("PrefsList", B_SINGLE_SELECTION_LIST);
	fPrefsListView->SetSelectionMessage(new BMessage(M_GENERALPREFS_SELECTION_CHANGED));

	BScrollView* scrollView = new BScrollView("list scrollView", fPrefsListView,
		B_FRAME_EVENTS | B_WILL_DRAW, false, true);

	BLayoutBuilder::Group<>(this)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.Add(scrollView)
			.Add(fPrefsBox)
			.End()
		.SetInsets(B_USE_WINDOW_SPACING)
	.End();
	fPrefsBox->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));
}

GeneralPrefsView::~GeneralPrefsView()
{
	while (fPrefsListView->CountItems() != 0) delete fPrefsListView->RemoveItem((int32)0);
}

void GeneralPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Aliases")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Application")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Colors")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Fonts")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Commands")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Events")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("DCC")));
	AddOptionsToListView(fPrefsListView, new BStringItem(B_TRANSLATE("Logging")));


}

void GeneralPrefsView::AllAttached()
{
	BView::AllAttached();
	fPrefsListView->SetTarget(this);
	fPrefsListView->Select(0);
	fPrefsListView->MakeFocus();
	//fPrefsListView->Select (0L, false);
}

void GeneralPrefsView::Show()
{
	BView::Show();
}

void GeneralPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
	case M_GENERALPREFS_SELECTION_CHANGED: {
		//int32 selectedItem(msg->FindInt32("index"));
		int32 selectedItem = fPrefsListView->CurrentSelection();

		if (selectedItem >= 0L && selectedItem < piEND)
		{
			BStringItem* item((BStringItem*)fPrefsListView->ItemAt(selectedItem));
			fPrefsBox->SetLabel(item->Text());
			((BCardLayout*) fPrefsContainerBox->GetLayout())->SetVisibleItem(selectedItem);
			fPreviousSelection = selectedItem;
		}
		else
		{
			/* Find previously selected item (stored in currentView) and make that the selected view
				This happens when user tries to deselect an item in the list */
			fPrefsListView->Select (fPreviousSelection);
		}
	} break;

	default:
		BView::MessageReceived(msg);
	}
}


void GeneralPrefsView::AddOptionsToListView(BListView* listView, BStringItem* item)
{
	listView->AddItem(item);
	// constraint the listview width so that the longest item fits
	float width = 0;
	listView->GetPreferredSize(&width, NULL);
	width += B_V_SCROLL_BAR_WIDTH;
	listView->SetExplicitMinSize(BSize(width, 0));
	listView->SetExplicitMaxSize(BSize(width, B_SIZE_UNLIMITED));

}
