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

class BColumnListView;
class ServerEntryWindow;

struct ServerData;

class NetPrefsServerView : public BView
{
public:
	NetPrefsServerView(BRect, const char*, BMessenger);
	virtual ~NetPrefsServerView();
	virtual void MessageReceived(BMessage*);
	virtual void AttachedToWindow();
	void SetNetworkData(BMessage*);
	void ClearNetworkData();

private:
	void AddServer(const ServerData*);
	void RemoveServer();
	void UpdateNetworkData(const ServerData*);

	BColumnListView* fServerList;
	BButton* fAddButton, *fEditButton, *fRemoveButton;
	BWindow* fEntryWin;
	BMessage* fActiveNetwork;
	BMessenger fNetWin;
};

#endif
