#ifndef _NETSERVERPREFSVIEW_H
#define _NETSERVERPREFSVIEW_H

#include <View.h>
#include <Message.h>

class BBox;
class BMenuItem;
class BMenuField;
class BButton;
class BListView;
class BStringView;
class BColumnListView;
class ServerEntryWindow;

struct ServerData;

class NetPrefsServerView : public BView
{
	public:
					NetPrefsServerView (BRect, const char *);
	virtual			~NetPrefsServerView (void);
	virtual void	MessageReceived (BMessage *);
	virtual void	AttachedToWindow (void);
	virtual void	DetachedFromWindow (void);
	void			SetNetworkData (BMessage *);
	
	private:

	void			AddServer (const ServerData *);
	void			RemoveServer (void);
	void            UpdateNetworkData (const ServerData *);
	BStringView *selectTitleString;
	BColumnListView   *serverList;
	BButton     *addButton,
	            *editButton,
	            *removeButton;
	BWindow *entryWin;
	BMessage *activeNetwork;
};

#endif
