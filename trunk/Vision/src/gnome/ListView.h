// FListView-like class
//
// see License at the end of file

#ifndef FLISTVIEW__
#define FLISTVIEW__

#include "PlatformDefines.h"

#include <Invoker.h>
#include <List.h>
#include <ListItem.h>
#include <View.h>
#include <ScrollView.h>

struct track_data;

enum list_view_type {
	B_SINGLE_SELECTION_LIST,
	B_MULTIPLE_SELECTION_LIST
};

/*----------------------------------------------------------------*/
/*----- FListView class ------------------------------------------*/

class FListView : public FView
{

public:
	FListView(BRect frame, const char *name,
		  list_view_type type = B_SINGLE_SELECTION_LIST,
		  uint32 resizeMask = B_FOLLOW_LEFT | B_FOLLOW_TOP,
		  uint32 flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
	virtual					~FListView();
	
	virtual	void			Draw(BRect updateRect);
	virtual	void			MessageReceived(BMessage *msg);
	virtual	void			MouseDown(BPoint where);
	virtual	void			KeyDown(const char *bytes, int32 numBytes);
	virtual	void			MakeFocus(bool state = true);
	virtual	void			FrameResized(float newWidth, float newHeight);
	virtual	void			TargetedByScrollView(BScrollView *scroller);
	void				ScrollTo(float x, float y);
	virtual	void			ScrollTo(BPoint where);
	virtual	bool			AddItem(BListItem *item);
	virtual bool			AddItem(BListItem *item, int32 atIndex);
	virtual bool			AddList(BList *newItems);
	virtual bool			AddList(BList *newItems, int32 atIndex);
	virtual bool			RemoveItem(BListItem *item);
	virtual BListItem		*RemoveItem(int32 index);
	virtual bool			RemoveItems(int32 index, int32 count);

	virtual	void			SetSelectionMessage(BMessage *message);
	virtual	void			SetInvocationMessage(BMessage *message);

	BMessage			*SelectionMessage() const;
	uint32				SelectionCommand() const;
	BMessage			*InvocationMessage() const;
	uint32				InvocationCommand() const;

	virtual	void			SetListType(list_view_type type);
	list_view_type			ListType() const;

	BListItem			*ItemAt(int32 index) const;
	int32				IndexOf(BPoint point) const;
	int32				IndexOf(BListItem *item) const;
	BListItem			*FirstItem() const;
	BListItem			*LastItem() const;
	bool				HasItem(BListItem *item) const;
	int32				CountItems() const;
	virtual	void			MakeEmpty();
	bool				IsEmpty() const;
	void				DoForEach(bool (*func)(BListItem *));
	void				DoForEach(bool (*func)(BListItem *, void *), void *);
	const BListItem			**Items() const;
	void				InvalidateItem(int32 index);
	void				ScrollToSelection();

	void			Select(int32 index, bool extend = false);
	void			Select(int32 from, int32 to, bool extend = false);
	bool			IsItemSelected(int32 index) const;
	int32			CurrentSelection(int32 index = 0) const;
	virtual	status_t		Invoke(BMessage *msg = NULL);

	void			DeselectAll();
	void			DeselectExcept(int32 except_from, int32 except_to);
	void			Deselect(int32 index);

	virtual void			SelectionChanged();

	void			SortItems(int (*cmp)(const void *, const void *));


	/* These functions bottleneck through DoMiscellaneous() */
	bool			SwapItems(int32 a, int32 b);
	bool			MoveItem(int32 from, int32 to);
	bool			ReplaceItem(int32 index, BListItem * item);

	virtual	void		AttachedToWindow();
	virtual	void		FrameMoved(BPoint new_position);

	BRect			ItemFrame(int32 index);

	virtual BHandler		*ResolveSpecifier(BMessage *msg,
										int32 index,
										BMessage *specifier,
										int32 form,
										const char *property);
	virtual status_t		GetSupportedSuites(BMessage *data);

	virtual status_t		Perform(perform_code d, void *arg);

	virtual void			WindowActivated(bool state);
	virtual	void			MouseUp(BPoint pt);
	virtual	void			MouseMoved(BPoint pt, uint32 code, const BMessage *msg);
	virtual	void			DetachedFromWindow();
	virtual bool			InitiateDrag(BPoint pt, int32 itemIndex, 
										bool initialySelected);
			
	virtual void		ResizeToPreferred();
	virtual void		GetPreferredSize(float *width, float *height);
	virtual void		AllAttached();
	virtual void		AllDetached();

protected:

	enum MiscCode { B_NO_OP, B_REPLACE_OP, B_MOVE_OP, B_SWAP_OP };
	union MiscData {
			struct Spare { int32 data[5]; };
			struct Replace { int32 index; BListItem * item; } replace;
			struct Move { int32 from; int32 to; } move;
			struct Swap { int32 a; int32 b; } swap;
		};
	virtual	bool			DoMiscellaneous(MiscCode code, MiscData * data);

/*----- Private or reserved -----------------------------------------*/

private:
	friend class BOutlineListView;

		FListView		&operator=(const FListView &);

		void			InitObject(list_view_type type);
		void			FixupScrollBar();
		void			InvalidateFrom(int32 index);
		status_t		PostMsg(BMessage *msg);
		void			FontChanged();
		int32			RangeCheck(int32 index);
		bool			_Select(int32 index, bool extend);
		bool			_Select(int32 from, int32 to, bool extend);
		bool			_Deselect(int32 index);
		void			Deselect(int32 from, int32 to);
		bool			_DeselectAll(int32 except_from, int32 except_to);
		void			PerformDelayedSelect();
		bool			TryInitiateDrag(BPoint where);
		int32			CalcFirstSelected(int32 after);
		int32			CalcLastSelected(int32 before);
	virtual void			DrawItem(BListItem *item, BRect itemRect, 
							bool complete = false);

		bool			DoSwapItems(int32 a, int32 b);
		bool			DoMoveItem(int32 from, int32 to);
		bool			DoReplaceItem(int32 index, BListItem * item);
		void			RescanSelection(int32 from, int32 to);
		void			DoMouseUp(BPoint where);
		void			DoMouseMoved(BPoint where);

		BList			fList;
		list_view_type	fListType;
		int32			fFirstSelected;
		int32			fLastSelected;
		int32			fAnchorIndex;
		float			fWidth;
		BMessage		*fSelectMessage;
		BScrollView		*fScrollView;
		track_data		*fTrack;
		uint32			_reserved[3];
};

	inline void	FListView::ScrollTo(float x, float y)
	/* OK, no private parts */
	{ ScrollTo(BPoint(x, y)); }

#endif


/*
License

Terms and Conditions

Copyright (c) 1999-2001, Pavel Cisler
Copyright (c) 1999-2001, Gene Ragan

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
