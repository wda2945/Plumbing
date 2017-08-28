//
//  ps_pubsub_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_rtos_hpp
#define ps_pubsub_rtos_hpp

#include "pubsub/ps_pubsub_class.hpp"
#include "network/ps_network.hpp"
#include "queue/ps_queue_rtos.hpp"

#include "FreeRTOS.h"
#include "task.h"

//PubSub Singleton
class ps_pubsub_rtos : public ps_pubsub_class {
protected:
    TaskHandle_t broker_task;
    
    ps_pubsub_rtos();
    ~ps_pubsub_rtos();
    
public:

    friend ps_pubsub_class& the_broker();
    friend void broker_thread_wrapper(void *pvParameters);
};

#endif /* ps_pubsub_rtos_hpp */
