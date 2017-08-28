//
//  ps_queue.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_queue_hpp
#define ps_queue_hpp

#include "ps_config.h"
#include "ps_api/ps_types.h"

class ps_queue_class {

public:
	ps_queue_class(){    
        PS_TRACE("queue: new");
    }
	virtual ~ps_queue_class(){}

    //append a message to the queue
    virtual ps_result_enum copy_message_to_q(const void *msg, int len) = 0;
    //helpers to minimise copying
    virtual ps_result_enum copy_2message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2) = 0;
    virtual ps_result_enum copy_3message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2, const void *msg3, int len3) = 0;

    //get next message
    //returns pointer (call done_with_message!)
    //msecs < 0 waits for ever
    //msecs == 0 does not wait
    //msecs > 0 waits x millisecs
    virtual void *get_next_message(int msecs, int *length) = 0;
    
    virtual void done_with_message(void *msg) = 0;	//when done with message Q entry -> freelist
    
    //queue info
    virtual bool empty() = 0;
    virtual int count() = 0;
};

#endif /* ps_queue_hpp */
