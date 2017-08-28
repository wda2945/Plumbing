    //
//  ps_registry_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_registry_rtos.hpp"
#include "queue/ps_queue_rtos.hpp"

ps_registry& the_registry() {
    static ps_registry_rtos the_registry;
    return the_registry;
}

//task wrapper
void registry_thread_wrapper(void *pvParameters)
{
    ps_registry_rtos *pspr = static_cast<ps_registry_rtos*>(pvParameters);
    pspr->registry_thread_method();
    //does not return
}

ps_registry_rtos::ps_registry_rtos(){
 
	//queue for sync messages
	registryQueue = new ps_queue_rtos(MAX_PS_PACKET, REGISTRY_Q_PRELOAD);
    
    if (!registryQueue)
    {
        PS_ERROR("registry: registry Queue FAIL!");
    }
    
	//start registry thread
    if (xTaskCreate(registry_thread_wrapper, /* The function that implements the task. */
            "Registry", /* The text name assigned to the task*/
            REGISTRY_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            REGISTRY_TASK_PRIORITY, /* The priority assigned to the task. */
            NULL) /*The task handle */
            != pdPASS) {
        PS_ERROR("Registry Task FAIL!");
    }
    
}

ps_registry_rtos::~ps_registry_rtos()
{
}

