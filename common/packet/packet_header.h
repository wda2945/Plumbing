//
//  packet_header.h
//  RobotMonitor
//
//  Created by Martin Lane-Smith on 7/6/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef packet_header_h
#define packet_header_h

typedef struct {
        uint8_t     length1;
        uint8_t     length2;
        uint16_t    csum;
}ps_packet_header_t;

#endif /* packet_header_h */
