//
//  ps_serial_class.hpp
//  RobotFramework
//
//	Generic serial service
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_class_hpp
#define ps_serial_class_hpp

#include "common/ps_root_class.hpp"
#include <stdbool.h>

typedef enum {
	PS_SERIAL_OFFLINE,
	PS_SERIAL_WRITE_ERROR,
	PS_SERIAL_READ_ERROR,
	PS_SERIAL_ONLINE
} ps_serial_status_enum;


class ps_serial_class : public ps_root_class {

public:
	ps_serial_class(const char *name);
	virtual ~ps_serial_class(){}

    //send bytes
    virtual ps_result_enum write_bytes(const void *data, int length) = 0;
    
    //receive bytes
    virtual bool data_available() = 0;
    virtual int read_bytes(void *data, int length) = 0;

    virtual ps_serial_status_enum get_serial_status();

    virtual void disconnect(){}		//called when transport goes offline

protected:
	ps_serial_status_enum serial_status;

public:
    //observer callbacks
    void process_observed_data(ps_root_class *src, const void *msg, int length){}
    void process_observed_event(ps_root_class *src, int event) {}

    void message_handler(ps_packet_source_t packet_source,
                                     ps_packet_type_t   packet_type,
                                     const void *msg, int length) {}
};

#endif /* ps_serial_class_hpp */
