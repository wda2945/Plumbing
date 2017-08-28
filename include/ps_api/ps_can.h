//
//  ps_can.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_can_h
#define ps_can_h

#include "ps_types.h"

//CAN client api

typedef uint32_t can_address_t;

typedef union {
 struct {
        can_address_t SID;               
        uint32_t EID;
        uint8_t data[8];  
};
int messageWord[4];
}can_packet_t;

#ifdef __cplusplus
extern "C" {
#endif

typedef void (can_packet_handler_t)(can_packet_t *, void *args);

//set filter mask - one only
ps_result_enum ps_can_set_filter_mask(can_address_t mask);

//suscribe to filter address. Receive a callback when a message is received.
ps_result_enum ps_can_subscribe(can_address_t address, message_handler_t*, void *args);

//send a packet
ps_result_enum ps_can_send_packet(can_packet_t *packet, int priority);

#ifdef __cplusplus
}
#endif


#endif /* ps_can_h */
