//
//  ps_notify.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

//#include <unistd.h>
#include <string.h>
#include "notify/ps_notify.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_notify_class::ps_notify_class() : ps_root_class(std::string("notify")){
    PS_TRACE("notify: new");
    
    the_broker().register_object(CONDITIONS_PACKET, this);
    the_broker().register_object(EVENT_PACKET, this);
    
    INIT_MUTEX(eventMtx);
    INIT_MUTEX(conditionMtx);
}

ps_notify_class::~ps_notify_class(){
    
}

/////////////////////// C API Methods

////////////////////////// EVENTS

//Raise an event
ps_result_enum ps_notify_class::ps_notify_event_method(ps_event_id_t event)
{
	if (event >= event_count)
	{
		PS_DEBUG("not: notify event #%i", event);
	}
	else
	{
		PS_DEBUG("not: notify event %s", event_names[event]);
	}

    the_broker().publish_packet(EVENT_PACKET, &event, (int) sizeof(ps_event_id_t));
    
    //notify observers
    notify_event_observer(event);
    
    return PS_OK;
}

ps_result_enum ps_notify_class::ps_add_event_observer_method(ps_event_id_t event,
                                                                 ps_event_observer_callback_t *callback, void *arg)
{
    PS_TRACE("notify: ps_add_event_observer_method");

    ps_event_observer obs {callback, arg};
    
    LOCK_MUTEX(eventMtx);

    auto pos = event_observers.find(event);
    
    if (pos == event_observers.end() )
    {
        //first observer
    	std::vector<ps_event_observer> new_vec;
        new_vec.push_back(obs);
        event_observers.insert(make_pair(event, new_vec));
    }
    else
    {
        pos->second.push_back(obs);
    }
    
    UNLOCK_MUTEX(eventMtx);

    return PS_OK;
}


////////////////////////// CONDITIONS


//Set/cancel a condition
ps_result_enum ps_notify_class::ps_set_condition_method(ps_condition_id_t condition)
{
	if (condition == 0) return PS_OK;

    if (condition < PS_CONDITIONS_COUNT)
    {
    	if (!conditions[SOURCE].test(condition))
    	{
    		if (condition >= condition_count)
    		{
    			PS_DEBUG("not: set condition #%i", condition);
    		}
    		else
    		{
    			PS_DEBUG("not: set condition %s", condition_names[condition]);
    		}

    		LOCK_MUTEX(conditionMtx);

    		conditions[SOURCE].set(condition);

    		UNLOCK_MUTEX(conditionMtx);

    		//notify observers
    		notify_condition_observer(SOURCE, condition, true);
    	}
        return PS_OK;
    }
    else
    {
    	PS_ERROR("not: invalid set condition #%i", condition);
        return PS_INVALID_PARAMETER;
    }
}

ps_result_enum ps_notify_class::ps_cancel_condition_method(ps_condition_id_t condition)
{
	if (condition == 0) return PS_OK;

	if (condition < PS_CONDITIONS_COUNT)
	{
		if (conditions[SOURCE].test(condition))
		{
			if (condition >= condition_count)
			{
				PS_DEBUG("not: cancel condition #%i", condition);
			}
			else
			{
				PS_DEBUG("not: cancel condition %s", condition_names[condition]);
			}

			LOCK_MUTEX(conditionMtx);

			conditions[SOURCE].reset(condition);

			UNLOCK_MUTEX(conditionMtx);

			//notify observers
			notify_condition_observer(SOURCE, condition, false);
		}
		return PS_OK;
    }
    else
    {
        PS_ERROR("not: invalid cancel condition #%i", condition);
        return PS_INVALID_PARAMETER;
    }
}

bool ps_notify_class::ps_test_condition_method(Source_t src, ps_condition_id_t condition)
{
    PS_TRACE("notify: ps_test_condition_method");

	if (condition == 0) return false;

    if (condition < PS_CONDITIONS_COUNT)
    {
        LOCK_MUTEX(conditionMtx);
        
        bool result = conditions[src].test(condition);
        
        UNLOCK_MUTEX(conditionMtx);
        
        return result;
    }
    else
    {
        PS_ERROR("not: invalid test condition #%i", condition);
        return false;
    }
}

ps_result_enum ps_notify_class::ps_add_condition_observer_method(Source_t src, ps_condition_id_t condition,
                                                                 ps_condition_observer_callback_t *callback, void *arg)
{
    PS_TRACE("notify: ps_add_condition_observer_method");

	if (condition == 0) return PS_INVALID_PARAMETER;

    if (condition < PS_CONDITIONS_COUNT)
    {
        ps_condition_observer obs {callback, arg};
        
        LOCK_MUTEX(conditionMtx);
        
        auto pos = condition_observers[src].find(condition);
        
        if (pos == condition_observers[src].end() )
        {
            //first observer
        	std::vector<ps_condition_observer> new_vec;
            new_vec.push_back(obs);
            condition_observers[src].insert(make_pair(condition, new_vec));
        }
        else
        {
            pos->second.push_back(obs);
        }
        UNLOCK_MUTEX(conditionMtx);
        
        return PS_OK;
    }
    else
    {
    	PS_ERROR("not: invalid add condition observer #%i", condition);
        return PS_INVALID_PARAMETER;
    }
}

ps_result_enum ps_notify_class::ps_republish_conditions()
{
    PS_TRACE("notify: ps_republish_conditions");

	LOCK_MUTEX(conditionMtx);
	current.reset();
	UNLOCK_MUTEX(conditionMtx);
	return PS_OK;
}

//helpers

void ps_notify_class::notify_event_observer(ps_event_id_t event)
{
   PS_TRACE("notify: notify_event_observer");

   LOCK_MUTEX(eventMtx);
   std::vector<ps_event_observer> _observers;
    auto pos = event_observers.find(event);
    
    if (pos != event_observers.end() )
    {
        _observers = pos->second;
    }
    UNLOCK_MUTEX(eventMtx);
    
    for (auto obs : _observers)
    {
        (obs.callback)(obs.callbackArg, event);
    }
}

void ps_notify_class::notify_condition_observer(Source_t src, ps_condition_id_t condition, bool state)
{
   PS_TRACE("notify: notify_condition_observer");

   LOCK_MUTEX(conditionMtx);
   std::vector<ps_condition_observer> _observers;
    
    auto pos = condition_observers[src].find(condition);
    
    if (pos != condition_observers[src].end() )
    {
        _observers = pos->second;

    }
    
    UNLOCK_MUTEX(conditionMtx);
    
    for (auto obs : _observers)
    {
        (obs.callback)(obs.callbackArg, src, condition, state);
    }
}

void ps_notify_class::notify_conditions_thread()
{
   PS_DEBUG("notify: notify_conditions_thread");

    while(1)
    {
        LOCK_MUTEX(conditionMtx);

        if (current != conditions[SOURCE])
        {
            ps_conditions_message_t msg = conditions[SOURCE].to_ullong();
            current = conditions[SOURCE];

            UNLOCK_MUTEX(conditionMtx);

            the_broker().publish_packet(CONDITIONS_PACKET, &msg, (int) sizeof(ps_conditions_message_t));

        }
        else
        {
        	UNLOCK_MUTEX(conditionMtx);
        }
        SLEEP_MS(250);
    }
}

void ps_notify_class::message_handler(ps_packet_source_t src,
                     ps_packet_type_t   packet_type,
                     const void *msg, int length)
{
   PS_TRACE("notify: message_handler");

   if (src == SOURCE) return;

    switch(packet_type)
    {
        case EVENT_PACKET:
            if (length >= (int) sizeof(ps_event_id_t))
        {
            const ps_event_id_t *event = (ps_event_id_t*) msg;
            notify_event_observer(*event);
        }
            break;
        case CONDITIONS_PACKET:
            if (length >= (int) sizeof(ps_conditions_message_t))
        {
            ps_conditions_message_t c_msg;
            memcpy(&c_msg, msg, sizeof(ps_conditions_message_t));

            std::bitset<PS_CONDITIONS_COUNT> msgbits(c_msg);

            LOCK_MUTEX(conditionMtx);

            std::bitset<PS_CONDITIONS_COUNT> changes = conditions[src] ^ msgbits;

            conditions[src] = msgbits;
            
            UNLOCK_MUTEX(conditionMtx);
            
        	for (int i=0; i < PS_CONDITIONS_COUNT; i++)
        	{
        		if (changes[i])
        		{
        			notify_condition_observer( (Source_t) src, i, msgbits[i]);
        		}
        	}
        }
            break;
        default:
            break;
    }
}


/////////////////////// C API

//Raise an event
ps_result_enum ps_notify_event(ps_event_id_t event)
{
    return the_notifier().ps_notify_event_method(event);
}

ps_result_enum ps_add_event_observer(ps_event_id_t event, ps_event_observer_callback_t *callback, void *arg)
{
    return  the_notifier().ps_add_event_observer_method(event, callback, arg);
}

ps_result_enum ps_register_event_names(const char **names, int count)
{
	the_notifier().event_names = const_cast<char**>(names);
	the_notifier().event_count = count;
	return PS_OK;
}

//Set/cancel a condition
ps_result_enum ps_set_condition(ps_condition_id_t condition)
{
    return the_notifier().ps_set_condition_method(condition);
}
ps_result_enum ps_cancel_condition(ps_condition_id_t condition)
{
    return the_notifier().ps_cancel_condition_method(condition);
}

bool ps_test_condition(Source_t src, ps_condition_id_t condition)
{
    return the_notifier().ps_test_condition_method(src, condition);
}

ps_result_enum ps_add_condition_observer(Source_t src, ps_condition_id_t condition, ps_condition_observer_callback_t *callback, void *arg)
{
    return the_notifier().ps_add_condition_observer_method(src, condition, callback, arg);
}

ps_result_enum ps_register_condition_names(const char **names, int count)
{
	the_notifier().condition_names = const_cast<char**>(names);
	the_notifier().condition_count = count;
	return PS_OK;
}
