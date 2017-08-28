//
//  ps_notify.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_notify_hpp
#define ps_notify_hpp

#include <vector>
#include <map>
#include <string>
#include <bitset>

#include "ps_api/ps_notify.h"
#include "common/ps_root_class.hpp"

//struct to record an event observer
typedef struct {
    ps_event_observer_callback_t *callback;
    void *callbackArg;
} ps_event_observer;

//struct to record a condition observer
typedef struct {
    ps_condition_observer_callback_t *callback;
    void *callbackArg;
} ps_condition_observer;

typedef uint64_t ps_conditions_message_t; //long long bitmap used to communicate conditions

class ps_notify_class : public ps_root_class {
public:
	char **event_names {nullptr};
	int event_count {0};
	char **condition_names {nullptr};
	int condition_count {0};

    ////////////////////////// EVENTS
    //methods called from C api
    //Raise an event
    ps_result_enum ps_notify_event_method(ps_event_id_t event);
    
    ps_result_enum ps_add_event_observer_method(ps_event_id_t event,
                                                ps_event_observer_callback_t *callback, void *arg);
    
    ////////////////////////// CONDITIONS
    //methods called from C api
    //Set/cancel a condition
    ps_result_enum ps_set_condition_method(ps_condition_id_t condition);

    ps_result_enum ps_cancel_condition_method(ps_condition_id_t condition);
    
    bool ps_test_condition_method(Source_t src, ps_condition_id_t condition);
    
    ps_result_enum ps_add_condition_observer_method(Source_t src, ps_condition_id_t condition, ps_condition_observer_callback_t *callback, void *arg);
    
    ps_result_enum ps_republish_conditions();

protected:
    ps_notify_class();
    ~ps_notify_class();

    //conditions
    std::bitset<PS_CONDITIONS_COUNT> conditions[SRC_COUNT];
    std::bitset<PS_CONDITIONS_COUNT> current {0};	//local conditions for publishing - last reported
    
    //observers
    std::map<ps_event_id_t, std::vector<ps_event_observer>> event_observers;
    std::map<ps_condition_id_t, std::vector<ps_condition_observer>> condition_observers[SRC_COUNT];
    
    //notify observers
    void notify_event_observer(ps_event_id_t event);
    void notify_condition_observer(Source_t src, ps_condition_id_t condition, bool state);
    
    //process received event & conditions packets
    void message_handler(ps_packet_source_t src, ps_packet_type_t   packet_type, const void *msg, int length) override;

    friend ps_notify_class& the_notifier();

    DECLARE_MUTEX(eventMtx);
    DECLARE_MUTEX(conditionMtx);

public:
	//thread
	void notify_conditions_thread();

    //observer callbacks - not used
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
    void process_observed_event(ps_root_class *src, int event) override {}

};

ps_notify_class& the_notifier();


#endif /* ps_notify_hpp */
