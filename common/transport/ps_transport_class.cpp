//
//  ps_transport_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright � 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_transport_class.hpp"
#include "pubsub/ps_pubsub_class.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

ps_transport_class::ps_transport_class(const char *name, ps_packet_class *driver)
    : ps_root_class(name)
{
    PS_TRACE("tran: %s: new", name);

    packet_driver = driver;

    packet_driver->add_data_observer(this);
    packet_driver->add_event_observer(this);
    
    int i;
    for (i=0; i<SRC_COUNT; i++)
    {
        source_filter[i] = false;
    }
}

ps_transport_class::ps_transport_class(ps_packet_class *driver)
: ps_transport_class(driver->name.c_str(), driver){}


ps_transport_class::~ps_transport_class() {
    delete packet_driver;
}

void ps_transport_class::transport_send_thread_method() {
    
    PS_TRACE("tran: %s: transport_send_thread_method", name.c_str());

    while (1) {
        //initially offline
        
        if (!transport_is_online)
        {
            //empty send queue
            void *pkt;
            do {
                pkt = sendQueue->get_next_message(0, nullptr);
                if (pkt) sendQueue->done_with_message(pkt);
            } while (pkt);
            
            outboundFlags = PS_TRANSPORT_MSG;
        }
        
        while (!transport_is_online) {
            
            SLEEP_MS(PS_TRANSPORT_MESSAGE_WAIT_MSECS);
            
            if (--keepalive_count <= 0 || status_packet_required)
            {
                //send a status packet as a poll
                
                statusPkt.packet_header.sequenceNumber = lastSent;
                statusPkt.packet_header.lastReceivedSequenceNumber = lastReceived;
                statusPkt.packet_header.status = PS_STATUS_ONLY | PS_PING;
                
                //send status poll
                PS_TRACE("tran: %s: offline - send PING", name.c_str());
                
                ps_transport_status_enum packet_status = send_and_confirm(&statusPkt, sizeof (statusPkt));
                
                PS_TRACE("tran: %s: PING status: %s", name.c_str(), transport_status_names[packet_status & 0xf]);
                
                status_packets_sent++;
                
                switch (packet_status & 0x0f) {
                    case PS_TRANSPORT_TIMEOUT:			//timeout
                        break;
                        
                    case PS_TRANSPORT_ERROR:
                        break;
                        
                    default:
                        //reply received -> online
                        report_transport_event(PS_TRANSPORT_ONLINE);
                        
                        keepalive_retry_count = PS_TRANSPORT_RETRIES;
                        
                        outboundFlags = PS_TRANSPORT_IGNORE_SEQ;
                        
                        break;
                }
                keepalive_count = KEEPALIVE_COUNT_INITIALIZER;
            }
        }

        while (transport_is_online) {
            //wait for a message to send
   
            int length;
            
            transport_packet_t *packet = (transport_packet_t *) sendQueue->get_next_message(PS_TRANSPORT_MESSAGE_WAIT_MSECS, &length);

            if (packet) {
                //new packet to send
                
                int retryCount = PS_TRANSPORT_RETRIES;

                if (++lastSent == 0) lastSent++; //packet sequence number

                packet->packet_header.lastReceivedSequenceNumber = lastReceived;
                packet->packet_header.sequenceNumber = lastSent;
                packet->packet_header.status = outboundFlags;

                do {
                    PS_TRACE("tran: %s: send #%i. lastTx = %i, lastRx = %i, flags = %s", name.c_str(),packet->packet_header.sequenceNumber,
                             lastSent, lastReceived, transport_status_names[(int) (outboundFlags & 0xf)]);

                    ps_transport_status_enum packet_status = send_and_confirm(packet, length);

                    PS_TRACE("tran: %s: packet status: %s", name.c_str(), transport_status_names[packet_status & 0xf]);

                    switch (packet_status & 0x0f) {
                        case PS_TRANSPORT_TIMEOUT:
                            timeouts++;
                        case PS_TRANSPORT_ERROR:
                        case PS_TRANSPORT_NAK:
                        case PS_TRANSPORT_MSG:
                            packet->packet_header.status |= PS_TRANSPORT_RETX;
                            retryCount--;
                            data_packets_resent++;
                            break;

                        case PS_TRANSPORT_ACK:
                        case PS_TRANSPORT_DUP:
                        case PS_TRANSPORT_SEQ_ERROR:
                            keepalive_retry_count = PS_TRANSPORT_RETRIES;
                            data_packets_sent++;
                            data_bytes_sent += length;
                            retryCount = -1;
                            break;
                    }

                    if (retryCount == 0) {
                        PS_DEBUG("tran: %s: tx failed", name.c_str());
                        report_transport_event(PS_TRANSPORT_OFFLINE);
                    }

                } while (retryCount > 0);

                sendQueue->done_with_message(packet);

                status_packet_required = false;

            } else {
                //queue read timeout - maybe send status packet
                if (status_packet_required) {
                	send_status_only();
                }
                else if (--keepalive_count <= 0)
                {
                    //send keepalive
                    statusPkt.packet_header.lastReceivedSequenceNumber = lastReceived;
                    statusPkt.packet_header.sequenceNumber = lastSent;
                    statusPkt.packet_header.status = PS_STATUS_ONLY | PS_PING;

                    PS_TRACE("tran: %s: send Keepalive packet", name.c_str());

                    //send ping and wait
                    ps_transport_status_enum packet_status = send_and_confirm(&statusPkt, sizeof (statusPkt));

                    PS_TRACE("tran: %s: Keepalive status: %s", name.c_str(), transport_status_names[packet_status & 0xf]);

                    status_packets_sent++;
                    
                    switch (packet_status & 0xff) {
                        case PS_TRANSPORT_TIMEOUT:
                            timeouts++;
                        case PS_TRANSPORT_ERROR:

                            if (--keepalive_retry_count <= 0) {
                                PS_DEBUG("tran: Keepalive failed");
                                report_transport_event(PS_TRANSPORT_OFFLINE);
                            }
                            break;
                        default:
                            keepalive_retry_count = PS_TRANSPORT_RETRIES;
                            break;
                    }
                    keepalive_count = KEEPALIVE_COUNT_INITIALIZER;
               }
            }
        }
    }
}

//send a status packet
ps_result_enum ps_transport_class::send_status_only()
{
    //unacknowledged message
    statusPkt.packet_header.lastReceivedSequenceNumber = lastReceived;
    statusPkt.packet_header.sequenceNumber = lastSent;
    statusPkt.packet_header.status = outboundFlags | PS_STATUS_ONLY;

    PS_TRACE("tran: %s: send status. lastTx = %i, lastRx = %i, flags = %s", name.c_str(),
             lastSent, lastReceived, transport_status_names[(int) (outboundFlags & 0xf)]);

    //send status packet - no wait

    ps_result_enum result = packet_driver->send_packet(&statusPkt, sizeof (statusPkt));

    if (result != PS_OK)
    {
    	return result;
    }

    status_packets_sent++;
    
    status_packet_required = false;
    
    return PS_OK;
}


//send packet

ps_result_enum ps_transport_class::send_packet2(const void *packet1, int len1, const void *packet2, int len2) {
    PS_TRACE("tran: %s: send_packet2, length %i", name.c_str(), len1+len2);

    if (!transport_is_online) {
        PS_TRACE("tran: %s offline", name.c_str());
        return PS_OFFLINE;
    }

    ps_transport_header_t packetHeader; //dummy for later use
    if (sendQueue->copy_3message_parts_to_q(&packetHeader, sizeof (ps_transport_header_t), packet1, len1, packet2, len2) != PS_OK)
    {
        PS_ERROR("tran: %s Q copy failed", name.c_str());
        return PS_QUEUE_FULL;
    }
    
    return PS_OK;
}

ps_result_enum ps_transport_class::send_packet(const void *packet, int length) {
    return send_packet2(packet, length, nullptr, 0);
}

//process packet received

void ps_transport_class::process_observed_data(ps_root_class *src, const void *_pkt, int len) {
    PS_TRACE("tran: %s: process_observed_data, length %d", name.c_str(), len);

    typedef struct {
        ps_transport_header_t header;
        uint8_t payload[];
    } transport_packet_t;
    
    transport_packet_t *packet = (transport_packet_t *) _pkt;

    //first look at the header
    if (len >= (int) sizeof (ps_transport_header_t)) {

        //check reception of last sent packet
        remoteLastReceived = packet->header.lastReceivedSequenceNumber;
        remoteLastFlags = packet->header.status;

        PS_TRACE("tran: %s: RX: lastTx = %i, remLastRx = %i, remFlags = %s", name.c_str(),
                 lastSent, remoteLastReceived, transport_status_names[(int) (remoteLastFlags & 0xf)]);

        if (remoteLastReceived == lastSent) {
            //this relates to the last one I sent 
            //report tx message result
            new_packet_status((ps_transport_status_enum) remoteLastFlags);
        }
        else
        {
            //report a message received
            new_packet_status(PS_TRANSPORT_MSG);
        }

        status_packets_received++;
        
        //reply to PING
        if (remoteLastFlags & PS_PING) status_packet_required = true;
    }

    //now look for data
    if (!(remoteLastFlags & PS_STATUS_ONLY) && (len > (int) sizeof (ps_transport_header_t))) {
        //data packet
        //check for duplicate
        if ((lastReceived == packet->header.sequenceNumber) && ((remoteLastFlags & PS_TRANSPORT_IGNORE_SEQ) == 0)) {
            //duplicate message
        	if ((remoteLastFlags & PS_TRANSPORT_RETX) == 0)
        	{
        		//not a simple re-transmission
        		PS_DEBUG("tran: %s: Rx duplicate packet %i", name.c_str(), lastReceived);
        		outboundFlags = PS_TRANSPORT_DUP;
                duplicates++;
        	}
        	else
        	{
        		PS_TRACE("tran: %s: Rx re-transmit packet %i", name.c_str(), lastReceived);
        		outboundFlags = PS_TRANSPORT_ACK;
        	}
        } else {
            //new message
        	PS_TRACE("tran: %s: new packet %i received", name.c_str(), packet->header.sequenceNumber);
            //calc expected sequence number
            uint8_t nextSequence = (lastReceived + 1) & 0xff;
            if (nextSequence == 0) nextSequence = 1;

            if ((nextSequence == packet->header.sequenceNumber)
                    || (lastReceived == 0)
                    || (packet->header.sequenceNumber == 0)
                    || (remoteLastFlags & PS_TRANSPORT_IGNORE_SEQ)) {
                //sequence number good
                outboundFlags = PS_TRANSPORT_ACK;
            } else {
                //flag bad sequence number
                PS_DEBUG("tran: %s bad sequence %i, expected %i", name.c_str(),packet->header.sequenceNumber, nextSequence);
                outboundFlags = PS_TRANSPORT_SEQ_ERROR;
                sequence_errors++;
            }
            lastReceived = packet->header.sequenceNumber;
            int length = len - sizeof (ps_transport_header_t);
            pass_new_data(this, packet->payload, length);
            data_packets_received++;
            data_bytes_received += len;
        }
        status_packet_required = true;
    }

    if (len < (int) sizeof (ps_transport_header_t)) {
        PS_ERROR("tran: %s: bad packet length: %i", name.c_str(), len);
        outboundFlags = PS_TRANSPORT_NAK;
        status_packet_required = true;
        other_rx_errors++;
    }

}

void ps_transport_class::process_observed_event(ps_root_class *src, int _event) {
    PS_TRACE("tran: %s: process_observed_event", name.c_str());

    ps_packet_event_t ev = static_cast<ps_packet_event_t> (_event);

    switch (ev) {
        case PS_PACKET_OFFLINE:
            break;
        case PS_PACKET_ONLINE:
            break;
        case PS_PACKET_REMOVED:
            report_transport_event(PS_TRANSPORT_REMOVED);
            break;
        case PS_PACKET_ERROR:
            new_packet_status(PS_TRANSPORT_NAK);
            break;
        default:
            break;
    }
}

void ps_transport_class::report_transport_event(ps_transport_event_enum ev) {
    if (transport_status != ev) {
        PS_DEBUG("tran: %s: %s", name.c_str(), transport_event_names[ev]);
        transport_status = ev;

        switch (transport_status) {
            case PS_TRANSPORT_ONLINE:
                transport_is_online = true;
                notify_new_event(this, ev);
                break;

            case PS_TRANSPORT_OFFLINE:
            case PS_TRANSPORT_REMOVED:
                disconnects++;
                transport_is_online = false;
                notify_new_event(this, ev);
                break;

            default:
                break;
        }
    }
}

const char *transport_event_names[] = TRANSPORT_EVENT_NAMES;
const char *transport_status_names[] = TRANSPORT_STATUS_NAMES;
