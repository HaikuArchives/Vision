// libgnobe base class template
// 
// Gtk object wrapper to simplify building a parallel inheritance
// tree.
// works a bit like the corresponding parts of gtk--

#include <Debug.h>
#include "ObjectGlue.h"

ObjectGlueBase::ObjectGlueBase(GtkObject *object, bool own)
	:	fObject(object),
		fOwn(own),
		fDestroying(false)
{
	ASSERT(GTK_IS_OBJECT(object));
	ASSERT(!gtk_object_get_data_by_id(fObject, QuarkID()));
	gtk_object_set_data_by_id_full(fObject, QuarkID(), this, 
		&ObjectGlueBase::DestroyNotifyBinder);
}

ObjectGlueBase::~ObjectGlueBase()
{
	fDestroying = true;
	if (fObject) {
		gtk_object_remove_no_notify_by_id (fObject, QuarkID());

		if (fOwn) {
			if (!GTK_OBJECT_DESTROYED(fObject)) {
				// ref once so that the object doesn't get
				// deleted before the destroy call returns
				gtk_object_ref(fObject);
				gtk_object_destroy(fObject);
			}
			// get rid of the object
			while (fObject->ref_count >= 1)
				gtk_object_unref(fObject);
		}
	}
}

bool 
ObjectGlueBase::DestroyNotify()
{
	return true;
}

void 
ObjectGlueBase::DestroyNotifyBinder(void *castToSelf)
{
	ObjectGlueBase *self = static_cast<ObjectGlueBase *>(castToSelf);
	bool deleteSelf = self->DestroyNotify();
	self->fObject = 0;
	if (deleteSelf && !self->fDestroying)
		delete self;
}

GQuark objectGlueQuark = 0;

GQuark 
ObjectGlueBase::QuarkID()
{
	if (!objectGlueQuark) 
		objectGlueQuark = g_quark_from_static_string ("libgnobe");

	return objectGlueQuark;
}
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
