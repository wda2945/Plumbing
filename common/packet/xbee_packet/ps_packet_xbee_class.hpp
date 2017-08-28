//
//  ps_packet_xbee_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//
//This class encapsulates an XBee link to a specific remote station
//It is wrapped by a transport object
//More than one _xbee_class objects share a single xbee_module object

#ifndef ps_packet_xbee_class_hpp
#define ps_packet_xbee_class_hpp

#include "packet/ps_packet_class.hpp"
#include "queue/ps_queue_class.hpp"

class ps_packet_xbee_module;

class ps_packet_xbee_class : public ps_packet_class {

public:
	ps_packet_xbee_class(const char *name, ps_packet_xbee_module *_xbee, uint16_t address);
	~ps_packet_xbee_class(){}
    
    ps_packet_xbee_module *xbee_object;
	uint16_t end_address;
    
    //send packet
    ps_result_enum send_packet(const void *packet, int length) override;
    
    //errors
    void process_observed_event(ps_root_class *src, int event) override;

    //not used
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}

    void pass_new_packet(const void *pkt, int len) {pass_new_data(this, pkt, len);}
    
protected:

};

#endif /* ps_packet_xbee_class_hpp */
