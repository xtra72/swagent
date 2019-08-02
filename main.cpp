/////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <signal.h>
#include "Terminal.h"
#include "SLABHIDtoUART.h"
#include <unistd.h>
#include "agent.h"
#include "mqttclient.h"
#include "config.h"

/////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////

#define ERROR_LEVEL_SUCCESS             0
#define ERROR_LEVEL_INVALID_ARG         1
#define ERROR_LEVEL_API_CODE            2

/////////////////////////////////////////////////////////////////////////////
// Namespaces
/////////////////////////////////////////////////////////////////////////////

using namespace std;

void SigintHandler(int param)
{
//	agent.Stop();
    exit(ERROR_LEVEL_SUCCESS);
}

int main(int argc, char* argv[])
{
    // Register for SIGINT events
//    signal(SIGINT, SigintHandler);
    
//	Config	config("/etc/swagent");
	Config	config("/home/ubuntu/projects/swagent/swagent.json");
   
	if (!config.Load(argc, argv))
	{
		std::cerr << "Invalid config" << std::endl;
		return	0;
	}

	Agent	agent;

	Config	trace_config;
	if (config.Find("trace", trace_config))
	{
		trace_master.Set(trace_config);
	}

	Config	agent_config;
	if (config.Find("agent", agent_config))
	{
		agent.Set(agent_config);
	}

	agent.Start();
	
	while(true)
	{
		usleep(1000);		
	}

	agent.Stop();

    return ERROR_LEVEL_SUCCESS;
}
