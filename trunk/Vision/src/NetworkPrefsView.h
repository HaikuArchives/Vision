#ifndef _NETWORKPREFSVIEW_H
#define _NETWORKPREFSVIEW_H

#include <View.h>
#include <Message.h>

class BBox;
class BMenuItem;
class BMenuField;
class BButton;
class BScrollView;
class BCheckBox;
class BTextView;
class BListView;
class BTextControl;
class BStringView;
class PromptWindow;
class NetPrefServerWindow;
class BColumnListView;
class TSpeedButton;
struct ServerData;

class NetworkPrefsView : public BView
{
	public:

					NetworkPrefsView (BRect, const char *);
	virtual			~NetworkPrefsView (void);
	virtual void	MessageReceived (BMessage *);
	virtual void	AttachedToWindow (void);
	virtual void	DetachedFromWindow (void);

	private:
	
	void			SetConnectServer (const char *);
	void			SetAlternateCount (uint32);
	void			UpdateNetworkData (BMessage &);
	void			UpdatePersonalData (BMessage &);
	void			SetupDefaults (BMessage &);
	void            BuildNetworkList (void);
	void            SaveCurrentNetwork();
	BMenuField *networkMenu;
	BScrollView *execScroller,
				*nickScroller;
	
	BBox *mainNetBox,
		*netDetailsBox,
		*personalBox;
	
	BButton *serverButton,
			*execButton,
			*nickAddButton,
			*nickRemoveButton;
	
	TSpeedButton *nickUpButton,
	             *nickDnButton;
	
	BCheckBox *nickDefaultsBox,
			  *startupBox;
			  
	BTextView *textView;
	BListView *listView;
	
	BTextControl *ident,
				*realName;
	
	BStringView *connectServer,
				*alternates;
	
	BMessage	activeNetwork;
	PromptWindow *nickPrompt;
	PromptWindow *netPrompt;
	PromptWindow *dupePrompt;
	BMenuItem   *removeItem;
	BMenuItem   *dupeItem;
	NetPrefServerWindow *serverPrefs;
};

#endif
