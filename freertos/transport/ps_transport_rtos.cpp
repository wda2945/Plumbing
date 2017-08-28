//
//  ps_transport_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>

#include "ps_common.h"
#include "ps_transport_rtos.hpp"
#include "network/ps_network.hpp"

using namespace std;

void transport_thread_wrapper(void *parameters)
{
    ps_transport_rtos *psn = static_cast<ps_transport_rtos*>(parameters);
    psn->transport_send_thread_method();
}

ps_transport_rtos::ps_transport_rtos(ps_packet_class *driver)
: ps_transport_rtos(driver->name.c_str(), driver){}

ps_transport_rtos::ps_transport_rtos(const char *name, ps_packet_class *driver)
: ps_transport_class(name, driver)
{
    sendQueue = new ps_queue_rtos(MAX_TRANSPORT_PACKET, TRANSPORT_Q_PRELOAD);
    
    if (!sendQueue)
    {
        PS_ERROR("trans: sendQueue FAIL!");
    }
    
    rx_status_semaphore = xSemaphoreCreateBinary();
    if (rx_status_semaphore == nullptr)
    {
        PS_ERROR("trans: rx_status_semaphore fail!");
    }
    
    	//Create thread
    if (xTaskCreate(transport_thread_wrapper, /* The function that implements the task. */
            name, /* The text name assigned to the task*/
            TRANSPORT_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            TRANSPORT_TASK_PRIORITY, /* The priority assigned to the task. */
            &transport_task) /* The task handle */
            != pdPASS) {
        PS_ERROR("ps_transport Task fail");
    }

}

ps_transport_rtos::~ps_transport_rtos() {
}

ps_transport_status_enum ps_transport_rtos::send_and_confirm(transport_packet_t *pkt, int len)  {
    PS_TRACE("tran: %s send_and_confirm", name.c_str());

    //send the packet
    ps_result_enum result = packet_driver->send_packet(pkt, len);

    if (result != PS_OK)
    {
    	return PS_TRANSPORT_ERROR;
    }
    
    uint8_t outflags = pkt->packet_header.status;
    
    int packet_wait_count = (PS_TRANSPORT_STATUS_WAIT_MSECS / PS_TRANSPORT_MESSAGE_WAIT_MSECS);

    do {
    
        if (xSemaphoreTake(rx_status_semaphore, PS_TRANSPORT_MESSAGE_WAIT_MSECS) == pdTRUE)
        {
            if (status_packet_required) send_status_only();
            
            //a message came
            if ((outflags & PS_PING) != 0) {
                PS_TRACE("tran: %s: PING returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);
                return newRxStatus;			//ping - any reply is good
            }
            if (newRxStatus != PS_TRANSPORT_MSG) {
                PS_TRACE("tran: %s: returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);
                return newRxStatus;			//relevant reply
            }
            //continue to wait for relevant message
        } else newRxStatus = PS_TRANSPORT_TIMEOUT;
        
    } while (packet_wait_count-- > 0);
    
    PS_TRACE("tran: %s: timeout. returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);

    return newRxStatus;
}

void ps_transport_rtos::new_packet_status(ps_transport_status_enum status) {

    newRxStatus = (ps_transport_status_enum)((uint8_t) status & 0xf);
    
    xSemaphoreGive(rx_status_semaphore);
    
    PS_TRACE("tran: %s new_packet_status: %s", name.c_str(), transport_status_names[(int) newRxStatus]);    
}
