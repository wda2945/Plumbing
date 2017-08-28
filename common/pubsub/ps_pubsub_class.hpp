//
//  ps_pubsub_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_class_hpp
#define ps_pubsub_class_hpp

#include "ps_config.h"
#include "ps_common.h"
#include "common/ps_root_class.hpp"
#include "transport/ps_transport_class.hpp"
#include "queue/ps_queue_class.hpp"
#include <vector>
#include <map>
#include <set>

#include "pubsub_header.h"  

//composite struct sent between brokers
typedef struct {
        ps_pubsub_header_t prefix;
        uint8_t message[MAX_PS_PAYLOAD];
} pubsub_packet_t;

//struct to record topic subscriptions
typedef struct {
	message_handler_t 		*messageHandler;
	std::set<ps_topic_id_t> topicList;
} psClient_t;

extern const char *packet_type_names[]; 	//name lookup from packet_macros.h

//PubSub Singleton
class ps_pubsub_class : public ps_root_class {

public:
    char **topic_names {nullptr};
    int topic_count {0};
    
    ////////////////////// PLUMBING CLIENT API
    
    //register for system packets (subscribe). Subclasses of ps_root_class only.
    ps_result_enum register_object(ps_packet_type_enum packet_type, ps_root_class *rcl);
    
    //publish a system packet via the broker queue. Message excludes pubsub prefix.
    ps_result_enum publish_packet(ps_packet_type_enum packet_type, const void *message, int length);
	//send a system packet directly to the transports - no queuing.  Message excludes pubsub prefix.
    ps_result_enum transmit_packet(ps_packet_type_enum packet_type, const void *message, int length);

    ////////////////////// PUBLIC API (calls in ps_pubsub.h)
    
    //suscribe to a topic. Receive a callback when a message is received
    ps_result_enum subscribe(ps_topic_id_t topicId, message_handler_t *msgHandler);
    //publish a message to a topic
    ps_result_enum publish(ps_topic_id_t topicId, const void *message, int length);

	////////////////////// TRANSPORT API
	
    //manage transport link subscriptions
    ps_result_enum subscribe_to_topic(ps_topic_id_t topicId, ps_transport_class *pst);
    ps_result_enum subscribe_to_packet(ps_packet_type_enum packet_type, ps_transport_class *pst);

    //callbacks from transports
    void process_observed_data(ps_root_class *src, const void *msg, int length) override;
    void process_observed_event(ps_root_class *src, int event) override;

	//////////////////////////////////////////////////////////////////////////
    //not used
    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override {}
protected:
    ps_pubsub_class();
    ~ps_pubsub_class(){}

	////////////////////// BROKER DATABASE

    //registered plumbing clients
	std::set<ps_root_class *> registered_objects[PACKET_TYPES_COUNT];  //one per packet type

	//registered transport links
    std::set<ps_transport_class*> transports;
    
    //list of client structs
	std::set<psClient_t*> clientList;

	//internal queue of messages
    ps_queue_class *brokerQueue {nullptr};

    /////////////////////// INTERNAL BROKER METHODS

    //send a message/packet to the subscribers & transports
    
    //user topic messages
    void forward_topic_message(const pubsub_packet_t *packet, int length);	//packet includes prefix

    //internal plumbing packets
    void forward_system_packet(const pubsub_packet_t *packet, int length);	//packet includes prefix

    //broker thread
	void broker_thread_method();

	void refresh_network();

	friend ps_pubsub_class& the_broker();

	//mutex
    DECLARE_MUTEX(pubsubMtx);
    
};

ps_pubsub_class& the_broker();

#endif /* ps_pubsub_class_hpp */
