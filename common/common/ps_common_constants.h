//
//  ps_common_constants.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_common_constants_h
#define ps_common_constants_h

//Packet prefix defines
//Packet Types

#define packet_macro(e, name, qos) e,
typedef enum {
#include "ps_packet_macros.h"
	PACKET_TYPES_COUNT
} ps_packet_type_enum;
#undef packet_macro

#endif /* ps_common_constants_h */
