//
//  ps_queue_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_queue_rtos_hpp
#define ps_queue_rtos_hpp

#include "FreeRTOS.h"
#include "queue.h"
#include "queue/ps_queue_class.hpp"

class ps_queue_rtos : public ps_queue_class {
    
public:
    
    ps_queue_rtos(int entrysize, int preload);
    ~ps_queue_rtos();
    
    //append to queue
    ps_result_enum copy_message_to_q(const void *msg, int len) override;
    ps_result_enum copy_2message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2) override;
    ps_result_enum copy_3message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2, const void *msg3, int len3) override;
//    void append_queue_entry(void *e);
    
    //get next message
    void *get_next_message(int msecs, int *length) override;	//waits msecs if empty, returns pointer
    
    void done_with_message(void *msg) override;	//when done with message Q entry -> freelist
    
    //queue info
    bool empty() override;
    int count() override;
    
private:
    //queue entry size
    int queueEntrySize;
    
    QueueHandle_t queue;
    QueueHandle_t freeList;
};

#endif /* ps_queue_rtos_hpp */
