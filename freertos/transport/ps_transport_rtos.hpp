//
//  ps_transport_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_rtos_hpp
#define ps_transport_rtos_hpp

#include "transport/ps_transport_class.hpp"
#include "queue/ps_queue_rtos.hpp"

using namespace std;

class ps_transport_rtos : public ps_transport_class
{

public:
    //init a transport with the provided packet driver
	ps_transport_rtos(ps_packet_class *driver);
    ps_transport_rtos(const char *name, ps_packet_class *driver);
    
    ~ps_transport_rtos();

protected:
        
    TaskHandle_t transport_task;
    
    SemaphoreHandle_t rx_status_semaphore;
    ps_transport_status_enum newRxStatus;
    ps_transport_status_enum send_and_confirm(transport_packet_t *pkt, int len) ;
    void new_packet_status(ps_transport_status_enum status);
    
    friend void transport_thread_wrapper(void *parameters);
};

#endif /* ps_transport_rtos_hpp */
