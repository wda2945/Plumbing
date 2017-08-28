//
//  packet_macros.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

//Packet Types
//packet_macro(enum, name, qos)
//pubsub
packet_macro(PUBLISH_PACKET, "publish.packet", 0)
packet_macro(SUBSCRIBE_PACKET, "subscribe.packet", 0)
packet_macro(SEND_SUBS_PACKET, "send.subs.packet", 0)
//syslog
packet_macro(SYSLOG_PACKET, "syslog.packet", 0)
//registry
packet_macro(REGISTRY_UPDATE_PACKET, "registry.update.packet", 0)
packet_macro(REGISTRY_SYNC_PACKET, "registry.sync.packet", 0)
packet_macro(REGISTRY_SYNC_REQUEST, "registry.sync.request", 0)
//notify
packet_macro(EVENT_PACKET, "event.packet", 0)
packet_macro(CONDITIONS_PACKET, "conditions.packet", 0)



