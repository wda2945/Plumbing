//
//  ps_transport_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_hpp
#define ps_transport_hpp

#include <set>
#include "common/ps_root_class.hpp"
#include "packet/ps_packet_class.hpp"
#include "transport/transport_header.h"
#include "queue/ps_queue_class.hpp"

typedef enum {
    PS_TRANSPORT_UNKNOWN,
    PS_TRANSPORT_ONLINE,
    PS_TRANSPORT_OFFLINE,
    PS_TRANSPORT_ADDED,
	PS_TRANSPORT_REMOVED
} ps_transport_event_enum;

#define TRANSPORT_EVENT_NAMES {\
    "PS_TRANSPORT_UNKNOWN",\
    "PS_TRANSPORT_ONLINE",\
    "PS_TRANSPORT_OFFLINE",\
    "PS_TRANSPORT_ADDED",\
    "PS_TRANSPORT_REMOVED"}

extern const char *transport_event_names[];

//composite struct sent between transports
typedef struct {
    ps_transport_header_t packet_header;
    uint8_t payload;         //first byte of packet payload
} transport_packet_t;

class ps_transport_class : public ps_root_class {

public:
    ps_transport_class(ps_packet_class *driver);
    ps_transport_class(const char *name, ps_packet_class *driver);
    virtual ~ps_transport_class();
  
    //packet filters
    std::set<ps_topic_id_t> topicList;      //subscribed topics
    std::set<ps_packet_type_t> packetList;  //subscribed system packets
    bool source_filter[SRC_COUNT];          //permit packets from specified source
    
    //packet layer
    ps_packet_class *packet_driver;
    
    //used by packet layer
    virtual void report_transport_event(ps_transport_event_enum newStatus);
    
    //helpers
    bool is_online()    {return transport_is_online;}
    int queue_length()  {return sendQueue->count();}    //how full is the send queue
    
    //send packet
    ps_result_enum send_packet(const void *packet, int len);
    ps_result_enum send_packet2(const void *packet1, int len1, const void *packet2, int len2);	//convenience version
    
    //not used
    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override {}
protected:
    //Q for outbound messages
    ps_queue_class *sendQueue {nullptr};
    
    //status
    bool transport_is_online {false};
    ps_transport_event_enum transport_status {PS_TRANSPORT_OFFLINE};

    //protocol data - serial numbers
    uint8_t lastReceived {0};
    uint8_t remoteLastReceived {0};
    uint8_t lastSent {0};
    
    //packet header flags
    uint8_t outboundFlags {PS_TRANSPORT_IGNORE_SEQ};				//outbound
    uint8_t remoteLastFlags {0};	//inbound
    
    //non-data status-only packet
    transport_packet_t statusPkt;
    
    //send a status packet ASAP
    bool status_packet_required {false};
    ps_result_enum send_status_only();
    
    //count of message wait intervals to send a keepalive
#define KEEPALIVE_COUNT_INITIALIZER (PS_TRANSPORT_KEEPALIVE_INTERVAL_MSECS / PS_TRANSPORT_MESSAGE_WAIT_MSECS)
    int keepalive_count {KEEPALIVE_COUNT_INITIALIZER};
    
    //count of retries before >> offline
    int keepalive_retry_count {PS_TRANSPORT_RETRIES};
    
    //transmit thread
    void transport_send_thread_method();
    
    //semaphore helpers - OS dependent
    virtual ps_transport_status_enum send_and_confirm(transport_packet_t *pkt, int len) = 0;
    virtual void new_packet_status(ps_transport_status_enum status) = 0;

public:
    //data and events from packet layer
    void process_observed_data(ps_root_class *src, const void *msg, int length) override;
    void process_observed_event(ps_root_class *src, int event) override;
    
    //comms stats - transmit
    int data_packets_sent {0};
    int data_bytes_sent {0};
    int data_packets_resent {0};
    int status_packets_sent {0};
    int timeouts {0};
    int disconnects {0};
    //comms stats - receive
    int data_packets_received {0};
    int data_bytes_received {0};
    int status_packets_received {0};
    int duplicates {0};
    int sequence_errors {0};
    int other_rx_errors {0};
};

#endif /* ps_transport_hpp */
