//
//  ps_packet_serial_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_serial_rtos_hpp
#define ps_packet_serial_rtos_hpp

#include <stdio.h>
#include "packet/serial_packet/ps_packet_serial_class.hpp"

class ps_packet_serial_rtos : public ps_packet_serial_class {
    
public:

    ps_packet_serial_rtos(const char *name, ps_serial_class *_driver);
    ~ps_packet_serial_rtos() {}
    
protected:

	TaskHandle_t packet_task;

    friend void packet_serial_thread_wrapper(void *parameters);
};

#endif /* ps_packet_serial_rtos_hpp */
