//
//  pubsub_header.h
//  RobotMonitor
//
//  Created by Martin Lane-Smith on 7/6/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef pubsub_header_h
#define pubsub_header_h

#include "common/ps_common_typedefs.h"

//pubsub message prefix
typedef struct {
    ps_packet_source_t		packet_source;
    ps_packet_type_t 		packet_type;
    ps_topic_id_t           topic_id;
} ps_pubsub_header_t;

#endif /* pubsub_header_h */
