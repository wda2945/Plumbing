//
//  ps_packet_xbee_module.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_xbee_module_hpp
#define ps_packet_xbee_module_hpp

#include <map>

#include "ps.h"

#include "ps_packet_xbee_class.hpp"
#include "serial/ps_serial_class.hpp"
#include "queue/ps_queue_class.hpp"

#include "ps_packet_xbee_structs.hpp"

#define XBEE_BUFFER_SIZE	200

class ps_packet_xbee_module {
public:
	ps_packet_xbee_module(ps_serial_class *_driver);
    
	~ps_packet_xbee_module() {}

	ps_result_enum add_link(ps_packet_xbee_class *link);
	ps_result_enum send_packet(const void *msg, int len, int address);

	ps_result_enum EnterCommandMode();
	ps_result_enum EnterAPIMode();

	ps_result_enum SetRegister8(const char *atCommand, uint8_t value);
	int GetRegister8(const char *atCommand);
    int GetRegister16(const char *atCommand);

	//tx power level
	ps_result_enum SetPowerLevel(int pl);
	int GetPowerLevel();

	//TODO: Registers
	ps_result_enum SetRegister16(const char *atCommand, uint16_t value);
	ps_result_enum RemoteReset();
	ps_result_enum RemoteSetRegister8(const char *atCommand, uint8_t value);
	ps_result_enum RemoteSetRegister16(const char *atCommand, uint16_t value);
	int RemoteGetRegister8(const char *atCommand);

	//TODO: API commands
	ps_result_enum LocalCommand(const char *atCommand, uint8_t *param, int len,
			uint8_t *buf, int buflen);
	ps_result_enum RemoteCommand(const char *atCommand, uint8_t *param, int len,
			uint8_t *buf, int buflen);

protected:

	ps_serial_class *serial_driver;

	std::map<int, ps_packet_xbee_class*> xbee_links;
	DECLARE_MUTEX(XBeeMapMutex);

	DECLARE_MUTEX(XBeeTxMutex);

	virtual XBeeTxStatus_enum wait_for_tx_status(const void *header, int headerLen, const void *pkt, int len)= 0;
	virtual void notify_tx_status(XBeeTxStatus_enum status) = 0;

	virtual XBeeATResponseStatus_enum wait_for_at_response(const void *header, int headerLen, const void *pkt, int len) = 0;
	virtual void notify_at_response(XBeeATResponseStatus_enum status) = 0;

	XBeeModemStatus_enum latestXBeeModemStatus = XBEE_HW_RESET;

	ATPacket_t 		 XBeeATPacket; 			//XBee AT command packet
    TxPacketHeader_t txPacketHeader;		//regular packet header

	int xbee_rx_buffer_size {XBEE_BUFFER_SIZE};

	union {
		RxPacketHeader_t rxph;
		uint8_t XBeeRxPacket [XBEE_BUFFER_SIZE]; 	//regular data packet (Rx) (header + data)
	};

	ATResponse_t XBeeATResponse { 0 }; //XBee AT Command Response

	LatestTxStatus_t latestTxStatus;

	int XBeeTxChecksum {0};
	int XBeeRxChecksum {0};
	uint8_t XBeeTxSequenceNumber {1};
	uint8_t XBeeRxSequenceNumber {0};

	void Write(uint8_t dat);
	void WriteEscapedByte(uint8_t b);
	void SendApiPacket(const void *header, int headerLen, const void *pkt, int len);
	void SendMessage(const void *msg);

	int ReadEscapedData(uint8_t *pkt, int len);
	int ReadApiPacket(int *len);

	//receives messages and passes to transport layer
	void RxThread();

	//AT Mode
	//Use prior to entering API mode and creating threads
	ps_result_enum SendATCommand(const char *cmdString, char *replyString);

	//Get/Set registers using AT command
	ps_result_enum AT_SetRegister(const char *atCommand, uint8_t value);
	int AT_GetRegister(const char *atCommand);

};

#endif /* ps_packet_xbee_class_hpp */
