//
//  ps_packet_xbee_structs.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_xbee_structs_hpp
#define ps_packet_xbee_structs_hpp

//regular data packet (Tx)

typedef struct {
    uint8_t apiIdentifier;
    uint8_t frameId;
    uint8_t destinationMSB;
    uint8_t destinationLSB;
    uint8_t options;
} TxPacketHeader_t;

//XBee AT command packet

typedef struct {

    struct {
        uint8_t apiIdentifier;
        uint8_t frameId;
        uint8_t ATcommand[2];
    } packetHeader;

    union {

        struct {
            uint8_t byteData;
            uint8_t fill[3];
        };
        int intData;
    };
} ATPacket_t;

//regular data packet (Rx) + status packets

typedef struct {
    uint8_t apiIdentifier;

    union {

        struct {
            uint8_t sourceMSB;
            uint8_t sourceLSB;
            uint8_t rssi;
            uint8_t options;
        };

        struct {
            uint8_t frameId;
            uint8_t txStatus;
        };

        struct {
            uint8_t modemStatus;
        };
    };
} RxPacketHeader_t;

typedef struct {
    uint8_t apiIdentifier;
    uint8_t frameId;
    uint8_t status;
} TxStatus_t;

typedef struct {
    uint8_t apiIdentifier;
    uint8_t status;
} ModemStatus_t;

//XBee AT Command Response

typedef struct {
    uint8_t apiIdentifier;
    uint8_t frameId;
    uint8_t ATcommand[2];
    uint8_t status;

    union {

        struct {
            uint8_t byteData;
            uint8_t fill[3];
        };
        uint16_t intData;
    };
} ATResponse_t; //XBee AT Command Response

//XBEE Tx Status

//TX Status

typedef enum {
	XBEE_TX_SUCCESS, XBEE_TX_NO_ACK, XBEE_TX_CCA_FAIL, XBEE_TX_PURGED, XBEE_NO_STATUS, XBEE_TX_NONE
} XBeeTxStatus_enum;

#define TX_STATUS_NAMES { "OK", "No Ack", "CCA Fail", "Tx Purged", "Status T/O", "NONE"}
extern const char *txStatusNames[];

typedef struct {
    XBeeTxStatus_enum status;
    uint8_t frameId;
} LatestTxStatus_t;

//Modem Status

typedef enum {
    XBEE_HW_RESET, XBEE_WATCHDOG_RESET, XBEE_ASSOCIATED, XBEE_DISASSOCIATED, XBEE_SYNC_LOST, XBEE_COORDINATOR_REALIGNMENT, XBEE_COORDINATOR_STARTED
} XBeeModemStatus_enum;
#define MODEM_STATUS_NAMES {"XBEE_HW_RESET", "XBEE_WATCHDOG_RESET", "XBEE_ASSOCIATED",\
 "XBEE_DISASSOCIATED", "XBEE_SYNC_LOST", "XBEE_COORDINATOR_REALIGNMENT", "XBEE_COORDINATOR_STARTED"}
extern const char *modemStatusNames[];

//AT Response
typedef enum {
    AT_RESPONSE_OK, AT_RESPONSE_ERROR, AT_RESPONSE_INVAL_CMD, AT_RESPONSE_INVAL_PARAM, AT_RESPONSE_NONE
} XBeeATResponseStatus_enum;
#define AT_RESPONSE_NAMES {"OK", "ERROR", "INVAL_CMD", "INVAL_PARAM", "NO RESPONSE"}
extern const char *ATResponseStatusNames[];

//XBee protocol defines
//API Mode
//escaped characters
#define FRAME_DELIMITER 0x7e
#define XON				0x11
#define XOFF			0x12
#define ESCAPECC		0x7d
#define CHAR_XOR		0x20

//API Identifiers
#define MODEM_STATUS		0x8A
#define AT_COMMAND			0x08
#define QUEUE_PARAM_VALUE	0x09
#define AT_RESPONSE			0x88
#define AT_COMMAND_REQUEST	0x17
#define REMOTE_CMD_RESPONSE	0x97
#define TRANSMIT_64			0x00
#define TRANSMIT_16			0x01
#define TRANSMIT_STATUS		0x89
#define RECEIVE_64			0x80
#define RECEIVE_16			0x81
#define RECEIVE_IO_64		0x82
#define RECEIVE_IO_16		0x83

//AT Commands
#define POWER_LEVEL			"PL"
#define SLEEP_MODE 			"SM"
#define SLEEP_OPTIONS		"SO"
#define TIME_TO_SLEEP		"ST"
#define CYCLIC_SLEEP_PERIOD	"SP"
#define API_ENABLE			"AP"
#define FIRMWARE_VERSION	"VR"
#define HARDWARE_VERSION	"HV"
#define SIGNAL_STRENGTH		"DB"
#define CCA_FAILURES		"EC"
#define ACK_FAILURES		"EA"

#endif /* ps_packet_xbee_structs_hpp */
