//
//  ps_packet_serial_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_packet_serial_class.hpp"


ps_packet_serial_class::ps_packet_serial_class(const char *name, ps_serial_class *_driver) : ps_packet_class(name)
{
	serial_driver = _driver;
	serial_driver->add_event_observer(this);
}
ps_packet_serial_class::~ps_packet_serial_class()
{
	delete serial_driver;
	free(rxmsg);
}

void ps_packet_serial_class::packet_serial_rx_thread_method()
{
    while (1)
    {
        int reply {0};
        unsigned char nextByte = 0;
        
//        PS_TRACE("pkt: waiting for STX");

        do {
            reply = serial_driver->read_bytes(&nextByte, 1);

            if (reply <= 0) {
                SLEEP_MS(100);
            }
        }        while (nextByte != STX_CHAR);

        ps_packet_header_t packetHeader;

        int charsRead = serial_driver->read_bytes(&packetHeader, sizeof (ps_packet_header_t));

        if (charsRead == sizeof (ps_packet_header_t)) {
            uint8_t length = packetHeader.length1;
            uint8_t length2 = packetHeader.length2;
            if (((length + length2) == 0xff) && (length <= MAX_SERIAL_PACKET)) {

                charsRead = serial_driver->read_bytes(&rxmsg, length);

                if (charsRead == length) {
                    if (packetHeader.csum == calculate_checksum(rxmsg, length)) {
                        //                            PS_DEBUG("pkt: packet received");
                        pass_new_data(this, rxmsg, length);
                    } else {
                        PS_ERROR("pkt:%s bad checksum: 0x%x read versus 0x%x", name.c_str(),
                                packetHeader.csum, calculate_checksum(rxmsg, length));
                    }
                } else {
                    PS_ERROR("pkt:%s short read: %i versus %i", name.c_str(), charsRead, length);
                }

            } else {
                PS_ERROR("pkt:%s bad message length: %i & %i", name.c_str(), length, length2);
            }
        } else {
            PS_ERROR("pkt:%s bad header read %i bytes", name.c_str(), charsRead);
        }

    }
}
//send packet
static uint8_t stx_char = STX_CHAR;

ps_result_enum ps_packet_serial_class::send_packet(const void *packet, int length)
{
    ps_packet_header_t 		packetHeader;

    PS_TRACE("pkt: %s send_packet, length = %i", name.c_str(), (int)( length + sizeof(packetHeader)));
    
    packetHeader.length1 = length & 0xff;
    packetHeader.length2 = ~(length & 0xff);

    packetHeader.csum = calculate_checksum(static_cast<const uint8_t*>(packet), length);

    ps_result_enum res = serial_driver->write_bytes(&stx_char, 1);
    if (res != PS_OK) return res;
    res = serial_driver->write_bytes(&packetHeader, sizeof(ps_packet_header_t));
    if (res != PS_OK) return res;
    
    res = serial_driver->write_bytes(packet, length);
    return res;
}

//serial driver event
void ps_packet_serial_class::process_observed_event(ps_root_class *src, int _event)
{
    ps_serial_status_enum stat = static_cast<ps_serial_status_enum>(_event);
    
	switch(stat)
	{
	case PS_SERIAL_OFFLINE:
        PS_TRACE("pkt: Serial offline");
		notify_new_event(this, PS_PACKET_OFFLINE);
		break;
	case PS_SERIAL_WRITE_ERROR:
	case PS_SERIAL_READ_ERROR:
        PS_TRACE("pkt: Serial error");
		notify_new_event(this, PS_PACKET_ERROR);
		break;
	case PS_SERIAL_ONLINE:
        PS_TRACE("pkt: Serial online");
		notify_new_event(this, PS_PACKET_ONLINE);
		break;
	}
}

uint16_t ps_packet_serial_class::calculate_checksum(const uint8_t *packet, int length)
{
    int i;
    int checksum = 0;
    
    for (i=0; i<length; i++)
    {
        int byte = *packet;
        checksum += byte;
        packet++;
    }
    return (checksum & 0xffff);
}

