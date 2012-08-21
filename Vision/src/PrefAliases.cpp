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

#include "ColumnTypes.h"
#include "ColumnListView.h"
#include "PrefAliases.h"
#include "Vision.h"

#include <Button.h>
#include <Catalog.h>

const uint32 M_ALIAS_SELECTION_CHANGED = 'mASC';
const uint32 M_ALIAS_ADD = 'mADD';
const uint32 M_ALIAS_REMOVE = 'MARE';

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AliasPrefs"
 
AliasesPrefsView::AliasesPrefsView (BRect frame)
 : BView(frame, "Alias Prefs", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	 fAliasView(NULL),
	 fAddButton(NULL),
	 fRemoveButton(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

AliasesPrefsView::~AliasesPrefsView (void)
{
}

void
AliasesPrefsView::MessageReceived (BMessage *msg)
{
	switch (msg->what)
	{
		case M_ALIAS_SELECTION_CHANGED:
		{
			if (msg->FindInt32("index") >= 0)
			{
				fRemoveButton->SetEnabled(true);
			}
			else
			{
				fRemoveButton->SetEnabled(false);
			}
		}
		break;
		
		case M_ALIAS_REMOVE:
		{
			BRow *row (fAliasView->CurrentSelection());
			if (row != NULL)
			{
				fAliasView->RemoveRow(row);
				vision_app->RemoveAlias(dynamic_cast<BStringField *>(row->GetField(0))->String());
				fRemoveButton->SetEnabled(false);
				delete row;
			} 
		}
		break;
		
		default:
		{
			BView::MessageReceived(msg);
		}		
		break;
	}
}

void
AliasesPrefsView::AttachedToWindow (void)
{
	BView::AttachedToWindow();
	
	BRect bounds (Bounds());
	bounds.InsetBySelf(10, 10);
	bounds.bottom -= 50;

	fAliasView = new BColumnListView(bounds, "clv", B_FOLLOW_ALL_SIDES, B_WILL_DRAW, B_FANCY_BORDER);
	AddChild(fAliasView);
	fAliasView->SetSelectionMessage(new BMessage(M_ALIAS_SELECTION_CHANGED));
	BString itemText = B_TRANSLATE("Name");
	fAliasView->AddColumn(new BStringColumn(itemText.String(), StringWidth(itemText.String()) * 2.0,
		0, bounds.Width(), 0), 0);
	itemText = B_TRANSLATE("Alias");
	fAliasView->AddColumn(new BStringColumn(itemText.String(), StringWidth(itemText.String()) * 6.0,
		0, bounds.Width(), 0), 1);
	itemText = B_TRANSLATE("Add" B_UTF8_ELLIPSIS);
	fAddButton = new BButton(BRect(0,0,0,0), "alAdd", itemText.String(), new BMessage(M_ALIAS_ADD), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	AddChild(fAddButton);
	fAddButton->ResizeToPreferred();
	fRemoveButton = new BButton(BRect(0,0,0,0), "alRemove", B_TRANSLATE("Remove"), new BMessage(M_ALIAS_REMOVE), B_FOLLOW_RIGHT | B_FOLLOW_BOTTOM);
	AddChild(fRemoveButton);
	fRemoveButton->ResizeToPreferred();
	fRemoveButton->MoveTo(fAliasView->Frame().right - fRemoveButton->Bounds().Width(),
		fAliasView->Frame().bottom + 10.0);
	fAddButton->MoveTo(fRemoveButton->Frame().left - (fAddButton->Frame().Width() + 5.0), fRemoveButton->Frame().top);
	
	fRemoveButton->SetEnabled(false);
	
	BuildAliasList();
}

void
AliasesPrefsView::AllAttached (void)
{
	BView::AllAttached();
	fAliasView->SetTarget(this);
	fAddButton->SetTarget(this);
	fRemoveButton->SetTarget(this);
}

void
AliasesPrefsView::BuildAliasList(void)
{
	void *cookie (NULL);
	BString name, value;
	while (vision_app->GetNextAlias(&cookie, name, value))
	{
		BRow *row (new BRow());
		row->SetField(new BStringField(name.String()), 0);
		row->SetField(new BStringField(value.String()), 1);
		fAliasView->AddRow(row);
	}
}
