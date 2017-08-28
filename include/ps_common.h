//
//  ps_common.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_common_h
#define ps_common_h

#include "common/ps_common_enums.h"
#include "common/ps_common_typedefs.h"
#include "common/ps_common_constants.h"

//message size parameters
#include "messages.h"
#include "pubsub/pubsub_header.h"
#include "transport/transport_header.h"
#include "packet/packet_header.h"
#include "syslog/ps_syslog_message.h"
#include "ps_api/ps_registry.h"
#include "registry/ps_registry_message.h"

#define MAX_USER_MESSAGE         (sizeof(psMessage_t))
#define SYSLOG_PACKET_SIZE       (sizeof(ps_syslog_message_t))
#define REGISTRY_PACKET_SIZE     (sizeof(ps_registry_update_packet_t))

#define Maximum(a,b) ((a) > (b) ? (a) : (b))

#define MAX_PS_PAYLOAD (Maximum(SYSLOG_PACKET_SIZE, Maximum(REGISTRY_PACKET_SIZE, MAX_USER_MESSAGE)))
#define MAX_PUBSUB_PACKET (MAX_PS_PAYLOAD + sizeof(ps_pubsub_header_t))
#define MAX_TRANSPORT_PACKET (MAX_PUBSUB_PACKET + sizeof(ps_transport_header_t))
#define MAX_SERIAL_PACKET (MAX_TRANSPORT_PACKET + sizeof(ps_packet_header_t))

#define MAX_PS_PACKET (MAX_SERIAL_PACKET)  

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* ps_common_h */
