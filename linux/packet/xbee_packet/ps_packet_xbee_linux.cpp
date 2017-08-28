//
//  ps_packet_xbee_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include "ps_packet_xbee_linux.hpp"

//#undef PS_TRACE
//#define PS_TRACE(...) PS_DEBUG(__VA_ARGS__)

using namespace std::chrono;
using namespace std;

ps_packet_xbee_linux::ps_packet_xbee_linux(ps_serial_class *_driver) :
		ps_packet_xbee_module(_driver) {

	//start thread for xbee receive
	xbee_thread = new thread([this]() {RxThread();});
}

XBeeTxStatus_enum ps_packet_xbee_linux::wait_for_tx_status(const void *header, int headerLen, const void *pkt, int len) {
	unique_lock<mutex> lck { remoteResponseMutex };

	tx_status = XBEE_TX_NONE;

	SendApiPacket(header, headerLen, pkt, len);

	PS_TRACE("xbee: wait_for_tx_status");

	std::chrono::milliseconds condWait(XBEE_TX_STATUS_WAIT_MS);

	//wait for a received status packet
	do {
		if (remoteResponseCond.wait_for(lck, condWait) == std::cv_status::timeout)
		{
			PS_ERROR("xbee: Tx Status Timeout");
			return XBEE_NO_STATUS;
		}
	} while (tx_status == XBEE_TX_NONE);

	return tx_status;
}

//called by the rx thread when a tx status is received
void ps_packet_xbee_linux::notify_tx_status(XBeeTxStatus_enum status) {
	PS_TRACE("xbee: notify_tx_status: %s", txStatusNames[(int) status]);

	//critical section
	unique_lock<mutex> lck { remoteResponseMutex };

	tx_status = status;

	//signal send thread
	remoteResponseCond.notify_one();
}

XBeeATResponseStatus_enum ps_packet_xbee_linux::wait_for_at_response(const void *header, int headerLen, const void *pkt, int len) {
	PS_TRACE("xbee: wait_for_at_response");

	unique_lock<mutex> lck { atResponseMutex };
	at_status = AT_RESPONSE_NONE;

	SendApiPacket(header, headerLen, pkt,len);

	std::chrono::milliseconds condWait(XBEE_AT_RESPONSE_WAIT_MS);

	//wait for a received packet
	do {
		if (atResponseCond.wait_for(lck, milliseconds(XBEE_AT_RESPONSE_WAIT_MS)) == std::cv_status::timeout)
		{
			PS_ERROR("xbee: No AT Respose");
			return AT_RESPONSE_NONE;
		}
	} while (at_status == AT_RESPONSE_NONE);

	return at_status;
}

void ps_packet_xbee_linux::notify_at_response(XBeeATResponseStatus_enum status) {
	PS_TRACE("xbee: notify_at_response: %s", ATResponseStatusNames[status]);

	//critical section
	unique_lock<mutex> lck { atResponseMutex };

	at_status = status;

	//signal send thread
	atResponseCond.notify_one();
}
