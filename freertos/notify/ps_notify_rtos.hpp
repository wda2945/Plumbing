//
//  ps_notify_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_rtos_hpp
#define ps_notify_rtos_hpp

#include "FreeRTOS.h"
#include "task.h"

#include "notify/ps_notify.hpp"

class ps_notify_rtos : public ps_notify_class {
public:
    ps_notify_rtos();
    ~ps_notify_rtos();
    
protected:
    TaskHandle_t notify_task;
    
    friend void notify_thread_wrapper(void *parameters);
};

#endif /* ps_notify_rtos_hpp */
