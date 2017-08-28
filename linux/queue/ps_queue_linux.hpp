//
//  ps_queue_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_queue_linux_hpp
#define ps_queue_linux_hpp

#include "queue/ps_queue_class.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>

typedef uint16_t ps_q_size_t;

class ps_queue_linux : public ps_queue_class {
    
public:
    
    ps_queue_linux(int entrysize, int preload);
    ~ps_queue_linux();
    
    //append to queue
    ps_result_enum copy_message_to_q(const void *msg, int len) override;
    ps_result_enum copy_2message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2) override;
    ps_result_enum copy_3message_parts_to_q(const void *msg1, int len1, const void *msg2, int len2, const void *msg3, int len3) override;
    
    //get next message
    void *get_next_message(int msecs, int *length) override;	//waits msecs if empty, returns pointer
    
    void done_with_message(void *msg) override;	//when done with message Q entry -> freelist
    
    //queue info
    bool empty() override;
    int count() override;
    
private:
    //queue entry total size
    int queueEntrySize    = 0;
    //overhead offsets (at end)
    int nextPointerOffset = 0;
    int sizeOffset        = 0;
    
    //queue mutex
    std::mutex mtx;     //access control
    std::condition_variable cond;    //signals item in previously empty queue
    
    //queue base
    void *qHead = nullptr;
    void *qTail = nullptr;
    
    int queueCount = 0;
    
    //freelist
    void *freelist = nullptr;       //common free list
    std::mutex freeMtx;     //freelist mutex
    
    void append_queue_entry(void *e);		//appends an allocated message q entry
    
    //freelist
    void *get_free_entry();				//new broker q entry <- freelist
    void *new_queue_entry();
    void add_to_freelist(void *e);
    //helpers
    void *get_nextPointer(void *e);
    void set_nextPointer(void *e, void *n);
    int get_size(void *e);
    void set_size(void *e, int size);
};

#endif /* ps_queue_linux_hpp */
