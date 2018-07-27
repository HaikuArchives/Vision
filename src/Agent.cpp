#include "Agent.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Agent"

Agent::Agent()
{
	
}

Agent::~Agent()
{
	delete fAgentWinItem;
}

BView* Agent::View()
{
	return NULL;
}

