#ifndef	LIST_H__
#define	LIST_H__

#include "PlatformDefines.h"

class FList {

public:
	FList(int32 itemsPerBlock = 20);
	FList(const FList&);
	virtual ~FList();
	
	FList	&operator=(const FList &from);

	// Adding and removing items
	bool	AddItem(void *item);
	bool	AddItem(void *item, int32 atIndex);
	bool	AddList(FList *newItems);
	bool	AddList(FList *newItems, int32 atIndex);
	bool	RemoveItem(void *item);
	void	*RemoveItem(int32 index);
	bool	RemoveItems(int32 index, int32 count);
	bool	ReplaceItem(int32 index, void *newItem);
	void	MakeEmpty();

	// Reordering items
	void	SortItems(int (*cmp)(const void *, const void *));
	bool	SwapItems(int32 indexA, int32 indexB);
	bool	MoveItem(int32 fromIndex, int32 toIndex);

	// Retrieving items
	void	*ItemAt(int32) const;
	void	*ItemAtFast(int32) const;
	void	*FirstItem() const;
	void	*LastItem() const;
	void	*Items() const;

	// Querying the list
	bool	HasItem(void *item) const;
	int32	IndexOf(void *item) const;
	int32	CountItems() const;
	bool	IsEmpty() const;

	// Iterating over the list
	void	DoForEach(bool (*func)(void *));
	void	DoForEach(bool (*func)(void *, void *), void *);

private:
	void	Resize(int32 count);

	void	**fObjectList;
	size_t	fPhysicalSize;
	int32	fItemCount;
	int32	fBlockSize;
};


#endif
