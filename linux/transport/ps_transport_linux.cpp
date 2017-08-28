//
//  ps_transport_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <chrono>
#include <mutex>
#include <signal.h>

#include "ps_config.h"
#include "ps_common.h"
#include "ps_transport_linux.hpp"
#include "network/ps_network.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

using namespace std::chrono;
using namespace std;

ps_transport_linux::ps_transport_linux(const char *name, ps_packet_class *_driver)
: ps_transport_class(name, _driver)
{
    sendQueue = new ps_queue_linux(MAX_PS_PACKET, 0);
    
    send_thread = new thread([this](){transport_send_thread_method();});
}

ps_transport_linux::ps_transport_linux(ps_packet_class *_driver)
: ps_transport_linux(_driver->name.c_str(), _driver) {}

ps_transport_linux::~ps_transport_linux()
{
    delete send_thread;
    delete sendQueue;
}

ps_transport_status_enum ps_transport_linux::send_and_confirm(transport_packet_t *pkt, int len){

	PS_TRACE("tran: %s: send_and_confirm", name.c_str());
    
    unique_lock<mutex> lck { remoteResponseMutex };

    ps_result_enum result = packet_driver->send_packet(pkt, len);

    if (result != PS_OK)
    {
    	return PS_TRANSPORT_ERROR;
    }
    
    uint8_t outflags = pkt->packet_header.status;
    
    int packet_count = (PS_TRANSPORT_STATUS_WAIT_MSECS / PS_TRANSPORT_MESSAGE_WAIT_MSECS);
    
    std::chrono::milliseconds condWait(PS_TRANSPORT_MESSAGE_WAIT_MSECS);

    do {

        if (remoteResponseCond.wait_for(lck, condWait) == std::cv_status::no_timeout)
        {
            //a message came
            if ((outflags & PS_PING) != 0) {
                PS_TRACE("tran: %s: PING returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);
                return newRxStatus;			//ping - any reply is good
            }
            if (newRxStatus != PS_TRANSPORT_MSG) {
                PS_TRACE("tran: %s: returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);
                return newRxStatus;			//relevant reply
            }
        }
        else newRxStatus = PS_TRANSPORT_TIMEOUT;

        if (status_packet_required) send_status_only();

    } while (packet_count-- > 0); //re-wait if timeout or non-relevant packet received
    
    PS_TRACE("tran: %s: timeout. returning: %s", name.c_str(), transport_status_names[(int) (newRxStatus & 0xf)]);
    return newRxStatus;			//relevant reply
}

void ps_transport_linux::new_packet_status(ps_transport_status_enum status){

	PS_TRACE("tran: %s: new_packet_status: %s", name.c_str(), transport_status_names[(int) (status & 0xf)]);

	//critical section
//	unique_lock<mutex> lck { remoteResponseMutex };

	newRxStatus = status;

	//signal send thread
	remoteResponseCond.notify_one();
}
