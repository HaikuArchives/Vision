// BScrollBar-like class
//
// see License at the end of file

#include "ScrollBar.h"
#include <gtk/gtkadjustment.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkhscrollbar.h>
#include <gtk/gtkrange.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtkvscrollbar.h>


static GtkWidget *
NewGtkScrollBar(orientation which, float min, float max)
{
	GtkAdjustment *adjustment = (GtkAdjustment *)gtk_adjustment_new(min, min, max, 1, 10, 10);
	GtkWidget *result;
	GtkWidget *scrollBar;
	if (which == B_VERTICAL) {
		result = gtk_vbox_new(true, 0);
		scrollBar = gtk_vscrollbar_new(adjustment);
		gtk_box_pack_start(GTK_BOX(result), scrollBar, true, true, 0);
	} else {
		gtk_hscrollbar_new(adjustment);
		result = gtk_hbox_new(true, 0);
		scrollBar = gtk_hscrollbar_new(adjustment);
		gtk_box_pack_start(GTK_BOX(result), scrollBar, true, true, 0);
	}
	// make the minimal thumb size easier to grab
	if (GTK_RANGE_CLASS(((struct _GtkObject *)scrollBar)->klass)->min_slider_size < 16)
		GTK_RANGE_CLASS(((struct _GtkObject *)scrollBar)->klass)->min_slider_size = 16;

	return result;
}

FScrollBar::FScrollBar(FRect frame, const char *name, BView *target,
	float min, float max, orientation which)
	:	FView(NewGtkScrollBar(which, min, max), frame, name, 
			which == B_VERTICAL ? B_FOLLOW_RIGHT : B_FOLLOW_BOTTOM, 0),
		fOrientation(which),
		fTarget(target),
		fScrolling(false)
{
	GtkWidget *scrollbar = ((GtkBoxChild *)(GTK_BOX(GtkObject())->children->data))->widget;
	gtk_signal_connect((struct _GtkObject *)(GTK_RANGE(scrollbar)->adjustment),
		"value_changed",
		(GtkSignalFunc)&FScrollBar::ValueChangedCallback,
		this);
}

orientation 
FScrollBar::Orientation() const
{
	return fOrientation;
}

void 
FScrollBar::ValueChangedCallback(void *, void *castToThis)
{
	FScrollBar *self = (FScrollBar *)castToThis;

	if (self->fScrolling)
		// avoid recursion
		return;
	
	self->fScrolling = true;
	if (self->Orientation() == B_VERTICAL)
		self->fTarget->VScrollToCallback(self->Value());
	else
		self->fTarget->HScrollToCallback(self->Value());
	self->fScrolling = false;
}

float 
FScrollBar::Value() const
{
	GtkWidget *scrollbar = ((GtkBoxChild *)(GTK_BOX(GtkObject())->children->data))->widget;
	float result = GTK_RANGE(scrollbar)->adjustment->value;
	
	if (GTK_RANGE(scrollbar)->adjustment->upper
		- GTK_RANGE(scrollbar)->adjustment->lower > 0) {
		result /= (GTK_RANGE(scrollbar)->adjustment->upper
			- GTK_RANGE(scrollbar)->adjustment->lower);
		result *= (GTK_RANGE(scrollbar)->adjustment->upper
			- GTK_RANGE(scrollbar)->adjustment->lower);
	}
	return result;
}

void 
FScrollBar::SetValue(float value)
{
	if (fScrolling)
		// avoid recursion
		return;

	GtkWidget *scrollbar = ((GtkBoxChild *)(GTK_BOX(GtkObject())->children->data))->widget;
	fScrolling = true;
	
	if (GTK_RANGE(scrollbar)->adjustment->upper
		- GTK_RANGE(scrollbar)->adjustment->lower > 0) {
		value /= (GTK_RANGE(scrollbar)->adjustment->upper
			- GTK_RANGE(scrollbar)->adjustment->lower);
		value *= (GTK_RANGE(scrollbar)->adjustment->upper
			- GTK_RANGE(scrollbar)->adjustment->lower);
	}

	gtk_adjustment_set_value(GTK_RANGE(scrollbar)->adjustment, value);

	fScrolling = false;
}

void 
FScrollBar::SetRange(float min, float max)
{
	GtkWidget *scrollbar = ((GtkBoxChild *)(GTK_BOX(GtkObject())->children->data))->widget;
	GtkAdjustment *adjustment = GTK_RANGE(scrollbar)->adjustment;
	adjustment->lower = min;
	adjustment->upper = max + 1.5 * adjustment->page_size;
	gtk_adjustment_value_changed (adjustment);
}

void 
FScrollBar::SetSteps(float arrow, float page)
{
	GtkWidget *scrollbar = ((GtkBoxChild *)(GTK_BOX(GtkObject())->children->data))->widget;
	GtkAdjustment *adjustment = GTK_RANGE(scrollbar)->adjustment;
	adjustment->step_increment = arrow;
	adjustment->page_increment = page;

	adjustment->upper -= 1.5 * adjustment->page_size;
	adjustment->page_size = page;
	adjustment->upper += 1.5 * page;
	gtk_adjustment_value_changed (adjustment);
}

int32 
FScrollBar::VScrollBarWidth()
{
	GtkWidget *tmpScrollBar = gtk_vscrollbar_new(NULL);
	int32 result = GTK_RANGE_CLASS(((struct _GtkObject *)tmpScrollBar)->klass)->slider_width +
        tmpScrollBar->style->klass->ythickness * 2;
    gtk_widget_unref(tmpScrollBar);
    return result;
}

int32 
FScrollBar::HScrollBarHeight()
{
	GtkWidget *tmpScrollBar = gtk_hscrollbar_new(NULL);
	int32 result = GTK_RANGE_CLASS(((struct _GtkObject *)tmpScrollBar)->klass)->slider_width +
        tmpScrollBar->style->klass->ythickness * 2;
    gtk_widget_unref(tmpScrollBar);
    return result;
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
