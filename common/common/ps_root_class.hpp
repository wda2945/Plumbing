//
//  ps_root_class.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_root_class_hpp
#define ps_root_class_hpp

#include "ps_config.h"
#include "ps_common.h"
#include "ps.h"

#include <string>
#include <vector>

//root class used by all platform nodes

class ps_root_class {

public:
    ps_root_class();
    ps_root_class(char *_name);
    ps_root_class(std::string _name);

    virtual ~ps_root_class() {}

    std::string name = "";
    
    void set_node_name(const char * _name){name = _name;}
    void set_node_name(std::string _name){name = _name;}

    //OVERRIDDEN METHODS

    //method to receive published messages in subclasses
    //following subscription via register_object()
    virtual void message_handler(ps_packet_source_t packet_source,
                                 ps_packet_type_t   packet_type,
                                 const void *msg, int length) = 0;

    //OBSERVER PATTERN

    //add observers for data and events
    //callbacks to process_observed_data() and process_observed_event()
    virtual void add_data_observer(ps_root_class *ob);
    virtual void add_event_observer(ps_root_class *eo);
    
    //observer callbacks
    virtual void process_observed_data(ps_root_class *src, const void *msg, int length) = 0;
    virtual void process_observed_event(ps_root_class *src, int event) = 0;
    
protected:
    
    //INVOKE THE OBSERVER PATTERN
 
    //action calls to observers with supplied source (typically 'this')
    virtual void pass_new_data(ps_root_class *src, const void *msg, int length);
    
    virtual void notify_new_event(ps_root_class *src, int event);
    
    //observer lists
    std::vector<ps_root_class*> data_observers;
    std::vector<ps_root_class*> event_observers;

    DECLARE_MUTEX(dataObserverMtx);
    DECLARE_MUTEX(eventObserverMtx);
};

#endif /* ps_root_class_hpp */
