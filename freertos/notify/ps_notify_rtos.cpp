//
//  ps_notify_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//



#include "ps_notify_rtos.hpp"

void notify_thread_wrapper(void *parameters)
{
    ps_notify_rtos *psn = static_cast<ps_notify_rtos*>(parameters);
    psn->notify_conditions_thread();
}

ps_notify_class& the_notifier()
{
    static ps_notify_rtos the_notifier;
    return the_notifier;
}

ps_notify_rtos::ps_notify_rtos()
{
	//Create thread
    if (xTaskCreate(notify_thread_wrapper, /* The function that implements the task. */
            "Notifier", /* The text name assigned to the task*/
            NOTIFY_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            NOTIFY_TASK_PRIORITY, /* The priority assigned to the task. */
            &notify_task) /* The task handle */
            != pdPASS) {
        PS_ERROR("ps_notify Task fail");
    }
}

ps_notify_rtos::~ps_notify_rtos()
{
}

