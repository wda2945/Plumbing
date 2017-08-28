//
//  ps_network.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_network.hpp"
#include <string.h>

ps_network& the_network()
{
	static ps_network network;
	return network;
}

ps_network::ps_network() : ps_root_class(std::string("network"))
{
    PS_TRACE("network: new");
    INIT_MUTEX(networkMtx);
}

void ps_network::process_observed_event(ps_root_class *_src, int _event)
{
    PS_TRACE("network: process_observed_event");

    ps_transport_class *pst = dynamic_cast<ps_transport_class*>(_src);
    ps_transport_event_enum ev = static_cast<ps_transport_event_enum>(_event);
    
    //pass to network observers
    notify_new_event(pst, ev);
}

void ps_network::add_transport_to_network(ps_transport_class *pst)
{
    PS_TRACE("network: add_transport_to_network");

	LOCK_MUTEX(networkMtx);

	transports.push_back(pst);

	UNLOCK_MUTEX(networkMtx);

    pst->add_event_observer(this);

    //notify network observers
    notify_new_event(pst, PS_TRANSPORT_ADDED);
}

void ps_network::iterate_transports(ps_root_class *cb)
{
    PS_TRACE("network: iterate_transports");

	LOCK_MUTEX(networkMtx);
    for (const auto pst : transports)
    {
        cb->process_observed_event(pst, PS_TRANSPORT_ADDED);
    }
	UNLOCK_MUTEX(networkMtx);
}

ps_transport_class *ps_network::get_transport_by_name(const char *_name)
{
    PS_TRACE("network: get_transport_by_name");
	LOCK_MUTEX(networkMtx);
	for (const auto& pst : transports)
	{
		if (pst->name.compare(_name) == 0)
		{
			UNLOCK_MUTEX(networkMtx);
			return pst;
		}
	}
	UNLOCK_MUTEX(networkMtx);

	return nullptr;
}

ps_result_enum ps_send_network_stats()
{
    PS_TRACE("network: ps_send_network_stats");
    return PS_OK;
}

