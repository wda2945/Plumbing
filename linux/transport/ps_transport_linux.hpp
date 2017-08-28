//
//  ps_transport_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_transport_linux_hpp
#define ps_transport_linux_hpp

#include "transport/ps_transport_class.hpp"
#include "queue/ps_queue_linux.hpp"
#include <thread>
#include <mutex>

class ps_transport_linux : public ps_transport_class
{

public:
    //init a transport with the provided packet driver
	ps_transport_linux(ps_packet_class *driver);
    ps_transport_linux(const char *name, ps_packet_class *driver);
    ~ps_transport_linux();
    
protected:

    std::thread              *send_thread;

    ps_transport_status_enum newRxStatus {PS_TRANSPORT_TIMEOUT};

    ps_transport_status_enum send_and_confirm(transport_packet_t *pkt, int len);
    void new_packet_status(ps_transport_status_enum status);

    std::condition_variable remoteResponseCond;
    std::mutex remoteResponseMutex;

};

#endif /* ps_transport_linux_hpp */
