//
//  ps_pubsub_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_pubsub_linux.hpp"
#include <set>
#include <signal.h>

ps_pubsub_class& the_broker()
{
	static ps_pubsub_linux p;
	return p;
}

ps_pubsub_linux::ps_pubsub_linux() : ps_pubsub_class()
{
	//queues for publish and admin messages
	brokerQueue = new ps_queue_linux(MAX_PS_PACKET + sizeof(ps_pubsub_header_t), 100);

	if (!brokerQueue)
	{
		PS_ERROR("pub: failed to create Q");
	}
	else
	{
		//start thread for messages
		broker_thread = new std::thread([this](){linux_broker_thread_method();});
	}
}

ps_pubsub_linux::~ps_pubsub_linux()
{
    delete broker_thread;
}

void ps_pubsub_linux::linux_broker_thread_method()
{
	const struct sigaction sa {{SIG_IGN}, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

	broker_thread_method();
}
