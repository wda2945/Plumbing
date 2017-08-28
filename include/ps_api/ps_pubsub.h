//
//  ps_pubsub.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_pubsub_h
#define ps_pubsub_h

#include "ps_types.h"

//pubsub client api

#ifdef __cplusplus
extern "C" {
#endif

//callback function definition
typedef void (message_handler_t)(const void *, int);

//suscribe to topic. Receive a callback when a message is received
ps_result_enum ps_subscribe(ps_topic_id_t topic_id, message_handler_t*);

//publish a message to a topic
ps_result_enum ps_publish(ps_topic_id_t topic_id, const void *message, int length);

//provide names to improve log messages
ps_result_enum ps_register_topic_names(const char **names, int count);

//TBD
ps_result_enum ps_send_network_stats();

#ifdef __cplusplus
}
#endif


#endif /* ps_pubsub_h */
