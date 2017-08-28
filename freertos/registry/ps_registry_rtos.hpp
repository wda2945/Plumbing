//
//  ps_registry_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/21/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_rtos_hpp
#define ps_registry_rtos_hpp

#include "registry/ps_registry.hpp"

class ps_registry_rtos : public ps_registry {
    
public:
    
    ps_registry_rtos();
    ~ps_registry_rtos();

private:

    friend void registry_thread_wrapper(void *pvParameters);
};

#endif /* ps_queue_rtos_hpp */
