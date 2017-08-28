//
//  ps_notify.h
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_h
#define ps_notify_h

#include "ps_types.h"

//notifications client api

#ifdef __cplusplus
extern "C" {
#endif
    
    ////////////////////////// EVENTS
    
    typedef uint8_t ps_event_id_t;
    
    //Raise an event
    ps_result_enum ps_notify_event(ps_event_id_t event);
    
    typedef void (ps_event_observer_callback_t)(void *arg, ps_event_id_t event);
    
    ps_result_enum ps_add_event_observer(ps_event_id_t event, ps_event_observer_callback_t *callback, void *arg);
    
    ps_result_enum ps_register_event_names(const char **names, int count);

    ////////////////////////// CONDITIONS

    typedef uint8_t ps_condition_id_t;
    
    //Set/cancel a condition
    ps_result_enum ps_set_condition(ps_condition_id_t condition);
    ps_result_enum ps_cancel_condition(ps_condition_id_t condition);
    
    bool ps_test_condition(Source_t src, ps_condition_id_t condition);
    
    typedef void (ps_condition_observer_callback_t)(void *arg, Source_t src, ps_condition_id_t condition, bool condition_set);
    
    ps_result_enum ps_add_condition_observer(Source_t src, ps_condition_id_t condition, ps_condition_observer_callback_t *callback, void *arg);
    
    ps_result_enum ps_register_condition_names(const char **names, int count);

#ifdef __cplusplus
}
#endif

#endif /* ps_notify_h */
