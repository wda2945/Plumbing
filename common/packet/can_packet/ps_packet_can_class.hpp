//
//  ps_packet_can_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//
//This class encapsulates an CANBUS link to a specific remote station
//It is wrapped by a transport object
//More than one _can_class objects share a single can_module object

//This class dis-assembles and re-assembles messages to fit into CAN packets.

#ifndef ps_packet_can_class_hpp
#define ps_packet_can_class_hpp

#include "ps.h"
#include "packet/ps_packet_class.hpp"
#include "packet/can_packet/ps_packet_can_module.hpp"

class ps_packet_can_module;

class ps_packet_can_class : public ps_packet_class {

public:
	ps_packet_can_class(const char *name, ps_packet_can_module *_can, can_address_t address);
	~ps_packet_can_class();
    
    ps_packet_can_module *can_module;
	can_address_t can_address;
    
    //send packet
    ps_result_enum send_packet(const void *_packet, int length) override;

    //receive packet)
    void new_can_packet(const can_packet_t *pkt);
    
    //errors
    void process_observed_event(ps_root_class *src, int event) override {};

    //not used
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
  
protected:

};

#endif /* ps_packet_can_class_hpp */
