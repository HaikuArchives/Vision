// libgnobe base class template
// 
// Gtk object wrapper to simplify building a parallel inheritance
// tree.
// works a bit like the corresponding parts of gtk--

#ifndef OBJECT_GLUE_H__
#define OBJECT_GLUE_H__

#include <gtk/gtkobject.h>

#undef DestroyNotify
// prevent a name clash

class ObjectGlueBase {
public:
	ObjectGlueBase(GtkObject *, bool own = true);
	virtual ~ObjectGlueBase();

	static GQuark QuarkID();

protected:
	virtual bool DestroyNotify();
		// return true if should delete self

private:
	static void DestroyNotifyBinder(void *);

protected:
	GtkObject *fObject;
	bool fOwn;
	bool fDestroying;
};

template <class RawGtkObjectType>
class ObjectGlue : public ObjectGlueBase {
public:
	ObjectGlue(RawGtkObjectType *gtkObject, bool own = true)
		:	ObjectGlueBase(reinterpret_cast< ::GtkObject *>(gtkObject), own)
		{}
		// (doing a reinterpret cast here because of some to be figured out
		// bad interaction of the GTK_OBJECT cast macro

	RawGtkObjectType *GtkObject()
		{ return reinterpret_cast<RawGtkObjectType *>(fObject); }

	const RawGtkObjectType *GtkObject() const
		{ return reinterpret_cast<const RawGtkObjectType *>(fObject); }
};

// call to retrieve the high-level GnoBe object from a Gtk object.
// wrap this in a type-safe call that provides the two matching types
template <class GnoBeType, class RawGtkObjectType>
GnoBeType *
GnoBeObject(RawGtkObjectType *rawObject)
{
	GtkObject *gtkObject = GTK_OBJECT(rawObject);
	if (!gtkObject)
		return NULL;

	ObjectGlueBase *baseObject = static_cast<ObjectGlueBase *>
		(gtk_object_get_data_by_id(gtkObject, ObjectGlueBase::QuarkID()));

	// test that the type we are getting is really right
	ASSERT((baseObject != NULL) == (dynamic_cast<GnoBeType *>(baseObject) != NULL));

	return static_cast<GnoBeType *>(baseObject);
}

// Call to retrieve the high-level GnoBe object from a Gtk object.
// Wrap this in a type-safe call that provides the two matching types.
// This call is used for things like FView::Parent() where the fObject for the
// top level window does not match the FWindow type.
template <class GnoBeType, class RawGtkObjectType>
GnoBeType *
GnoBeObjectIfTypeMatches(RawGtkObjectType *rawObject)
{
	GtkObject *gtkObject = GTK_OBJECT(rawObject);
	if (!gtkObject)
		return NULL;

	ObjectGlueBase *baseObject = static_cast<ObjectGlueBase *>
		(gtk_object_get_data_by_id(gtkObject, ObjectGlueBase::QuarkID()));

	return dynamic_cast<GnoBeType *>(baseObject);
}


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
