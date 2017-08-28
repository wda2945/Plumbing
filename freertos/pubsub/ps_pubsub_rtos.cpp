//
//  ps_pubsub_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_pubsub_rtos.hpp"

ps_pubsub_class& the_broker()
{
	static ps_pubsub_rtos p;
	return p;
}

//task wrapper
void broker_thread_wrapper(void *pvParameters)
{
    ps_pubsub_rtos *pspr = static_cast<ps_pubsub_rtos*>(pvParameters);
    pspr->broker_thread_method();
    //does not return
}

ps_pubsub_rtos::ps_pubsub_rtos()
{
	//queues for publish and admin messages
	brokerQueue = new ps_queue_rtos(MAX_PUBSUB_PACKET, BROKER_Q_PRELOAD);
    
    if (!brokerQueue)
    {
        PS_ERROR("pubsub: brokerQueue FAIL!");
        
    }
	//start bbroker thread
    if (xTaskCreate(broker_thread_wrapper, /* The function that implements the task. */
            "Broker", /* The text name assigned to the task*/
            BROKER_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            BROKER_TASK_PRIORITY, /* The priority assigned to the task. */
            &broker_task) /*The task handle */
            != pdPASS) {
        PS_ERROR("pubsub: Broker Task FAIL!");
    }
}

ps_pubsub_rtos::~ps_pubsub_rtos()
{
}
