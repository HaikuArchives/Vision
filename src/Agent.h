#ifndef _AGENT_H_
#define _AGENT_H_

#include "WindowList.h"

class Agent
{
public:
	Agent();
	virtual ~Agent();
	virtual BView *View();
	virtual void ActivateInputBox();
	WindowListItem* fAgentWinItem;

private:

};

#endif
