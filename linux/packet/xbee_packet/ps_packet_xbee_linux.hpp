//
//  ps_packet_xbee_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_packet_xbee_linux_hpp
#define ps_packet_xbee_linux_hpp

#include <thread>
#include <condition_variable>

#include "packet/xbee_packet/ps_packet_xbee_module.hpp"

//this class just handles synchronization between tx and rx threads

class ps_packet_xbee_linux : public ps_packet_xbee_module {
public:
	ps_packet_xbee_linux(ps_serial_class *_driver);
	virtual ~ps_packet_xbee_linux() {}

protected:

	std::thread *xbee_thread;	//xbee receive thread

	//calls and mutex to wait for xbee data packet response
    XBeeTxStatus_enum wait_for_tx_status(const void *header, int headerLen, const void *pkt, int len);			//send & wait for a tx status
    void notify_tx_status(XBeeTxStatus_enum status);//tx status received
    XBeeTxStatus_enum tx_status {XBEE_TX_SUCCESS};
    std::condition_variable remoteResponseCond;
    std::mutex remoteResponseMutex;

    //calls and mutex to wait for reply to at command
	XBeeATResponseStatus_enum wait_for_at_response(const void *header, int headerLen, const void *pkt, int len);						//wait for an at response
    void notify_at_response(XBeeATResponseStatus_enum status);			//at response received
    std::condition_variable atResponseCond;
    std::mutex atResponseMutex;
	XBeeATResponseStatus_enum at_status {AT_RESPONSE_NONE};


};

#endif /* ps_packet_xbee_linux_hpp */
