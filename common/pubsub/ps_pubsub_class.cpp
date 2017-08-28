//
//  ps_pubsub_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "pubsub/ps_pubsub_class.hpp"
#include "network/ps_network.hpp"
#include <set>

ps_pubsub_class::ps_pubsub_class() : ps_root_class(std::string("broker")) {

    INIT_MUTEX(pubsubMtx);

    PS_TRACE("pubsub: new");
}
////////////////////////////// INTERNAL BROKER METHODS

//Broker thread
void ps_pubsub_class::broker_thread_method() {
    PS_DEBUG("pub: broker thread started");
    int length;

    pubsub_packet_t *ps_packet;

    the_network().add_event_observer(this);
    refresh_network();

    while (1) {

        //next message from queue
        ps_packet = (pubsub_packet_t*) brokerQueue->get_next_message(-1, &length);

        if (ps_packet) {

            if (ps_packet->prefix.packet_type > PACKET_TYPES_COUNT) {
                PS_ERROR("pub: packet_type %i!", ps_packet->prefix.packet_type);
            } else {
                PS_TRACE("pub: %s, length %d", packet_type_names[ps_packet->prefix.packet_type], length);

                switch (ps_packet->prefix.packet_type) {
                    case PUBLISH_PACKET:
                        forward_topic_message(ps_packet, length);
                        break;

                        //packet for other plumbing function
                    default:
                        forward_system_packet(ps_packet, length);
                        break;
                }
            }

            brokerQueue->done_with_message(ps_packet);
        }
    }

}

//called by broker thread to send a topic message to all subscribers
//user topics

void ps_pubsub_class::forward_topic_message(const pubsub_packet_t *packet, int len) {

    PS_TRACE("pub: forward_topic_message to topic %i, length %d", packet->prefix.topic_id, len);

    //length user message to be published
    int length = len - sizeof (ps_pubsub_header_t);
    ps_topic_id_t topic = packet->prefix.topic_id;

    LOCK_MUTEX(pubsubMtx);
    
    //iterate transports
    for (auto trns : transports) {
        //find the subscribed topic
        auto tId = trns->topicList.find(topic);
        if (tId != trns->topicList.end()) {
            //this transport is subscribed
            if (trns->source_filter[packet->prefix.packet_source] && trns->is_online()) {
                if (topic >= topic_count) {
                    PS_TRACE("pub: sending topic %i to %s, length %d", topic, trns->name.c_str(), len);
                } else {
                    PS_DEBUG("pub: sending topic %s to %s, length %d", topic_names[topic], trns->name.c_str(), len);
                }

                trns->send_packet(packet, len);
            }
        }
    }
    
    //iterate client list
    for (psClient_t *client : clientList) {
        //find the subscribed topic
        auto tId = client->topicList.find(topic);
        if (tId != client->topicList.end()) {
            //this client is subscribed
            if (topic >= topic_count) {
                PS_TRACE("pub: sending topic %i to message handler, length %d", topic, length);
            } else {
                PS_TRACE("pub: sending topic %s to message handler, length %d", topic_names[topic], length);
            }

            //pointer to messageHandler C function
            (client->messageHandler)(packet->message, length);
        }
    }
    
    UNLOCK_MUTEX(pubsubMtx);
}

//called by broker thread to send a system message to all plumbing subscribers

void ps_pubsub_class::forward_system_packet(const pubsub_packet_t *packet, int len) {

    //access the pubsub prefix
    ps_packet_type_t packet_type = (packet->prefix.packet_type);

    if (packet_type < PACKET_TYPES_COUNT) {
        PS_TRACE("pub: forward_system_packet(%s) length %d", packet_type_names[packet_type], len);

        LOCK_MUTEX(pubsubMtx);

        //iterate transports
        for (auto trns : transports) {
            //find the subscribed packet type
            auto tId = trns->packetList.find(packet_type);
            if (tId != trns->packetList.end()) {
                //this transport is subscribed
                if ((trns->source_filter[packet->prefix.packet_source]) && (trns->is_online())) {
                    trns->send_packet(packet, len);
                    PS_TRACE("pub: sending %s to %s, length %d", packet_type_names[packet_type], trns->name.c_str(), len);
                }
            }
        }

        //send to registered objects
        
        //remove pubsub header from packet
        int length = len - sizeof (ps_pubsub_header_t);

        std::set<ps_root_class *> rcl_set = registered_objects[packet_type];

        for (ps_root_class *rcl : rcl_set) {
            rcl->message_handler(packet->prefix.packet_source, packet_type, packet->message, length);
            PS_TRACE("pub: sending %s to %s, length %d", packet_type_names[packet_type], rcl->name.c_str(), length);
        }

        UNLOCK_MUTEX(pubsubMtx);
    } else {
        PS_ERROR("pub: packet_type %i!", packet_type);
    }
}

//Get the network to report all available transports >> process_observed_event(...)
void ps_pubsub_class::refresh_network() {
    PS_DEBUG("pub: refresh_network()");
    the_network().iterate_transports(this);
}

/////////////////////////// C API -> METHOD

//suscribe to topic. Receive a callback when a message is received

ps_result_enum ps_subscribe(ps_topic_id_t topic_id, message_handler_t *msgHandler) {
    return the_broker().subscribe(topic_id, msgHandler);
}

//publish a message to a topic

ps_result_enum ps_publish(ps_topic_id_t topic_id, const void *message, int length) {
    return the_broker().publish(topic_id, message, length);
}

ps_result_enum ps_register_topic_names(const char **names, int count) {
    the_broker().topic_names = const_cast<char**> (names);
    the_broker().topic_count = count;
    return PS_OK;
}

/////////////////////////// PLUMBING SERVICES

//register for system packets

ps_result_enum ps_pubsub_class::register_object(ps_packet_type_enum packet_type, ps_root_class *rcl) {
    int type = packet_type;

    LOCK_MUTEX(pubsubMtx);
    PS_DEBUG("pub: register_object(%s, %s)", packet_type_names[type], rcl->name.c_str());

    if (type < PACKET_TYPES_COUNT) {
        registered_objects[type].insert(rcl);
    }
    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

//called by other plumbing objects to send a system packet

ps_result_enum ps_pubsub_class::publish_packet(ps_packet_type_enum packet_type, const void *message, int length) {

    ps_pubsub_header_t prefix;
    prefix.packet_type = packet_type; //top bit must be 1
    prefix.packet_source = SOURCE;

    PS_TRACE("pub: publish_packet(%s)", packet_type_names[(packet_type)]);

    if (length <= (int) MAX_PS_PACKET) {
        //queue for broker thread
        if (brokerQueue->copy_2message_parts_to_q(&prefix, sizeof (ps_pubsub_header_t), message, length) != PS_OK) {
            PS_ERROR("pub: copy to Q failed");
            return PS_QUEUE_FULL;
        } else return PS_OK;
    } else {
        PS_ERROR("pub: %s packet too big: %d", packet_type_names[packet_type ], length);
        return PS_LENGTH_ERROR;
    }

    return PS_OK;
}

//Send a low priority system packet directly to the transports.
//Bypasses broker queue. Waits if transport queue not near empty.
//Message excludes pubsub prefix.
ps_result_enum ps_pubsub_class::transmit_packet(ps_packet_type_enum packet_type, const void *message, int length)
{
    ps_pubsub_header_t prefix;
    prefix.packet_type = packet_type; //top bit must be 1
    prefix.packet_source = SOURCE;

    PS_TRACE("pub: transmit_packet(%s)", packet_type_names[(packet_type)]);

    if (length <= (int) MAX_PS_PACKET) {
    
    	LOCK_MUTEX(pubsubMtx);

        auto transIterator = transports.begin();

        while (transIterator != transports.end()) {
            //find the subscribed packet type
            ps_transport_class *pst = *transIterator;

            auto tId = pst->packetList.find(packet_type);
            if (tId != pst->packetList.end()) {
                //this transport is subscribed
                UNLOCK_MUTEX(pubsubMtx);

                if ((pst->source_filter[SOURCE]) && (pst->is_online())) {
                    
                    while (pst->queue_length() > 5) {
                        //Wait & retry if queue too full
                        SLEEP_MS(1000);
                    }

                    pst->send_packet2(&prefix, sizeof (ps_pubsub_header_t), message, length);
                    
                    PS_TRACE("pub: sending %s to %s, length %d", packet_type_names[packet_type], pst->name.c_str(), length);
                }

                LOCK_MUTEX(pubsubMtx);
            }
            transIterator++;
        }
        UNLOCK_MUTEX(pubsubMtx);
        return PS_OK;
    } else {
        PS_ERROR("pub: %s packet too big: %d", packet_type_names[packet_type ], length);
        return PS_LENGTH_ERROR;
    }
}

//////////////////////////transport api

//suscribe to topic Id. Transport version.

//manage transport link subscriptions

ps_result_enum ps_pubsub_class::subscribe_to_topic(ps_topic_id_t topicId, ps_transport_class *pst) {

    if (topicId >= topic_count) {
        PS_DEBUG("pub: subscribe_to_topic(%i, %s)", topicId, pst->name.c_str());
    } else {
        PS_DEBUG("pub: subscribe_to_topic(%s, %s)", topic_names[ topicId], pst->name.c_str());
    }

    LOCK_MUTEX(pubsubMtx);

    //insert the topic
    pst->topicList.insert(topicId); //add new subscription

    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

ps_result_enum ps_pubsub_class::subscribe_to_packet(ps_packet_type_enum type, ps_transport_class *pst) {

    if (type < PACKET_TYPES_COUNT) {

        PS_DEBUG("pub: subscribe_to_packet(%s, %s)", packet_type_names[type], pst->name.c_str());

        LOCK_MUTEX(pubsubMtx);

        //insert the topic
        pst->packetList.insert(type); //add new subscription

        UNLOCK_MUTEX(pubsubMtx);

        return PS_OK;
    } else {
        PS_ERROR("pub: subscribe_to_packet(%d, %s)", type, pst->name.c_str());
        return PS_INVALID_PARAMETER;
    }
}

//transport callbacks, data and events

void ps_pubsub_class::process_observed_data(ps_root_class *_pst, const void *message, int len) {
    ps_transport_class *pst = dynamic_cast<ps_transport_class *> (_pst);

    //incoming from Transport
    const ps_pubsub_header_t *prefix = static_cast<const ps_pubsub_header_t*> (message);

    if (prefix->packet_type < PACKET_TYPES_COUNT) {
        if (prefix->packet_type == PUBLISH_PACKET) {
            if (prefix->topic_id >= topic_count) {
                PS_TRACE("pub: Received Topic %i from %s", prefix->topic_id, pst->name.c_str());
            } else {
            	PS_TRACE("pub: Received Topic %s from %s", topic_names[prefix->topic_id], pst->name.c_str());
            }

        } else {
        	PS_TRACE("pub: Received %s from %s", packet_type_names[prefix->packet_type], pst->name.c_str());
        }

        //queue it for the broker thread
        if (brokerQueue->copy_message_to_q(message, len) != PS_OK) {
            PS_ERROR("pub: copy to Q failed");
        }

    } else {
        PS_DEBUG("pub: Received bad pkt %i from %s", prefix->packet_type, pst->name.c_str());
        //discard
    }
}

void ps_pubsub_class::process_observed_event(ps_root_class *_pst, int _ev) {
    ps_transport_class *pst = dynamic_cast<ps_transport_class *> (_pst);
    ps_transport_event_enum ev = (ps_transport_event_enum) _ev;

    PS_DEBUG("pub: status %s from %s", transport_event_names[ev], pst->name.c_str());

    LOCK_MUTEX(pubsubMtx);

    switch (ev) {
        case PS_TRANSPORT_ONLINE:
        case PS_TRANSPORT_OFFLINE:
            break;
        case PS_TRANSPORT_ADDED:
        {
            transports.insert(pst);

            pst->add_data_observer(this);
        }
            break;
        case PS_TRANSPORT_REMOVED:
        {
            //remove transports entry
            auto pos = transports.find(pst);
            if (pos != transports.end()) {
                transports.erase(pos);
            }
        }
            break;
        default:
            break;
    }

    UNLOCK_MUTEX(pubsubMtx);

}

////////////////////////////public api

//suscribe to topic Id. Receive a callback when a message is received

ps_result_enum ps_pubsub_class::subscribe(ps_topic_id_t topicId, message_handler_t *msgHandler) {
    LOCK_MUTEX(pubsubMtx);

    //find the client
    for (psClient_t *client : clientList) {
        if (client->messageHandler == msgHandler) {
            client->topicList.insert(topicId); //add new subscription

            if (topicId >= topic_count) {
                PS_DEBUG("pub: subscribe to topic %i", topicId);
            } else {
                PS_DEBUG("pub: subscribe to topic %s", topic_names[topicId]);
            }

            UNLOCK_MUTEX(pubsubMtx);
            return PS_OK;
        }
    }
    //add new client
    {
        psClient_t *newClient = new psClient_t();

        if (newClient == nullptr) {
            PS_ERROR("subscribe: no memory");
            return PS_NO_MEMORY;
        }
        newClient->messageHandler = msgHandler;
        newClient->topicList.insert(topicId); //add new subscription

        clientList.insert(newClient);

        if (topicId >= topic_count) {
            PS_DEBUG("pub: new client subscribed to topic %i", topicId);
        } else {
            PS_DEBUG("pub: new client subscribed to topic %s", topic_names[topicId]);
        }

    }
    UNLOCK_MUTEX(pubsubMtx);
    return PS_OK;
}

//publish a user message to a topic

ps_result_enum ps_pubsub_class::publish(ps_topic_id_t topicId, const void *message, int length) {
    ps_pubsub_header_t prefix;
    prefix.packet_type = PUBLISH_PACKET;
    prefix.topic_id = topicId;
    prefix.packet_source = SOURCE;

    if (length <= (int) MAX_PS_PACKET) {

        if (topicId >= topic_count) {
            PS_DEBUG("pub: publish to topic %i", topicId);
        } else {
            PS_TRACE("pub: publish to topic %s", topic_names[topicId]);
        }

        //queue for broker thread
        if (brokerQueue->copy_2message_parts_to_q(&prefix, sizeof (ps_pubsub_header_t), message, length) != PS_OK) {
            PS_ERROR("pub: copy to Q failed");
            return PS_QUEUE_FULL;
        } else return PS_OK;
    } else {
        if (topicId >= topic_count) {
            PS_ERROR("pub: packet to topic %i too big", topicId);
        } else {
            PS_ERROR("pub: packet to topic %s too big", topic_names[topicId]);
        }

        return PS_LENGTH_ERROR;
    }
}


#define packet_macro(e, name, qos) name,
const char *packet_type_names[] = {
#include "common/ps_packet_macros.h"
};
#undef packet_macro
