//
//  ps_types.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_types_h
#define ps_types_h

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

//subsystems and sources
typedef enum {SRC_UNKNOWN,
    SRC_IOSAPP,
    SRC_GATEWAY,
    SRC_FIDOJR_PIC,
    SRC_FIDOJR_EDI,
	SRC_FIDOJR_EUC,
    SRC_HEXAPOD,
    SRC_FIDOBBB, SRC_FIDOMCP, SRC_FIDOMOT,
    SRC_4SERVO, SRC_4MIND,
    SRC_COUNT
} Source_t;

//response and error codes
typedef enum {
    PS_OK,
    PS_NOT_IMPLEMENTED,
    PS_NAME_NOT_FOUND,
    PS_NAME_EXISTS,
    PS_LENGTH_ERROR,
    PS_IO_ERROR,
	PS_INVALID_PARAMETER,
	PS_WRONG_DATA_TYPE,
    PS_IO_INIT_FAIL,
    PS_NO_MEMORY,
    PS_TASK_INIT_FAIL,
    PS_QUEUE_FULL,
    PS_EXCEPTION,
    PS_TIMEOUT,
    PS_OFFLINE,
//    PS_CHECKSUM_ERROR,
//	  PS_PARSE_ERROR,
//    PS_QUEUE_EMPTY,
//    PS_SOURCE_NOT_FOUND,
//    PS_UNKNOWN_LINK_NUMBER
} ps_result_enum;

typedef uint8_t ps_topic_id_t;
typedef uint16_t event_id_t;
typedef uint16_t condition_id_t;

#define PS_CONDITIONS_COUNT 64

#endif /* ps_types_h */
