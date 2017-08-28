//
//  ps_packet_can_module.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_can_module.hpp"

ps_packet_can_module::ps_packet_can_module()
{
}

ps_packet_can_module::~ps_packet_can_module()
{
}

//associate a callback with a filter address pattern
ps_result_enum ps_packet_can_module::can_subscribe(can_address_t address, can_packet_handler_t *mh, void *args) {
    can_callback_t callback;
    
    callback.address = address;
    callback.message_handler = mh;
    callback.args = args;
    callback.filterNumber = add_can_filter(address);
    
    callbacks.insert(std::make_pair(address, callback));
    
    return PS_OK;
}

void ps_packet_can_module::can_rx_thread()
{
    while (1)
    {
        
    }
}

//************************************************************************************
//c api in ps_can.h

//set filter mask - one only
ps_result_enum ps_can_set_filter_mask(can_address_t mask)
{
    the_can_module().can_set_filter_mask( mask);
    return PS_OK;
}

//suscribe to filter address. Receive a callback when a message is received.
ps_result_enum ps_can_subscribe(can_address_t address, can_packet_handler_t *mh, void *args)
{
    return the_can_module().can_subscribe(address, mh, args);
}

//send a packet
ps_result_enum ps_can_send_packet(can_packet_t *packet, int priority)
{
    return the_can_module().can_send_packet(packet, priority);
}