//
//  ps_packet_xbee_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_xbee_rtos_hpp
#define ps_packet_xbee_rtos_hpp

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "packet/xbee_packet/ps_packet_xbee_module.hpp"
#include "serial/ps_serial_class.hpp"

class ps_packet_xbee_rtos : public ps_packet_xbee_module {
    
public:
        
    ps_packet_xbee_rtos(ps_serial_class *_driver);
    
protected:
        
    XBeeTxStatus_enum wait_for_tx_status(const void *header, int headerLen, const void *pkt, int len) override;
    void notify_tx_status(XBeeTxStatus_enum status) override;

    XBeeATResponseStatus_enum wait_for_at_response(const void *header, int headerLen, const void *pkt, int len) override;
    void notify_at_response(XBeeATResponseStatus_enum status) override;
    
    friend void XBEE_task_wrapper(void *pvParameters);
    
private:
    TaskHandle_t xbee_rx_task;
    
    SemaphoreHandle_t txStatusSemaphore;
    XBeeTxStatus_enum txStatus;
    
    SemaphoreHandle_t atResponseSemaphore;
    XBeeATResponseStatus_enum atResponse;
    
};

#endif /* ps_packet_xbee_rtos_hpp */
