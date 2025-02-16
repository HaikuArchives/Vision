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

#include "PrefAliases.h"
#include "ColumnListView.h"
#include "ColumnTypes.h"
#include "Vision.h"

#include <Button.h>
#include <LayoutBuilder.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PrefAliases"

const uint32 M_ALIAS_SELECTION_CHANGED = 'mASC';
const uint32 M_ALIAS_ADD = 'mADD';
const uint32 M_ALIAS_REMOVE = 'MARE';

AliasesPrefsView::AliasesPrefsView()
	: BView("Alias Prefs", 0),
	  fAliasView(NULL),
	  fAddButton(NULL),
	  fRemoveButton(NULL)
{
	AdoptSystemColors();
}

AliasesPrefsView::~AliasesPrefsView() {}

void
AliasesPrefsView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case M_ALIAS_SELECTION_CHANGED:
		{
			if (msg->FindInt32("index") >= 0) {
				fRemoveButton->SetEnabled(true);
			} else {
				fRemoveButton->SetEnabled(false);
			}
		} break;

		case M_ALIAS_REMOVE:
		{
			BRow* row(fAliasView->CurrentSelection());
			if (row != NULL) {
				fAliasView->RemoveRow(row);
				vision_app->RemoveAlias(dynamic_cast<BStringField*>(row->GetField(0))->String());
				fRemoveButton->SetEnabled(false);
				delete row;
			}
		} break;

		default:
		{
			BView::MessageReceived(msg);
		} break;
	}
}

void
AliasesPrefsView::AttachedToWindow()
{
	BView::AttachedToWindow();

	fAliasView = new BColumnListView("clv", B_WILL_DRAW, B_FANCY_BORDER);

	fAliasView->SetSelectionMessage(new BMessage(M_ALIAS_SELECTION_CHANGED));
	fAliasView->AddColumn(
		new BStringColumn(B_TRANSLATE("Name"), StringWidth(B_TRANSLATE("Name")) * 2.0, 0, 300, 0),
		0);
	fAliasView->AddColumn(
		new BStringColumn(B_TRANSLATE("Alias"), StringWidth(B_TRANSLATE("Alias")) * 6.0, 0, 300, 0),
		1);

	fAddButton = new BButton("alAdd", B_TRANSLATE("Add"), new BMessage(M_ALIAS_ADD));
	fRemoveButton = new BButton("alRemove", B_TRANSLATE("Remove"), new BMessage(M_ALIAS_REMOVE));

	fRemoveButton->SetEnabled(false);

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fAliasView)
		.AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
			.AddGlue()
			.Add(fAddButton)
			.Add(fRemoveButton)
		.End()
		.SetInsets(B_USE_WINDOW_SPACING)
	.End();
	// clang-format on

	BuildAliasList();
}

void
AliasesPrefsView::AllAttached()
{
	BView::AllAttached();
	fAliasView->SetTarget(this);
	fAddButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
}

void
AliasesPrefsView::BuildAliasList()
{
	void* cookie(NULL);
	BString name, value;
	while (vision_app->GetNextAlias(&cookie, name, value)) {
		BRow* row(new BRow());
		row->SetField(new BStringField(name.String()), 0);
		row->SetField(new BStringField(value.String()), 1);
		fAliasView->AddRow(row);
	}
}
