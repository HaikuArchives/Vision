// Fake node monitoring
//
// see License at the end of file

#ifndef NODE_MONITOR_H_
#define NODE_MONITOR_H_

#include "PlatformDefines.h"
#include "Entry.h"

const uint32 B_NODE_MONITOR = 'ndmn';
const uint32 B_STAT_CHANGED = 'stch';
const uint32 B_ENTRY_MOVED = 'emov';
const uint32 B_ENTRY_REMOVED = 'ermv';

const uint32 B_WATCH_DIRECTORY = 1;
const uint32 B_WATCH_NAME = 2;
const uint32 B_WATCH_STAT = 4;
const uint32 B_STOP_WATCHING = 0;

class EntryRef;
class FHandler;
class NodeMonitorDetails;

status_t watch_node(const EntryRef *, uint32, FHandler *);

class NodeMonitor {
public:
	static NodeMonitor *GetNodeMonitor();
	
	~NodeMonitor();
	
	void Tickle();
	void Tickle(const EntryRef *);

	static void WatchNode(const EntryRef *, uint32, FHandler *);
	
private:
	NodeMonitor();

	NodeMonitorDetails *fDetails;

	static NodeMonitor *nodeMonitor;
};

#endif

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
