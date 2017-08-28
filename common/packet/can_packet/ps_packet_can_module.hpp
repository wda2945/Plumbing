//
//  ps_packet_can_module.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_can_module_hpp
#define ps_packet_can_module_hpp

#include <map>

#include "ps.h"

typedef struct {
    can_address_t address;
    can_packet_handler_t *message_handler;
    void *args;
    int filterNumber;
}can_callback_t;

class ps_packet_can_module {
public:
    ps_packet_can_module();
    virtual ~ps_packet_can_module();
    
    //specify the mask to apply to all filters
    virtual int can_set_filter_mask(can_address_t mask) = 0;
    //associate a callback with a filter address pattern
    ps_result_enum can_subscribe(can_address_t address, can_packet_handler_t *mh, void *args);
    //send a packet
    virtual ps_result_enum can_send_packet(can_packet_t *packet, int priority) = 0;
    
    void can_rx_thread();
    
protected:
    virtual can_packet_t *get_next_can_packet() = 0;
    virtual int add_can_filter(can_address_t address) = 0;
    
    std::map<can_address_t, can_callback_t> callbacks;
};

//singleton access
ps_packet_can_module& the_can_module();

#endif /* ps_packet_can_module_hpp */
