//
//  ps_registry_message.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_message_hpp
#define ps_registry_message_hpp

#include "ps_api/ps_registry.h"

//report a value change
typedef struct {
    char domain[REGISTRY_DOMAIN_LENGTH];
    char name[REGISTRY_NAME_LENGTH];
    ps_registry_api_struct value;
} ps_registry_update_packet_t;

int get_update_packet_length(ps_registry_update_packet_t& pkt);

typedef struct {
    uint8_t entry_count;                        // count of entries in the domain
    uint8_t domain_owner;                       // true if source owns this domain
    char domain[REGISTRY_DOMAIN_LENGTH + 1 ];    // extra byte to ensure zero is sent
} ps_registry_sync_packet_t;

int get_sync_packet_length(ps_registry_sync_packet_t& pkt);

#endif /* ps_registry_message_hpp */
