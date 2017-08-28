    //
//  ps_queue_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_queue_rtos.hpp"
#include <string.h>

//what's copied onto the FreeRTOS queue
typedef struct 
{
    void *entry;    //pointer to block of pre-assigned memory
    int size;       //length of current contents
} queue_entry_t;

ps_queue_rtos::ps_queue_rtos(int entrysize, int preload){
 
    queueEntrySize = entrysize;
    
    queue = xQueueCreate(preload, sizeof(queue_entry_t));
    freeList = xQueueCreate(preload, sizeof(void *));       //don't need size field
    
    if (!queue || !freeList) {
        PS_ERROR("queue: xQueueCreate failed");
    }
    
    //pre-allocate message buffers into freelist
    int i;
    for (i=0; i<preload; i++)
    {
        void *buff = pvPortMalloc(queueEntrySize);
        if (buff)
        {
            //pointer to message buffer saved in freelist
            xQueueSend(freeList, &buff, portMAX_DELAY);
            
            PS_TRACE("queue: block 0x%lX assigned", (uint32_t) buff)
        }
        else
        {
            PS_ERROR("queue: ps_queue preload failed @ %d!", i);
            return;
        }
    }
}

ps_queue_rtos::~ps_queue_rtos()
{
}

//append to queue

ps_result_enum ps_queue_rtos::copy_3message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2, const void *msg3, int len3) {
    queue_entry_t new_entry;    //new queue entry
    
    int totalSize = len1 + len2 + len3;
    
    if (totalSize > queueEntrySize) 
    {
        PS_ERROR("queue: message too long (%d > %d)", totalSize, queueEntrySize);
        return PS_LENGTH_ERROR;
    }
    
    //get the address of a freelist block
    void *e;
    if (xQueueReceive(freeList, &e, 0) == pdTRUE) {

        PS_TRACE("queue: block 0x%lX queued", (uint32_t) e)

        //copy data into q block
        uint8_t *f = (uint8_t*) e;
        new_entry.entry = e;
        new_entry.size = 0;
        
        if (msg1 && len1) {
            memcpy(f, msg1, len1);
            f += len1;
            new_entry.size += len1;
        }
        if (msg2 && len2) {
            memcpy(f, msg2, len2);
            f += len2;
            new_entry.size += len2;
        }
        if (msg3 && len3) {
            memcpy(f, msg3, len3);
            new_entry.size += len3;
        }

        xQueueSend(queue, &new_entry, portMAX_DELAY);
        return PS_OK;
    }
    else
    {
        PS_ERROR("queue: freelist empty");
        return PS_QUEUE_FULL;
    }
}
    
ps_result_enum ps_queue_rtos::copy_2message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2)
{
	return copy_3message_parts_to_q(msg1, len1, msg2, len2, nullptr, 0);
}
ps_result_enum ps_queue_rtos::copy_message_to_q(const void *msg, int len)
{
    return copy_3message_parts_to_q(msg, len, nullptr, 0, nullptr, 0);
}

//get_next_message
//returns pointer & length (must call done_with_message!)
//msecs < 0 waits for ever
//msecs == 0 does not wait
//msecs > 0 waits x millisecs
void *ps_queue_rtos::get_next_message(int msecs, int *length)
{
//    PS_TRACE("queue: get_next_message");
    TickType_t wait = (msecs >= 0 ? msecs : portMAX_DELAY);
    
    queue_entry_t qentry;
    
    if (xQueueReceive(queue, &qentry, wait) == pdTRUE)
    {
        PS_TRACE("queue: block 0x%lX unqueued", (uint32_t) qentry.entry);
        if (length) *length = qentry.size;
        return qentry.entry;
    }
    else return nullptr;
}

bool ps_queue_rtos::empty()
{
    return ((uxQueueMessagesWaiting(queue) == 0));
}
int ps_queue_rtos::count()
{
    return uxQueueMessagesWaiting(queue);
}
void ps_queue_rtos::done_with_message(void *msg)
{
    if (msg) {
        xQueueSend(freeList, &msg, 0);
        PS_TRACE("queue: block 0x%lX freed", (uint32_t) msg)
    }
}

