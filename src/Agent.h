#ifndef _AGENT_H_
#define _AGENT_H_

#include "WindowList.h"

class Agent
{
public:
	Agent();
	virtual ~Agent();
	virtual BView *View();

	WindowListItem* fAgentWinItem;

private:

};

#endif
