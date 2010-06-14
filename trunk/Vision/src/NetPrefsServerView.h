#ifndef _NETSERVERPREFSVIEW_H
#define _NETSERVERPREFSVIEW_H

#include <View.h>
#include <Message.h>
#include <Messenger.h>

class BBox;
class BMessenger;
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
					NetPrefsServerView (BRect, const char *, BMessenger);
	virtual			~NetPrefsServerView (void);
	virtual void	MessageReceived (BMessage *);
	virtual void	AttachedToWindow (void);
	void			SetNetworkData (BMessage *);
	
	private:

	void			AddServer (const ServerData *);
	void			RemoveServer (void);
	void						UpdateNetworkData (const ServerData *);
	BStringView		 *fSelectTitleString,
									*fLegend4;
	BColumnListView *fServerList;
	BButton				 *fAddButton,
									*fEditButton,
									*fRemoveButton,
									*fOkButton;
	BWindow				 *fEntryWin;
	BMessage				*fActiveNetwork;
	BMessenger			fNetWin;
};

#endif
