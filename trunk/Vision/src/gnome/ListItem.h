#ifndef LIST_ITEM_H__
#define LIST_ITEM_H__

#include <PlatformDefines.h>
#include <Rect.h>

class BFont;
class BMessage;
class BOutlineListView;
class BView;

class FListItem {
public:
	FListItem(uint32 outlineLevel = 0, bool expanded = true);
	virtual				~FListItem();

	float		Height() const;
	float		Width() const;
	bool		IsSelected() const;
	void		Select();
	void		Deselect();

	virtual	void		SetEnabled(bool on);
	bool		IsEnabled() const;

	void		SetHeight(float height);
	void		SetWidth(float width);
	virtual	void		DrawItem(BView *owner,
							BRect bounds,
							bool complete = false) = 0;
	virtual	void		Update(BView *owner, const BFont *font);

	virtual status_t	Perform(perform_code d, void *arg);

	bool 		IsExpanded() const;
	void 		SetExpanded(bool expanded);
	uint32 		OutlineLevel() const;

private:
	friend class BOutlineListView;

	bool 		HasSubitems() const;

	FListItem(const FListItem &);
	FListItem	&operator=(const FListItem &);

	/* calls used by BOutlineListView*/
	bool 		IsItemVisible() const;
	void 		SetItemVisible(bool);

	float		fWidth;
	float		fHeight;
	uint32 		fLevel;
	bool		fSelected;
	bool		fEnabled;
	bool 		fExpanded;
	bool 		fHasSubitems : 1;
	bool 		fVisible : 1;
};



class FStringItem : public FListItem {
public:
	FStringItem(const char *text, uint32 outlineLevel = 0, bool expanded = true);
	virtual			~FStringItem();
				FStringItem(BMessage *data);

	virtual	void		DrawItem(BView *owner, BRect frame, bool complete = false);
	virtual	void		SetText(const char *text);
	const char		*Text() const;
	virtual	void		Update(BView *owner, const BFont *font);

	virtual status_t	Perform(perform_code d, void *arg);

private:
		FStringItem(const FStringItem &);
		FStringItem	&operator=(const FStringItem &);

		char		*fText;
		float		fBaselineOffset;
};

#endif
