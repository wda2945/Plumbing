//
//  ps_packet_serial_linux.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "ps_packet_serial_linux.hpp"

ps_packet_serial_linux::ps_packet_serial_linux(const char *name, ps_serial_class *_driver):
    ps_packet_serial_class(name, _driver)
{
    packet_thread = new std::thread([this](){packet_serial_linux_thread_method();});
}

ps_packet_serial_linux::~ps_packet_serial_linux()
{
    delete packet_thread;
}

void ps_packet_serial_linux::packet_serial_linux_thread_method()
{
	const struct sigaction sa {{SIG_IGN}, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

    packet_serial_rx_thread_method();
}


