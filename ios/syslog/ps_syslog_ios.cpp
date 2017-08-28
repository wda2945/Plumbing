/*
 * ps_syslog_IOS.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <errno.h>
#include <string.h>
#include <string>

#include "ps_config.h"
#include "ps_syslog_ios.hpp"
#include "pubsub/ps_pubsub_class.hpp"

ps_syslog_class &the_logger()
{
	static ps_syslog_ios syslog;
	return syslog;
}

ps_syslog_ios::ps_syslog_ios(syslog_message_callback_t *nsm)
{

    syslog_message_callback = nsm;
    the_broker().register_object(SYSLOG_PACKET, this);
}
ps_syslog_ios::ps_syslog_ios() : ps_syslog_ios(nullptr) {}

ps_syslog_ios::~ps_syslog_ios()
{

}

void ps_syslog_ios::message_handler(ps_packet_source_t packet_source, ps_packet_type_t  packet_type,
                     const void *msg, int length)
{
    ps_syslog_message_t *log_msg = (ps_syslog_message_t *) msg;
    
    if (packet_type == SYSLOG_PACKET)
    {
        if (syslog_message_callback) (*syslog_message_callback)(log_msg);
    }
}

void ps_syslog_ios::new_log_message(ps_syslog_message_t *msg)
{
    if (syslog_message_callback) (*syslog_message_callback)(msg);
}
