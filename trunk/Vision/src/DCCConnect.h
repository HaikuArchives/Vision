
#ifndef DCCCONNECT_H_
#define DCCCONNECT_H_

#include <View.h>
#include <String.h>
#include <File.h>

#ifdef BONE_BUILD
#include <sys/socket.h>
#include <arpa/inet.h>
#elif NETSERVER_BUILD
#include <socket.h>
#include <netdb.h>
#endif

class BMessenger;
class BStatusBar;
class BStringView;
class StopButton;
class PauseButton;

class DCCConnect : public BView
{
	StopButton			*stop;

	protected:

	BString				nick,
							file_name,
							size,
							ip,
							port;
		
	BStatusBar			*bar;
	BStringView			*label;

	int32				totalTransferred;
	int32				finalRateAverage;

	thread_id			tid;
	bool					success;
	bool					isStopped;

	virtual void		Stopped (void);
	virtual void		Lock (void);
	virtual void		Unlock (void);

	public:
							DCCConnect (
								const char *,
								const char *,
								const char *,
								const char *,
								const char *);
	virtual				~DCCConnect (void);

	virtual void		AttachedToWindow (void);
	virtual void		AllAttached (void);
	virtual void		DetachedFromWindow (void);
	virtual void		Draw (BRect);
	virtual void		MessageReceived (BMessage *);
	static void			UpdateBar (const BMessenger &, int, int, uint32, bool);
	static void			UpdateStatus (const BMessenger &, const char *);

};

class DCCReceive : public DCCConnect
{
	friend DCCConnect;
    protected:
	bool					resume;

	public:
							DCCReceive (
								const char *,
								const char *,
								const char *,
								const char *,
								const char *,
								bool);
	virtual				~DCCReceive (void);
	virtual void		AttachedToWindow (void);
	static int32		Transfer (void *);
};

class DCCSend : public DCCConnect
{
	friend DCCConnect;
	BMessenger			caller;
	int64				pos;
	struct in_addr		addr;
	

	public:
							DCCSend (
								const char *,
								const char *,
								const char *,
								const BMessenger &,
								struct in_addr);
	virtual				~DCCSend (void);
	virtual void		AttachedToWindow (void);
	static int32		Transfer (void *);
	bool					IsMatch (const char *, const char *) const;
	void					SetResume (off_t);
	
};

uint32 const M_DCC_FINISH					= 'fnsh';

#endif
