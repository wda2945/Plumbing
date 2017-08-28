    //
//  ps_queue_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_queue_linux.hpp"
#include <string.h>
#include <sys/time.h>

#include <chrono>
using namespace std::chrono;

ps_queue_linux::ps_queue_linux(int entrysize, int preload){
    //round length to multiple of 4 bytes
    int rounding = (entrysize % 4);
    nextPointerOffset = entrysize + (rounding > 0 ? 4 - rounding : 0);
    sizeOffset = nextPointerOffset + sizeof(void *);
    queueEntrySize = sizeOffset + sizeof(int);
    
    int i;
    for (i=0; i<preload; i++)
    {
        void *entry = new_queue_entry();
        if (entry == nullptr) return;
        add_to_freelist(entry);
    }
}

ps_queue_linux::~ps_queue_linux()
{
	while(qHead)
	{
		void *e = get_next_message(0, nullptr);
		free(e);
	}

	while(freelist)
	{
		void *e = get_free_entry();
		free(e);
	}
}

void ps_queue_linux::append_queue_entry(void *e)		//appends an allocated message q entry
{
    set_nextPointer(e, nullptr);
    bool wake = false;
    
    {
        //critical section
    	std:: unique_lock<std::mutex> lck {mtx};
        
        if (qHead == nullptr)
        {
            //Q empty
            qHead = qTail = e;
            wake = true;
        }
        else
        {
            set_nextPointer(qTail, e);
            qTail = e;
        }
        queueCount++;
    }
    //end critical section
    
    if (wake) cond.notify_one();
}

//appends to queue
ps_result_enum ps_queue_linux::copy_3message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2, const void *msg3, int len3)
{
	if (len1 + len2 + len3 <= nextPointerOffset)
	{
		void *e = get_free_entry();

		if (e != nullptr)
		{
			int size = 0;

        //copy data into q block
        uint8_t *f = (uint8_t*) e;
        
        if (msg1 && len1) {
            memcpy(f, msg1, len1);
            f += len1;
            size += len1;
        }
        if (msg2 && len2) {
            memcpy(f, msg2, len2);
            f += len2;
            size += len2;
        }
        if (msg3 && len3) {
            memcpy(f, msg3, len3);
            size += len3;
        }

			set_size(e, size);
			append_queue_entry(e);
		}
		return PS_OK;
	}
	else
	{
		PS_ERROR("queue: message too long");
		return PS_LENGTH_ERROR;
	}
}
ps_result_enum ps_queue_linux::copy_2message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2)
{
	return copy_3message_parts_to_q(msg1, len1, msg2, len2, nullptr, 0);
}
ps_result_enum ps_queue_linux::copy_message_to_q(const void *msg, int len)
{
    return copy_3message_parts_to_q(msg, len, nullptr, 0, nullptr, 0);
}

//returns pointer (call done_with_message!)
//msecs < 0 waits for ever
//msecs == 0 does not wait
//msecs > 0 waits x millisecs
void *ps_queue_linux::get_next_message(int msecs, int *length)
{
    //critical section
	std::unique_lock<std::mutex> lck {mtx};
    
    if ((queueCount == 0) && (msecs == 0))
    {
        return nullptr;
    }
    
    std::cv_status result {std::cv_status::no_timeout};
    //empty wait case
    
    while (queueCount == 0 && result == std::cv_status::no_timeout)
    {
    	if (msecs > 0)
    	{
            auto now = system_clock::now();
            result = cond.wait_until(lck, now + milliseconds(msecs));
    	}
    	else
    	{
    		cond.wait(lck);;
    	}
    }
    
    void *e = qHead;
    
    if (e)
    {
        qHead = get_nextPointer(e);
        set_nextPointer(e, nullptr);
        
        if (qHead == nullptr)
        {
            //end of queue
            qTail = nullptr;
        }
        
        queueCount--;
        
        if (length != nullptr)
        {
            *length = get_size(e);
        }
    }
    
    return e;
}

bool ps_queue_linux::empty()
{
    return ((queueCount == 0));
}
int ps_queue_linux::count()
{
    return queueCount;
}
void ps_queue_linux::done_with_message(void *msg)
{
    if (msg) add_to_freelist(msg);
}

////////////////Freelist

void *ps_queue_linux::get_free_entry()				//new broker q entry <- freelist
{
	void *e;
    
    //critical section
    freeMtx.lock();
    
    if (freelist == nullptr)
    {
        e = new_queue_entry();
    }
    else
    {
        e = freelist;
        freelist = get_nextPointer(e);
    }
    
    freeMtx.unlock();
    //end critical section
    
    return e;
}

void *ps_queue_linux::new_queue_entry()
{
    return malloc(queueEntrySize);
}

void ps_queue_linux::add_to_freelist(void *e)
{
    //critical section
    freeMtx.lock();

    set_nextPointer(e, freelist);
    freelist = e;
    
    freeMtx.unlock();
    //end critical section
}

///////////////// Helpers

void *ps_queue_linux::get_nextPointer(void *e)
{
    uint8_t *eb = (uint8_t*) e;
    void **next = (void**)(eb + nextPointerOffset);
    return *next;
}
void ps_queue_linux::set_nextPointer(void *e, void *n)
{
    uint8_t *eb = (uint8_t*) e;
    void **next = (void**)(eb + nextPointerOffset);
    *next = n;
}
int ps_queue_linux::get_size(void *e)
{
    uint8_t *eb = (uint8_t*) e;
    int *size = (int*)(eb + sizeOffset);
    return *size;
}
void ps_queue_linux::set_size(void *e, int _size)
{
    uint8_t *eb = (uint8_t*) e;
    int *size = (int*)(eb + sizeOffset);
    *size = _size;
}
