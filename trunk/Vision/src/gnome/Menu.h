// BMenu, BMenuItem, BMenuBar, BSeparatorItem-like classes
//
// see License at the end of file

#ifndef F_MENU__
#define F_MENU__

#include "PlatformDefines.h"

#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkmenubar.h>

#include "Invoker.h"
#include "Menu.h"
#include "FunctionObjectMessage.h"
#include "ObjectList.h"
#include "ObjectGlue.h"

class FWindow;
class FMenu;
class FMenuBar;
struct _GtkAccelGroup;

// ToDo:
// split these up into individual headers

class FMenuItem : public ObjectGlue<GtkMenuItem>, public FInvoker {
public:
	FMenuItem(const char *, FunctionObject *, uchar = '\0', uint32 = 0);
	FMenuItem(const char *, FMessage *, uchar = '\0', uint32 = 0);

	virtual ~FMenuItem();

	FMenu *Parent()
		{ return fParent; }

	void SetMarked(bool);

	const char *Label() const;
	void SetLabel(const char *);

	void SetEnabled(bool);

	void AddAccelerator(GtkAccelGroup *);
	
protected:
	FMenuItem();

	virtual void Invoke(FMessage * = NULL);

private:
	static void InvokeBinder(GtkMenuItem *, FMenuItem *self);
	void SetFont(FFont *);
	
	FunctionObject *fInvokeItem;
	FMenu *fParent;
	uchar fKey;
	int32 fModifiers;
	
	friend class FMenu;

	typedef FInvoker inherited_;
};

class FSeparatorItem : public FMenuItem {
public:
	FSeparatorItem();
};


class FMenu : public ObjectGlue<GtkMenu> {
public:
	FMenu(const char *);
	virtual ~FMenu();

	void AddItem(FMenuItem *);
	void AddItem(FMenuItem *, int32);
	FMenuItem *RemoveItem(int32);
	bool RemoveItem(FMenuItem *);

	virtual void AttachedToWindow();
	
	void AddSeparatorItem();
	const char *Name() const
		{ return fTitle; }

	int32 CountItems() const;
	FMenuItem *ItemAt(int32);
	int32 IndexOf(const FMenuItem *) const;

	FMenuBar *Parent()
		{ return fParent; }

	FMenu *Supermenu() 
		{
#ifdef GTK_BRINGUP
		return NULL;
#endif
		}

	FMenuItem *FindItem(const char *);


	void SetFont(const FFont *);
	void GetFont(FFont *) const;

	void SetTargetForItems(FHandler *);

		// Non-BeOS
	void InitTargetForItems(FHandler *);
	virtual void Invoke(FMenuItem *);
		// allows FPopUpMenu to hook in

	void SetEnabled(bool);
		
	
private:
	static void RealizeHook(GtkMenu *, FMenu *);	

	const char *fTitle;
	ObjectList<FMenuItem> fItems;
	FMenuBar *fParent;
	FFont *fFont;
	
	friend class FMenuBar;
};

class FMenuBar : public ObjectGlue<GtkMenuBar> {
public:
	FMenuBar();
	virtual ~FMenuBar();
	
	void AddItem(FMenu *);
	void AddedToWindow(FWindow *);

	FRect Frame() const;

	int32 CountItems() const;
	FMenu *ItemAt(int32) const;
		// should return FMenuItem

	void InitTargetForItems(FHandler *);


	// the following if called Window() collides with some
	// X Window struct. could be fixed with namespace
	FWindow *__Window()
		{ return fParent; }
	
	FMenu *SubmenuAt(int32) const;

	GtkWidget *AsGtkWidget()
		{ return GTK_WIDGET(fObject); }
		
	const GtkWidget *AsGtkWidget() const
		{ return reinterpret_cast<const GtkWidget *>(fObject); }

protected:
	virtual void SizeRequest(GtkRequisition *);

private:
	void InitializeClass();
	static void SizeRequestBinder(GtkWidget *, GtkRequisition *);
	static FMenuBar *GnoBeObject(GtkWidget *gtkObject)
		{ return ::GnoBeObject<FMenuBar, GtkWidget>(gtkObject); }

private:
	ObjectList<FMenu> fItems;
	FWindow *fParent;
	GtkAccelGroup *fAccelerators;

	static bool classInitialized;

friend class FMenu;
friend class FWindow;
	// temporary, just so we can get at the gtkobject
};


#endif
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
