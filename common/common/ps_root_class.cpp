//
//  ps_root_class.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_root_class.hpp"

ps_root_class::ps_root_class(){
    INIT_MUTEX(dataObserverMtx);
    INIT_MUTEX(eventObserverMtx);
}
ps_root_class::ps_root_class(char *_name)
{
    name = std::string(_name);
    INIT_MUTEX(dataObserverMtx);
    INIT_MUTEX(eventObserverMtx);
}
ps_root_class::ps_root_class(std::string _name)
{
    name = _name;
    INIT_MUTEX(dataObserverMtx);
    INIT_MUTEX(eventObserverMtx);
}


void ps_root_class::add_data_observer(ps_root_class *ob)
{
    PS_DEBUG("add_data_observer: %s observing %s", ob->name.c_str(), name.c_str());
    LOCK_MUTEX(dataObserverMtx);
    data_observers.push_back(ob);
    UNLOCK_MUTEX(dataObserverMtx);
}
void ps_root_class::add_event_observer(ps_root_class *eo)
{
    PS_DEBUG("add_event_observer: %s observing %s", eo->name.c_str(), name.c_str());
    LOCK_MUTEX(eventObserverMtx);
    event_observers.push_back(eo);
    UNLOCK_MUTEX(eventObserverMtx);
}

void ps_root_class::pass_new_data(ps_root_class *src, const void *msg, int length)
{
//    PS_TRACE("pass_new_data: from %s to %s", src->name.c_str(), name.c_str());
    LOCK_MUTEX(dataObserverMtx);

    std::vector<ps_root_class*>::iterator dataIterator = data_observers.begin();

    while (dataIterator != data_observers.end())
    {
    	ps_root_class *obs = *dataIterator;

        UNLOCK_MUTEX(dataObserverMtx);

        PS_TRACE("pass_new_data: from %s to %s", src->name.c_str(), obs->name.c_str());

        obs->process_observed_data(src, msg, length);

        LOCK_MUTEX(dataObserverMtx);
        
        dataIterator++;
    }
    UNLOCK_MUTEX(dataObserverMtx);
}

void ps_root_class::notify_new_event(ps_root_class *src, int event)
{
    PS_TRACE("notify_new_event %d: at %s", event, name.c_str());

    LOCK_MUTEX(eventObserverMtx);

    std::vector<ps_root_class*>::iterator obsIterator = event_observers.begin();

    while (obsIterator != event_observers.end())
    {
        ps_root_class *obs = *obsIterator;

        UNLOCK_MUTEX(eventObserverMtx);

        PS_TRACE("notify_new_event %d: from %s to %s", event, src->name.c_str(), obs->name.c_str());

        obs->process_observed_event(src, event);

        LOCK_MUTEX(eventObserverMtx);

        obsIterator++;
    }

    UNLOCK_MUTEX(eventObserverMtx);
}
