// Fake node monitoring
//
// Note that the node monitoring messages are slightly different than
// the BeOS equivalents because we do not have the necessary
// i-node based APIs and entry_ref
//
// see License at the end of file

#include "NodeMonitor.h"

#include "AutoLock.h"
#include "Directory.h"
#include "Entry.h"
#include "Looper.h"
#include "Thread.h"
#include "ObjectList.h"

#include <sys/stat.h>
#include <unistd.h>
#include <gtk/gtkmain.h>

class NotificationEntry {
public:
	NotificationEntry()
		:	fNotification(0),
			fNotifyee(NULL)
		{}
	void Notify(FMessage *);
	uint32 fNotification;
	FHandler *fNotifyee;
};

class NodeMonitorEntry {
public:
	NodeMonitorEntry(const EntryRef *);
	void AddMonitor(uint32, FHandler *);
	bool RemoveMonitor(FHandler *);
		// returns true if this was the last monitor for this entry

	void Check();

private:
	NotificationEntry *Find(const FHandler *);
	
	void StatChanged(const struct stat *);
	void MovedTo(const EntryRef *, const struct stat *);
	void Removed();

	bool Watching(uint32) const;
	void Notify(BMessage *);

	EntryRef fRef;
	struct stat fLastStat;
	ObjectList<NotificationEntry> fNotifications;
	
	friend class MatchRef;
};

class NodeMonitorDetails {
public:
	NodeMonitorDetails();
	void AddMonitor(const EntryRef *, uint32, FHandler *);
	void RemoveMonitor(const EntryRef *, FHandler *);
	
	void Check();
	void Check(const EntryRef *);
	void Quit();
	
	void SetCheckRate(bigtime_t rate)
		{ fCheckInterval = rate; }

private:
	~NodeMonitorDetails();
	NodeMonitorEntry *Find(const EntryRef *);

	bool CheckOne();

#if DEBUG
	static int CheckTimeoutBinder(void *);
#else
	static status_t CheckThreadBinder(void *);
#endif	
	void CheckCommon();

	ObjectList<NodeMonitorEntry> fMonitoredEntries;

#if DEBUG
	int fCheckTimeoutID;
#else
	thread_id fCheckThread;
#endif
	FLocker fLock;

	volatile bool fQuit;
	bigtime_t fLastCheckTime;
	bigtime_t fCheckInterval;	
};

void 
NotificationEntry::Notify(FMessage *message)
{
	fNotifyee->Looper()->PostMessage(message, fNotifyee);
}


NodeMonitorEntry::NodeMonitorEntry(const EntryRef *ref)
	:	fRef(*ref),
		fNotifications(5, true)
{
	stat(fRef.Path(), &fLastStat);
}

void 
NodeMonitorEntry::AddMonitor(uint32 what, FHandler *handler)
{
	NotificationEntry *entry = Find(handler);
	if (!entry || (entry->fNotification & what) != 0) {
		// new entry or a duplicate request, create a new entry
		// in either case -- in the latter to make sure we can
		// balance out monitor removals correctly
		entry = new NotificationEntry;
		entry->fNotifyee = handler;
		fNotifications.AddItem(entry);
	}
	entry->fNotification |= what;
}

bool 
NodeMonitorEntry::RemoveMonitor(FHandler *handler)
{
	NotificationEntry *entry = Find(handler);
	if (!entry)
		return true;

	fNotifications.RemoveItem(entry);
	return Find(handler) != NULL;
}

void 
NodeMonitorEntry::Check()
{
	struct stat statBuffer;

	bool move = false;
	
	if (stat(fRef.Path(), &statBuffer) == 0) {
		if (statBuffer.st_ino != fLastStat.st_ino) {
			// got an alias
			move = true;
		}
		if (statBuffer.st_uid != fLastStat.st_uid
			|| statBuffer.st_gid != fLastStat.st_gid
			|| statBuffer.st_mode != fLastStat.st_mode
			|| statBuffer.st_size != fLastStat.st_size
			|| statBuffer.st_mtime != fLastStat.st_mtime
			|| statBuffer.st_ctime != fLastStat.st_ctime) {
			StatChanged(&statBuffer);
			return;
		}
	} else
		move = true;

	if (move) {
		// figure out if we have a rename or a move
		// -- look in the parent directory for a file
		// with the same inode

		FEntry entry(&fRef, false);
		FDirectory parent;
		entry.GetParent(&parent);
		
		for (;;) {
			EntryRef ref;
			if (parent.GetNextRef(&ref) != B_OK)
				break;
			
			struct stat statBuffer;
			if (stat(ref.Path(), &statBuffer) != 0)
				continue;
			
			if (statBuffer.st_ino == fLastStat.st_ino) {
				// found the file, it got renamed
				MovedTo(&ref, &statBuffer);
				return;
			}
		}

		// move not yet supported, just assume the file got
		// removed completely
		
		Removed();
	}
}

void 
NodeMonitorEntry::StatChanged(const struct stat *newStat)
{
	fLastStat = *newStat;

	if (!Watching(B_WATCH_STAT))
		// not interested
		return;
	
	FMessage message(B_NODE_MONITOR);
	message.AddInt32("opcode", B_STAT_CHANGED);
	message.AddRef("item", &fRef);
	Notify(&message);
}

void 
NodeMonitorEntry::MovedTo(const EntryRef *newRef, const struct stat *newStat)
{
	if (!Watching(B_WATCH_NAME)) {
		// not interested
		fLastStat = *newStat;
		fRef = *newRef;
		return;
	}

	FMessage message(B_NODE_MONITOR);
	message.AddInt32("opcode", B_ENTRY_MOVED);
	message.AddRef("from item", &fRef);
	message.AddRef("to item", newRef);

	fRef = *newRef;
	fLastStat = *newStat;
	Notify(&message);
}

void 
NodeMonitorEntry::Removed()
{
	if (!Watching(B_WATCH_NAME))
		// not interested
		return;

	FMessage message(B_NODE_MONITOR);
	message.AddInt32("opcode", B_ENTRY_REMOVED);
	message.AddRef("item", &fRef);
	Notify(&message);
}

struct MatchWhat : public UnaryPredicate<NotificationEntry> {
	MatchWhat(uint32 what)
		:	fWhat(what)
		{}

	virtual int operator()(const NotificationEntry *item) const
		{ return (item->fNotification & fWhat) != 0 ? 0 : -1; }

	uint32 fWhat;
};

bool 
NodeMonitorEntry::Watching(uint32 what) const
{
	MatchWhat predicate(what);
	return fNotifications.Search(predicate) != NULL;
}

struct MatchHandler : public UnaryPredicate<NotificationEntry> {
	MatchHandler(const FHandler *handler)
		:	fHandler(handler)
		{}

	virtual int operator()(const NotificationEntry *item) const
		{ return item->fNotifyee == fHandler ? 0 : -1; }

	const FHandler *fHandler;
};

NotificationEntry * 
NodeMonitorEntry::Find(const FHandler *handler)
{
	MatchHandler predicate(handler);
	return fNotifications.Search(predicate);
}

void 
NodeMonitorEntry::Notify(BMessage *message)
{
	EachListItem(&fNotifications, &NotificationEntry::Notify, message);
}


NodeMonitorDetails::NodeMonitorDetails()
	:	fMonitoredEntries(100, true),
		fQuit(false),
		fCheckInterval(1000000)
{
#if DEBUG
	// in the debug version use gtk_timeout instead of a thread so that
	// we get core dumps on a crash
	fCheckTimeoutID = gtk_timeout_add (50, &NodeMonitorDetails::CheckTimeoutBinder,
		this);
#else
	fCheckThread = spawn_thread(&NodeMonitorDetails::CheckThreadBinder,
		"node monitor", 10000, this);
	resume_thread(fCheckThread);
#endif
}


NodeMonitorDetails::~NodeMonitorDetails()
{
}

bool 
NodeMonitorDetails::CheckOne()
{
	AutoLock<FLocker> lock(&fLock);
	if (fQuit)
		return false;
		
	if (system_time() > fLastCheckTime
		+ fCheckInterval
		// decrease the check period a bit as we monitor more files
		+ (fCheckInterval * fMonitoredEntries.CountItems()) / 5) {
		CheckCommon();
		fLastCheckTime = system_time();
	}
	return true;
}

#if DEBUG

int 
NodeMonitorDetails::CheckTimeoutBinder(void *params)
{
	return ((NodeMonitorDetails *)params)->CheckOne();
}

#else

status_t 
NodeMonitorDetails::CheckThreadBinder(void *params)
{
	for (;;) {
		if (((NodeMonitorDetails *)params)->CheckOne())
			break;

		usleep(50000);
	}
	delete (NodeMonitorDetails *)params;
	return B_OK;
}

#endif

void 
NodeMonitorDetails::AddMonitor(const EntryRef *ref, uint32 what,
	FHandler *handler)
{
	NodeMonitorEntry *entry = Find(ref);
	if (!entry) {
		entry = new NodeMonitorEntry(ref);
		fMonitoredEntries.AddItem(entry);
	}
	entry->AddMonitor(what, handler);
}

void 
NodeMonitorDetails::RemoveMonitor(const EntryRef *ref, FHandler *handler)
{
	NodeMonitorEntry *entry = Find(ref);
	if (entry && entry->RemoveMonitor(handler))
		fMonitoredEntries.RemoveItem(entry);
}

void 
NodeMonitorDetails::Check()
{
	AutoLock<FLocker> lock(&fLock);
	CheckCommon();
}

void 
NodeMonitorDetails::Check(const EntryRef *ref)
{
	AutoLock<FLocker> lock(&fLock);
	NodeMonitorEntry *entry = Find(ref);
	if (entry)
		entry->Check();
}

void 
NodeMonitorDetails::Quit()
{
	fQuit = true;
}

void 
NodeMonitorDetails::CheckCommon()
{
	EachListItem(&fMonitoredEntries, &NodeMonitorEntry::Check);
}

struct MatchRef : public UnaryPredicate<NodeMonitorEntry> {
	MatchRef(const EntryRef *ref)
		:	fRef(ref)
		{}

	virtual int operator()(const NodeMonitorEntry *item) const
		{ return item->fRef == *fRef ? 0 : -1; }

	const EntryRef *fRef;
};

NodeMonitorEntry *
NodeMonitorDetails::Find(const EntryRef *ref)
{
	MatchRef predicate(ref);
	return fMonitoredEntries.Search(predicate);
}


NodeMonitor *
NodeMonitor::GetNodeMonitor()
{
	if (!nodeMonitor)
		nodeMonitor = new NodeMonitor;

	return nodeMonitor;
}


NodeMonitor::NodeMonitor()
	:	fDetails(new NodeMonitorDetails)
{
}

NodeMonitor::~NodeMonitor()
{
	// tell fDetails to delete themselves.
	fDetails->Quit();
}

void 
NodeMonitor::Tickle()
{
	fDetails->Check();
}

void 
NodeMonitor::Tickle(const EntryRef *ref)
{
	fDetails->Check(ref);
}

void 
NodeMonitor::WatchNode(const EntryRef *ref, uint32 what, FHandler *handler)
{
	if (what)
		GetNodeMonitor()->fDetails->AddMonitor(ref, what, handler);
	else
		GetNodeMonitor()->fDetails->RemoveMonitor(ref, handler);
}

status_t 
watch_node(const EntryRef *ref, uint32 what, FHandler *handler)
{
	ASSERT(NodeMonitor::GetNodeMonitor());
	NodeMonitor::GetNodeMonitor()->WatchNode(ref, what, handler);
	return B_OK;
}


NodeMonitor *NodeMonitor::nodeMonitor = NULL;

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
