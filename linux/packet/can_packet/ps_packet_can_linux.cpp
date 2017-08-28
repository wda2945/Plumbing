//
//  ps_packet_can_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_can_linux.hpp"


ps_packet_can_module& the_can_module()
{
    static ps_packet_can_linux c;
    return c;
}

ps_packet_can_linux::ps_packet_can_linux()
{

}

int ps_packet_can_linux::can_set_filter_mask(can_address_t mask)
{
	return 0;
}
//send a packet
ps_result_enum ps_packet_can_linux::can_send_packet(can_packet_t *packet, int priority)
{
	return PS_OK;
}

can_packet_t *ps_packet_can_linux::get_next_can_packet()
{
	return nullptr;
}

int ps_packet_can_linux::add_can_filter(can_address_t address)
{
	return 0;
}
