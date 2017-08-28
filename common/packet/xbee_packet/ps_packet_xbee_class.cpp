//
//  ps_packet_xbee_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_xbee_class.hpp"
#include "ps_packet_xbee_module.hpp"

ps_packet_xbee_class::ps_packet_xbee_class(const char *name, ps_packet_xbee_module *_xbee, uint16_t address)
    : ps_packet_class(name)
{
	xbee_object = _xbee;
	end_address = address;
    
	xbee_object->add_link(this);
}

    //send packet
ps_result_enum ps_packet_xbee_class::send_packet(const void *packet, int length)
{
    return xbee_object->send_packet(packet, length, end_address);
}
    
    //errors
void ps_packet_xbee_class::process_observed_event(ps_root_class *src, int event)
{
}
