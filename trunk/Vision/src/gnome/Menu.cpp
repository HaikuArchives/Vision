// BMenu, BMenuItem, BMenuBar, BSeparatorItem-like classes
//
// see License at the end of file

#include "PlatformDefines.h"

#include <gtk/gtklabel.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkaccelgroup.h>
#include <stdio.h>

#include "Font.h"
#include "Window.h"
#include "Menu.h"


FMenuItem::FMenuItem(const char *title, FunctionObject *functor, uchar key, uint32 modifiers)
	:	ObjectGlue<GtkMenuItem>(GTK_MENU_ITEM(gtk_menu_item_new_with_label(title))),
		fInvokeItem(functor),
		fParent(NULL),
		fKey(key),
		fModifiers(modifiers)
{
	gtk_signal_connect(fObject, "activate", GTK_SIGNAL_FUNC(&FMenuItem::InvokeBinder), this);

}

FMenuItem::FMenuItem(const char *title, FMessage *message, uchar key, uint32 modifiers)
	:	ObjectGlue<GtkMenuItem>(GTK_MENU_ITEM(gtk_menu_item_new_with_label(title))),
		FInvoker(message, NULL),
		fInvokeItem(NULL),
		fParent(NULL),
		fKey(key),
		fModifiers(modifiers)
{
	gtk_signal_connect(fObject, "activate", GTK_SIGNAL_FUNC(&FMenuItem::InvokeBinder), this);
}

FMenuItem::FMenuItem()
	:	ObjectGlue<GtkMenuItem>(GTK_MENU_ITEM(gtk_menu_item_new())),
		fInvokeItem(NULL)
{
	// special constructor for separator items
}

FMenuItem::~FMenuItem()
{
	delete fInvokeItem;
}

void 
FMenuItem::Invoke(FMessage *message)
{
	if (fInvokeItem)
		fInvokeItem->operator()();
	else {
		FMessage copy;
		
		if (!message)
			message = Message();
		
		if (message)
			copy = *message;

		copy.AddInt32("index", fParent->IndexOf(this));
		copy.AddPointer("source", this);

		inherited_::Invoke(&copy);
	}
}

void
FMenuItem::InvokeBinder(GtkMenuItem *, FMenuItem *self)
{
	ASSERT(self->fParent);
	self->fParent->Invoke(self);
}

const char *
FMenuItem::Label() const
{
	if (!GTK_BIN (GtkObject())->child)
		return "";

	return GTK_LABEL(GTK_BIN (GtkObject())->child)->label;
}

void 
FMenuItem::SetLabel(const char *label)
{
	gtk_label_set_text (GTK_LABEL(GTK_BIN (GtkObject())->child), label);
}

void 
FMenuItem::SetFont(FFont *font)
{
	if (!GTK_BIN (GtkObject())->child)
		return;

	FFont::Set(font, GTK_BIN (GtkObject())->child);
}

void 
FMenuItem::AddAccelerator(GtkAccelGroup *accelerators)
{
	if (fKey != '\0')
		// have to use some signal here because passing an empty one doesn't work
		gtk_widget_add_accelerator(GTK_WIDGET(GtkObject()), "activate", accelerators,
			fKey, BeOSToGnomeModifiers(fModifiers), GTK_ACCEL_VISIBLE);
}

void 
FMenuItem::SetMarked(bool)
{
}

void 
FMenuItem::SetEnabled(bool on)
{
	gtk_widget_set_sensitive (GTK_WIDGET(GtkObject()), on);
}

FSeparatorItem::FSeparatorItem()
{
}

FMenu::FMenu(const char *title)
	:	ObjectGlue<GtkMenu>(GTK_MENU(gtk_menu_new())), 
		fTitle(title),
		fItems(10),
		fParent(NULL),
		fFont(NULL)
{
	gtk_signal_connect (fObject, "realize", GTK_SIGNAL_FUNC(&FMenu::RealizeHook),
		this);
}


FMenu::~FMenu()
{
	delete fFont;
}


int32 
FMenu::CountItems() const
{
	return fItems.CountItems();
}

FMenuItem *
FMenu::ItemAt(int32 index)
{
	return fItems.ItemAt(index);
}

void
FMenu::AddItem(FMenuItem *item)
{
	fItems.AddItem(item);
	if (!item->Target() && fParent && fParent->__Window())
		item->SetTarget(fParent->__Window());

	item->fParent = this;
	gtk_menu_append(GtkObject(), GTK_WIDGET(item->GtkObject()));
	if (fFont)
		item->SetFont(fFont);
	gtk_widget_show(GTK_WIDGET(item->GtkObject()));
}

void 
FMenu::AddItem(FMenuItem *item, int32 index)
{
	fItems.AddItem(item, index);
	if (!item->Target() && fParent && fParent->__Window())
		item->SetTarget(fParent->__Window());

	item->fParent = this;
	if (fFont)
		item->SetFont(fFont);
	gtk_menu_insert(GtkObject(), GTK_WIDGET(item->GtkObject()), index);
	gtk_widget_show(GTK_WIDGET(item->GtkObject()));
}

FMenuItem * 
FMenu::RemoveItem(int32 index)
{
	FMenuItem *item = fItems.ItemAt(index);
	if (item) {
		gtk_widget_ref(GTK_WIDGET(item->GtkObject()));
#if 1
		if (GtkObject()->old_active_menu_item == GTK_WIDGET(item->GtkObject())) {
			//work around a gtk bug
			gtk_widget_unref(GTK_WIDGET(item->GtkObject()));
			GtkObject()->old_active_menu_item = NULL;
		}
#endif
		gtk_container_remove(GTK_CONTAINER(fObject),
			GTK_WIDGET(item->GtkObject()));
		fItems.RemoveItemAt(index);
		item->fParent = NULL;
		g_assert(GTK_WIDGET(item->GtkObject())->parent == NULL);
		gtk_widget_unref(GTK_WIDGET(item->GtkObject()));
	}
	return item;
}

bool 
FMenu::RemoveItem(FMenuItem *item)
{
	if (fItems.IndexOf(item) < 0) 
		return false;

	gtk_widget_ref(GTK_WIDGET(item->GtkObject()));
#if 1
	if (GtkObject()->old_active_menu_item == GTK_WIDGET(item->GtkObject())) {
		//work around a gtk bug
		gtk_widget_unref(GTK_WIDGET(item->GtkObject()));
		GtkObject()->old_active_menu_item = NULL;
	}
#endif
	gtk_container_remove(GTK_CONTAINER(fObject), GTK_WIDGET(item->GtkObject()));
	fItems.RemoveItem(item);
	gtk_widget_unref(GTK_WIDGET(item->GtkObject()));

	delete item;
	return true;
}

FMenuItem *
FMenu::FindItem(const char *label)
{
	int32 count = fItems.CountItems();
	for (int32 index = 0; index < count; index++) {
		if (strcmp(fItems.ItemAt(index)->Label(), label) == 0)
			return fItems.ItemAt(index);
	}
	return NULL;
}

int32
FMenu::IndexOf(const FMenuItem *item) const
{
	return fItems.IndexOf(item);
}

void 
FMenu::AddSeparatorItem()
{
	AddItem(new FSeparatorItem());
}

void 
FMenu::InitTargetForItems(FHandler *handler)
{
	int32 count = CountItems();
	GtkAccelGroup *accelerators = Parent()->fAccelerators;
	for (int32 index = 0; index < count; index++) {
		FMenuItem *item = ItemAt(index);
		if (!item)
			break;
		if (item->Target())
			continue;
		item->SetTarget(handler);
		item->AddAccelerator(accelerators);
	}
}

void 
FMenu::SetTargetForItems(FHandler *handler)
{
	int32 count = CountItems();
	for (int32 index = 0; index < count; index++) {
		FMenuItem *item = ItemAt(index);
		if (!item)
			break;
		item->SetTarget(handler);
	}
}


void 
FMenu::Invoke(FMenuItem *menuItem)
{
	menuItem->Invoke();
}

void 
FMenu::SetFont(const FFont *font)
{
	delete fFont;
	fFont = new FFont(*font);
	int32 count = CountItems();
	for (int32 index = 0; index < count; index++) {
		FMenuItem *item = ItemAt(index);
		if (!item)
			break;
		item->SetFont(fFont);
	}
}

void 
FMenu::GetFont(FFont *result) const
{
	if (!fFont)
		*result = *be_plain_font;
	else
		*result = *fFont;
}

void 
FMenu::RealizeHook(GtkMenu *, FMenu *menu)
{
	menu->AttachedToWindow();
}

void 
FMenu::AttachedToWindow()
{
	// empty
}

void 
FMenu::SetEnabled(bool on)
{
	gtk_widget_set_sensitive (GTK_WIDGET(GtkObject()), on);
}

FMenuBar::FMenuBar()
	:	ObjectGlue<GtkMenuBar>(GTK_MENU_BAR(gtk_menu_bar_new())),
		fParent(NULL),
		fAccelerators(gtk_accel_group_new())
{
	InitializeClass();

	// hack to set the menubar shorter than default
	gtk_widget_set_usize(GTK_WIDGET(GtkObject()), 1000, 22);
	AsGtkWidget()->allocation.width = 1000;
	AsGtkWidget()->allocation.height = 22;	
}


FMenuBar::~FMenuBar()
{
}


FRect 
FMenuBar::Frame() const
{
	return FRect(AsGtkWidget()->allocation.x, 
		AsGtkWidget()->allocation.y,
		AsGtkWidget()->allocation.x + AsGtkWidget()->allocation.width,
		AsGtkWidget()->allocation.y + AsGtkWidget()->allocation.height);
}

void 
FMenuBar::InitTargetForItems(FHandler *handler)
{
	int32 count = CountItems();
	for (int32 index = 0; index < count; index++) {
		FMenu *menu = ItemAt(index);
		if (menu)
			menu->InitTargetForItems(handler);
	}
}

void
FMenuBar::AddedToWindow(FWindow *window)
{
	fParent = window;
	InitTargetForItems(window);
	gtk_accel_group_attach(fAccelerators, (struct _GtkObject *)(window->GtkObject()));
}

void
FMenuBar::AddItem(FMenu *menu)
{
	GtkMenuItem *menuItem = GTK_MENU_ITEM(gtk_menu_item_new_with_label(menu->Name()));
	gtk_menu_item_set_submenu(menuItem, GTK_WIDGET(menu->GtkObject()));
	gtk_menu_bar_append(GtkObject(), GTK_WIDGET(menuItem));
	gtk_widget_show(GTK_WIDGET(menuItem));

	fItems.AddItem(menu);
	menu->fParent = this;
	InitTargetForItems(fParent);
}

int32 
FMenuBar::CountItems() const
{
	return fItems.CountItems();
}

FMenu *
FMenuBar::ItemAt(int32 index) const
{
	if (index >= CountItems())
		return NULL;
	
	return fItems.ItemAt(index);
}

FMenu *
FMenuBar::SubmenuAt(int32 index) const
{
	return ItemAt(index);
}

#define BORDER_SPACING  0
#define CHILD_SPACING   3

void 
FMenuBar::SizeRequest(GtkRequisition *requisition)
{
	requisition->width = (int)Frame().Width();
	requisition->height = (int)Frame().Height();

	if (fObject->flags & GTK_VISIBLE) {	
		for (GList *children = GTK_MENU_SHELL (AsGtkWidget())->children;
			children != NULL;
			children = children->next) {
			
			GtkWidget *child = (GtkWidget *)children->data;
			
			if (((struct _GtkObject *)child)->flags & GTK_VISIBLE) {
				GtkRequisition child_requisition;
				GTK_MENU_ITEM(child)->show_submenu_indicator = FALSE;
				gtk_widget_size_request(child, &child_requisition);
			}
		}
		
	}
}

void 
FMenuBar::InitializeClass()
{
	if (classInitialized)
		return;

	GTK_WIDGET_CLASS(reinterpret_cast< ::GtkObject *>(GtkObject())->klass)
		->size_request = &FMenuBar::SizeRequestBinder;

	classInitialized = true;
}

void 
FMenuBar::SizeRequestBinder(GtkWidget *widget, GtkRequisition *requisition)
{
	ASSERT(GnoBeObject(widget));
	return GnoBeObject(widget)->SizeRequest(requisition);
}

bool FMenuBar::classInitialized = false;

/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met: 

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer. 

Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution. 

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF TITLE,
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF, OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE. 
*/
