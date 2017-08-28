//
//  transport_header.h
//  RobotMonitor
//
//  Created by Martin Lane-Smith on 7/6/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef transport_header_h
#define transport_header_h

typedef struct {
    uint8_t sequenceNumber;					//this packet (if a data packet)
    uint8_t lastReceivedSequenceNumber;		//last data packet received OK
    uint8_t status;							//status of last received data packet
} ps_transport_header_t;

//status codes - what happened to your data packet
typedef enum {
    PS_TRANSPORT_MSG,           //some message received
    PS_TRANSPORT_ACK,           //last message received OK
    PS_TRANSPORT_DUP,           //duplicate, ignored
    PS_TRANSPORT_SEQ_ERROR,     //out of sequence, processed
    PS_TRANSPORT_NAK,			//other error
    PS_TRANSPORT_TIMEOUT,       //(send thread waiting for status packet)
	PS_TRANSPORT_ERROR			//other send error
}ps_transport_status_enum;

//extra flags, for this message
#define PS_TRANSPORT_IGNORE_SEQ   0x10     //ignore sequence number
#define PS_TRANSPORT_RETX         0x20     //re-transmission
#define PS_STATUS_ONLY			  0x40	   //no data in packet
#define PS_PING                   0x80     //need reply to a status packet

extern const char *transport_status_names[];

#define TRANSPORT_STATUS_NAMES {\
    "PACKET_RX",\
    "ACK",\
    "DUPLICATE",\
    "SEQ_ERROR",\
    "NAK",\
    "TIMEOUT",\
	"ERROR"}

#endif /* transport_header_h */
