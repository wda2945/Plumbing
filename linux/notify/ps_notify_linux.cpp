//
//  ps_notify.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/22/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//


#include <thread>
#include "notify/ps_notify_linux.hpp"

ps_notify_class& the_notifier()
{
    static ps_notify_linux the_notifier;
    return the_notifier;
}

ps_notify_linux::ps_notify_linux()
{
    conditionsThread  = new std::thread([this](){notify_conditions_thread();});
}

ps_notify_linux::~ps_notify_linux()
{
    delete conditionsThread;
}

