//
//  ps_packet_can_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "string.h"

#include "ps_packet_can_class.hpp"
#include "ps_packet_can_module.hpp"

ps_packet_can_class::ps_packet_can_class(const char *name, ps_packet_can_module *_can, can_address_t _address)
    : ps_packet_class(name)
{
	can_module = _can;
	can_address = _address;
}

ps_packet_can_class::~ps_packet_can_class()
{
}
    //send packet
ps_result_enum ps_packet_can_class::send_packet(const void *_packet, int length)
{

    return PS_OK;
}
    
    //received packet
void ps_packet_can_class::new_can_packet(const can_packet_t *packet)
{

}
