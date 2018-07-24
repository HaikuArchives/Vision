
#include <Beep.h>
#include <FilePanel.h>
#include <Layout.h>
#include <MenuItem.h>
#include <Notification.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <SplitView.h>
#include <ScrollView.h>
#include <TextControl.h>

#include <stdio.h>


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

void Agent::ActivateInputBox()
{
	// do nothing implemented in ClientAgent;	
}
