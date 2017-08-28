//
//  ps_packet_xbee_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_xbee_rtos.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

//task wrapper function
void XBEE_task_wrapper(void *pvParameters)
{
    ps_packet_xbee_rtos *xb = static_cast<ps_packet_xbee_rtos*>(pvParameters);
    xb->RxThread();
    //does not return
}

ps_packet_xbee_rtos::ps_packet_xbee_rtos(ps_serial_class *_driver)
    : ps_packet_xbee_module(_driver) {
    
    txStatusSemaphore = xSemaphoreCreateBinary();
    if (txStatusSemaphore == nullptr)
    {
        PS_ERROR("XBEE Tx Semaphore FAIL!");
    }
    
    atResponseSemaphore = xSemaphoreCreateBinary();
    if (atResponseSemaphore == nullptr)
    {
        PS_ERROR("XBEE AT Response Semaphore FAIL!");
    }
        //start the task
    if (xTaskCreate(XBEE_task_wrapper, /* The function that implements the task. */
            "XBEE Rx", /* The text name assigned to the task*/
            XBEE_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
            (void *) this, /* The parameter passed to the task. */
            XBEE_TASK_PRIORITY, /* The priority assigned to the task. */
            &xbee_rx_task) 
            != pdPASS) {
        PS_ERROR("XBEE Rx Task Create FAIL!");
    }
    
    //give it time to initialize
    vTaskDelay(250);
    
    //read firmware version
    int vr = GetRegister8(FIRMWARE_VERSION);
    if (vr >= 0)
    {
        PS_DEBUG("xbee: firmware version : %x", vr);
    }
    else
    {
        PS_ERROR("xbee: get firmware version : failed");
    }
    
    //read power level
    int pl = GetRegister8(POWER_LEVEL);
    if (pl >= 0)
    {
        PS_DEBUG("xbee: power_level: %d", pl);
    }
    else
    {
        PS_ERROR("xbee: get power_level: failed");
    }
    
}

XBeeTxStatus_enum ps_packet_xbee_rtos::wait_for_tx_status(const void *header, int headerLen, const void *pkt, int len)
{
    txStatus = XBEE_TX_NONE;
    
    SendApiPacket(header, headerLen, pkt, len);

    PS_TRACE("xbee: wait_for_tx_status");

    do {

        if (xSemaphoreTake(txStatusSemaphore, XBEE_TX_STATUS_WAIT_MS) == pdFALSE) {
            PS_ERROR("xbee: No Tx Status");
            return XBEE_NO_STATUS;
        }

    } while (txStatus == XBEE_TX_NONE);

    return txStatus;
}

void ps_packet_xbee_rtos::notify_tx_status(XBeeTxStatus_enum status) {
    PS_TRACE("xbee: notify_tx_status: %s", txStatusNames[(int) status]);
    
    txStatus = status;
    
    xSemaphoreGive(txStatusSemaphore);
}

XBeeATResponseStatus_enum ps_packet_xbee_rtos::wait_for_at_response(const void *header, int headerLen, const void *pkt, int len)
{
    PS_TRACE("xbee: wait_for_at_response");

    atResponse = AT_RESPONSE_NONE;
    SendApiPacket(header, headerLen, pkt,len);
    
    do {
        if (xSemaphoreTake(atResponseSemaphore, XBEE_AT_RESPONSE_WAIT_MS) == pdFALSE) {
            PS_ERROR("xbee: No AT Response");
            return AT_RESPONSE_NONE;
        }
    } while (atResponse == AT_RESPONSE_NONE);
    
    return atResponse;
}

void ps_packet_xbee_rtos::notify_at_response(XBeeATResponseStatus_enum status) {
    
    PS_TRACE("xbee: notify_at_response: %s", ATResponseStatusNames[status]);

    atResponse = status;
    xSemaphoreGive(atResponseSemaphore);
}