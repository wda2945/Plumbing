//
//  ps_packet_can_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_can_rtos_hpp
#define ps_packet_can_rtos_hpp

#include <map>

#include "packet/can_packet/ps_packet_can_module.hpp"
#include "ps_config.h"

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include <plib.h>

#define CHANNELS_IN_USE (TRANSMIT_CHANNELS + RECEIVE_CHANNELS)
#define BUFFER_SIZE     (((TRANSMIT_CHANNELS * TRANSMIT_FIFO) + (RECEIVE_CHANNELS * RECEIVE_FIFO)) * 4) //words

class ps_packet_can_rtos : public ps_packet_can_module {
public:
    ps_packet_can_rtos(CAN_MODULE _can);
    ~ps_packet_can_rtos();
    
        //specify the mask to apply to all filters
    int can_set_filter_mask(can_address_t mask) override;
    //send a packet
    ps_result_enum can_send_packet(can_packet_t *packet, int priority) override;

protected:
    
    //associate a channel with a filter address pattern
    int add_can_filter(can_address_t address) override;

    //wait for received packet
    can_packet_t *get_next_can_packet() override;

    
    uint32_t can_buffer[BUFFER_SIZE];
    
	CAN_MODULE myModule;
   
    int next_receive_channel {0};
};

#endif /* ps_packet_can_rtos_hpp */
