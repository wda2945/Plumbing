//
//  ps_packet_serial.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_serial_rtos.hpp"

void packet_serial_thread_wrapper(void *parameters)
{
    ps_packet_serial_rtos *psn = static_cast<ps_packet_serial_rtos*>(parameters);
    psn->packet_serial_rx_thread_method();
}

ps_packet_serial_rtos::ps_packet_serial_rtos(const char *name, ps_serial_class *_driver) :
    ps_packet_serial_class(name, _driver)
{
	//Create thread
    if (xTaskCreate(packet_serial_thread_wrapper, /* The function that implements the task. */
            name, /* The text name assigned to the task*/
            PACKET_SERIAL_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            PACKET_SERIAL_TASK_PRIORITY, /* The priority assigned to the task. */
            &packet_task) /* The task handle */
            != pdPASS) {
        PS_ERROR("ps_packet_serial Task fail");
    }
}

