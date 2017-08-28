//
//  ps_registry_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_registry_linux_hpp
#define ps_registry_linux_hpp

#include <thread>
#include "registry/ps_registry.hpp"

class ps_registry_linux : public ps_registry {
  
public:

    ps_result_enum load_registry_method(const char *path);
    ps_result_enum save_registry_method(const char *path, const char *domain);
        
protected:

    ps_registry_linux();
    virtual ~ps_registry_linux(){}

    std::thread *registry_thread;

    friend ps_registry& the_registry();
};

#endif /* ps_registry_linux_hpp */
