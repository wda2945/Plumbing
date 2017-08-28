//
//  ps_packet_can_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_can_linux_hpp
#define ps_packet_can_linux_hpp

#include "packet/can_packet/ps_packet_can_module.hpp"
#include "ps_config.h"

class ps_packet_can_linux : public ps_packet_can_module {
public:
	ps_packet_can_linux();
    virtual ~ps_packet_can_linux() {}

        //specify the mask to apply to all filters
    int can_set_filter_mask(can_address_t mask) override;
    //send a packet
    ps_result_enum can_send_packet(can_packet_t *packet, int priority) override;

protected:
    can_packet_t *get_next_can_packet() override;
    int add_can_filter(can_address_t address) override;
};

#endif /* ps_packet_can_linux_hpp */
