//
//  ps_packet_xbee_module.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>

#include "ps_packet_xbee_module.hpp"
#include "serial/ps_serial_class.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

const char *txStatusNames[] = TX_STATUS_NAMES;
const char *modemStatusNames[] = MODEM_STATUS_NAMES;
const char *ATResponseStatusNames[] = AT_RESPONSE_NAMES;

ps_packet_xbee_module::ps_packet_xbee_module(ps_serial_class *_driver)
{
	serial_driver = _driver;
	//mutex
	INIT_MUTEX(XBeeTxMutex);
	INIT_MUTEX(XBeeMapMutex);
    
    if (MAX_TRANSPORT_PACKET > 100)
    {
        PS_ERROR("MAX_TRANSPORT_PACKET (%d) > 100", (int) MAX_TRANSPORT_PACKET);
    }
}

//register a ps_packet_xbee_class against an address in order to receive incoming packets
ps_result_enum ps_packet_xbee_module::add_link(ps_packet_xbee_class *link)
{
    PS_DEBUG("xbee: add_link %s at %2x", link->name.c_str(), link->end_address);
    
    LOCK_MUTEX(XBeeMapMutex);
    xbee_links.insert(std::make_pair(link->end_address, link));
    UNLOCK_MUTEX(XBeeMapMutex);
    return PS_OK;
}

//send a data packet to an address
ps_result_enum ps_packet_xbee_module::send_packet(const void *msg, int len, int address)
{
    PS_TRACE("xbee: send_packet to 0x%2x", address);
    
	ps_result_enum result = PS_OK;

    LOCK_MUTEX(XBeeTxMutex);

    txPacketHeader.apiIdentifier = TRANSMIT_16;
    txPacketHeader.frameId = XBeeTxSequenceNumber;
    txPacketHeader.destinationMSB = 0;
    txPacketHeader.destinationLSB = address & 0xff;
    txPacketHeader.options = 0;
    
    switch(wait_for_tx_status(&txPacketHeader, sizeof(TxPacketHeader_t), msg, len))
    {
        case XBEE_TX_SUCCESS:
            PS_TRACE("xbee: send_packet to 0x%2x - OK", address);
        	result = PS_OK;
            break;
        case XBEE_TX_NO_ACK:
            PS_TRACE("xbee: send_packet to 0x%2x - NO ACK", address);
        	result = PS_TIMEOUT;
            break;
        case XBEE_NO_STATUS:
            PS_ERROR("xbee: send_packet to 0x%2x - NO TX STATUS", address);
        	result = PS_TIMEOUT;
            break;
        default:
             PS_TRACE("xbee: send_packet to 0x%2x - IO ERROR", address);
       	result = PS_IO_ERROR;
            break;
    }
    
    UNLOCK_MUTEX(XBeeTxMutex);
    
    return result;
}

//--------------------------------------------------Transmission

//Write a byte
void ps_packet_xbee_module::Write(uint8_t dat) {
	serial_driver->write_bytes(&dat, 1);
}

//write an escaped byte
void ps_packet_xbee_module::WriteEscapedByte(uint8_t b) {
    XBeeTxChecksum += b;

    switch (b) {
        case FRAME_DELIMITER:
        case ESCAPECC:
        case XON:
        case XOFF:
            Write(ESCAPECC);
            Write(b ^ CHAR_XOR);
            break;
        default:
            Write(b);
            break;
    }
}

//Write a packet
void ps_packet_xbee_module::SendApiPacket(const void *header, int headerLen, const void *pkt, int len) {
    PS_TRACE("xbee: send_api_packet");
    
	int totalLen = len + headerLen;
	
    Write(FRAME_DELIMITER);
    WriteEscapedByte((totalLen >> 8) & 0xff);
    WriteEscapedByte(totalLen & 0xff);

    XBeeTxChecksum = 0;	//checksum starts here

    //send header
    uint8_t *next = static_cast<uint8_t*> (const_cast<void*> (header));
    int count = headerLen;

    if (count) {
        do {
            WriteEscapedByte(*next++);
            count--;
        } while (count > 0);
    }
    //send data
    next = static_cast<uint8_t*> (const_cast<void*> (pkt));
    count = len;

    if (count) {
        do {
            WriteEscapedByte(*next++);
            count--;
        } while (count > 0);
    }
    //send checksum
    WriteEscapedByte(0xff - (XBeeTxChecksum & 0xff));
}

//--------------------------------------------------------Reception

//read and un-escape
int ps_packet_xbee_module::ReadEscapedData(uint8_t *pkt, int len) {
    uint8_t *next = pkt;
    int count = len;
    uint8_t readByte;

    do {
        serial_driver->read_bytes(&readByte, 1);

        if (readByte == ESCAPECC) {
            serial_driver->read_bytes(&readByte, 1);
            *next = (readByte ^ CHAR_XOR);
        } else {
            *next = readByte;
        }

        XBeeRxChecksum += *next;

        next++;
        count--;
    } while (count > 0);

    return 0;
}

//read a packet
int ps_packet_xbee_module::ReadApiPacket(int *len) {
    int reply;
    uint8_t checksum;
    uint8_t lengthBytes[2];
    uint8_t readByte {0};
    
    do {
        //read until frame delimiter
        serial_driver->read_bytes(&readByte, 1);
    } while (readByte != FRAME_DELIMITER);

    reply = ReadEscapedData(lengthBytes, 2); //read length

    int length = lengthBytes[1];
    if (length > xbee_rx_buffer_size)
    	{
    		length = xbee_rx_buffer_size;
    		PS_ERROR("xbee: Bad packet length: %d", length);
    		return -1;
    	}

    XBeeRxChecksum = 0; //length not included

    reply += ReadEscapedData((uint8_t*) XBeeRxPacket, length);
    reply += ReadEscapedData((uint8_t*) & checksum, 1);

    *len = length;
    
    if (reply < 0 || (XBeeRxChecksum & 0xff) != 0xff) return -1;
    else return 0;
}

//---------------------------------------------------------Receive Thread
//receives messages and passes to broker via ps_packet_xbee_class object

void ps_packet_xbee_module::RxThread() {
    int length;

    PS_DEBUG("xbee: RX ready");

    for (;;) {
        if (ReadApiPacket(&length) == 0) {
//            PS_TRACE("xbee: Rx Packet length %d", length);
            switch (rxph.apiIdentifier) {
                case MODEM_STATUS:
                {
                    latestXBeeModemStatus = static_cast<XBeeModemStatus_enum>(rxph.modemStatus);
                    PS_DEBUG("xbee: Received modem status : %s", modemStatusNames[rxph.modemStatus]);
                }
                    break;
                case TRANSMIT_STATUS:
                {
                    latestTxStatus.status = static_cast<XBeeTxStatus_enum>(rxph.txStatus);
                    latestTxStatus.frameId = rxph.frameId;
                    if (latestTxStatus.status != XBEE_TX_SUCCESS)
                    {
                    	PS_DEBUG("xbee: Received TX Status: %s", txStatusNames[latestTxStatus.status]);
                    }
                    else
                    {
                    	PS_TRACE("xbee: Received TX Status: %s", txStatusNames[latestTxStatus.status]);
                    }
                    notify_tx_status(latestTxStatus.status);
                }
                    break;
                case AT_RESPONSE:
                {
                    PS_DEBUG("xbee: Received AT Response");
                    memcpy(&XBeeATResponse, &rxph, sizeof(ATResponse_t));
                    notify_at_response((XBeeATResponseStatus_enum) XBeeATResponse.status);
                }
                    break;
                case RECEIVE_16:
                {
                    int remote_address = rxph.sourceLSB;
                    PS_TRACE("xbee: Received data packet from: %2x, length %d", remote_address, length);
                    uint8_t *msg = XBeeRxPacket + sizeof(RxPacketHeader_t);
                    int msgLen = length - sizeof (RxPacketHeader_t);
                   //lookup link
                    LOCK_MUTEX(XBeeMapMutex);
                    auto pos = xbee_links.find(remote_address);
                    if (pos != xbee_links.end()) {
                        ps_packet_xbee_class *link = pos->second;
                        if (link) {
                            link->pass_new_packet(msg, msgLen);
                        }
                    }
                    else
                    {
                        PS_DEBUG("xbee: Received packet from %2x - transport unknown", remote_address);
                    }
                    UNLOCK_MUTEX(XBeeMapMutex);
                }
                    break;
                case RECEIVE_IO_16:
                    PS_DEBUG("xbee: Received IO_16 packet");
                     //tbd

                    break;
                default:
                    PS_ERROR("xbee: Received Unknown Packet");
                    //ignore
                    break;
                    }
        } else {
            PS_DEBUG("xbee: Rx Error");
            //TODO
//            ps_set_condition(XBEE_COMMS_ERRORS);
        }
    }
    return;
}

//----------------------------------------------------AT Commands
//Use prior to entering API mode and creating threads
#define MAX_REPLY 10

ps_result_enum ps_packet_xbee_module::SendATCommand(const char *cmdString, char *replyString) {
    PS_TRACE("xbee: SendATCommand: '%s'", cmdString);
    
    size_t len = strlen(cmdString);
    char *next = const_cast<char*>(cmdString);

    while (len-- > 0) {
        Write(*next++);
    }

    next = replyString;
    len = MAX_REPLY - 1;

    do {
        serial_driver->read_bytes(next, 1);
    } while (len-- > 0 && *next++ != '\r');

    *next = '\0';

    return PS_OK;
}

ps_result_enum ps_packet_xbee_module::AT_SetRegister(const char *atCommand, uint8_t value) {
    int reply;
    char cmdString[MAX_REPLY];
    char replyString[MAX_REPLY];

    PS_DEBUG("xbee: AT_SetRegister %s = %i", atCommand, value);

    sprintf(cmdString, "AT%s%i\r", atCommand, value);

    reply = SendATCommand(cmdString, replyString);

    if (reply < 0 || strncmp(replyString, "OK", 2) != 0)
    {
        return PS_IO_ERROR;
    }
    else return PS_OK;
}

int ps_packet_xbee_module::AT_GetRegister(const char *atCommand) {
    int reply;
    char cmdString[MAX_REPLY];
    char replyString[MAX_REPLY];

    PS_DEBUG("xbee: AT_GetRegister %s", atCommand);

    sprintf(cmdString, "AT%s\r", atCommand);

    reply = SendATCommand(cmdString, replyString);

    if (reply >= 0) {
        reply = -1;
        sscanf(replyString, "%i", &reply);
        return reply;
    } else return -1;
}

//-----------------------------------------------API tools
//Get/Set 1 byte registers using API

ps_result_enum ps_packet_xbee_module::SetRegister8(const char *atCommand, uint8_t value) {
    
    PS_DEBUG("xbee: SetRegister8 %s = %i", atCommand, value);

    LOCK_MUTEX(XBeeTxMutex);

    XBeeATPacket.packetHeader.apiIdentifier = AT_COMMAND;
    XBeeATPacket.packetHeader.frameId = XBeeTxSequenceNumber;
    XBeeATPacket.packetHeader.ATcommand[0] = atCommand[0];
    XBeeATPacket.packetHeader.ATcommand[1] = atCommand[1];

    XBeeATPacket.byteData = value;

    XBeeATResponseStatus_enum response = wait_for_at_response(&XBeeATPacket, sizeof(XBeeATPacket.packetHeader) + 1, nullptr, 0);
    
    UNLOCK_MUTEX(XBeeTxMutex);

    return (response == AT_RESPONSE_OK ? PS_OK : PS_IO_ERROR);
}

int ps_packet_xbee_module::GetRegister8(const char *atCommand) {
    
    PS_DEBUG("xbee: GetRegister8 %s", atCommand);

    LOCK_MUTEX(XBeeTxMutex);

    XBeeATPacket.packetHeader.apiIdentifier = AT_COMMAND;
    XBeeATPacket.packetHeader.frameId = XBeeTxSequenceNumber;
    XBeeATPacket.packetHeader.ATcommand[0] = atCommand[0];
    XBeeATPacket.packetHeader.ATcommand[1] = atCommand[1];

    XBeeATResponseStatus_enum response = wait_for_at_response(&XBeeATPacket, sizeof(XBeeATPacket.packetHeader) + 1, nullptr, 0);
    
    UNLOCK_MUTEX(XBeeTxMutex);

    if (response == AT_RESPONSE_OK)
    {
        PS_TRACE("xbee: %s = %0x", atCommand, XBeeATResponse.byteData);
        return XBeeATResponse.byteData;
    }
    else {
        PS_DEBUG("xbee: %s Response: %s", atCommand, ATResponseStatusNames[response]);
        return -1;
    }
}

int ps_packet_xbee_module::GetRegister16(const char *atCommand) {
    
    PS_DEBUG("xbee: GetRegister16 %s", atCommand);

    LOCK_MUTEX(XBeeTxMutex);

    XBeeATPacket.packetHeader.apiIdentifier = AT_COMMAND;
    XBeeATPacket.packetHeader.frameId = XBeeTxSequenceNumber;
    XBeeATPacket.packetHeader.ATcommand[0] = atCommand[0];
    XBeeATPacket.packetHeader.ATcommand[1] = atCommand[1];

    XBeeATResponseStatus_enum response = wait_for_at_response(&XBeeATPacket, sizeof(XBeeATPacket.packetHeader) + 1, nullptr, 0);
    
    UNLOCK_MUTEX(XBeeTxMutex);

    if (response == AT_RESPONSE_OK)
    {
        PS_TRACE("xbee: %s = %0x", atCommand, XBeeATResponse.intData);
        return XBeeATResponse.intData;
    }
    else {
        PS_DEBUG("xbee: %s Response: %s", atCommand, ATResponseStatusNames[response]);
        return -1;
    }
}

//tx power level

ps_result_enum ps_packet_xbee_module::SetPowerLevel(int pl) {
    return SetRegister8(POWER_LEVEL, pl);
}

int ps_packet_xbee_module::GetPowerLevel() {
    return GetRegister8(POWER_LEVEL);
}

//get into API mode
ps_result_enum ps_packet_xbee_module::EnterCommandMode() {
    char replyString[MAX_REPLY];
    SLEEP_MS(2000);
    Write('+');
    Write('+');
    Write('+');
    SLEEP_MS(2000);
    //check for OK
    int reply = SendATCommand("", replyString);

    PS_DEBUG("xbee: Enter Command Mode: %s", replyString)

    if (reply < 0 || strncmp(replyString, "OK", 2) != 0)
    {
        return PS_IO_ERROR;
    }
    else return PS_OK;
}

ps_result_enum ps_packet_xbee_module::EnterAPIMode() {
    return AT_SetRegister("AP", 2);
}
