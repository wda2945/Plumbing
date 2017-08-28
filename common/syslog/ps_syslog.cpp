/*
 * ps_syslog.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <string.h>
#include "ps.h"
#include "ps_common.h"
#include "ps_syslog.hpp"
#include "pubsub/ps_pubsub_class.hpp"
#include "network/ps_network.hpp"
#include "transport/ps_transport_class.hpp"

void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file)
{
    PS_TRACE("_LogMessage: %s", _message);
    
	const char *filecomponent = (strrchr(_file, '/') + 1);
	if (!filecomponent) filecomponent = _file;
	if (!filecomponent) filecomponent = "****";

	ps_syslog_message_t msg;

	msg.severity = _severity;
	strncpy(msg.source, filecomponent, PS_SOURCE_LENGTH);
	strncpy(msg.text, _message, PS_MAX_LOG_TEXT);

	//publish a copy
	the_logger().new_log_message(&msg);
}

void ps_syslog_class::debug_transport_stats()
{
    the_network().iterate_transports(this);
}

void ps_syslog_class::process_observed_event(ps_root_class *src, int event) {
    if (event == PS_TRANSPORT_ADDED) {
        ps_transport_class *transport = (ps_transport_class *) src;

        //comms stats - transmit
        PS_DEBUG("%s: data_packets_sent = %i", src->name.c_str(), transport->data_packets_sent);
        PS_DEBUG("%s: data_bytes_sent = %i", src->name.c_str(), transport->data_bytes_sent);
        if (transport->data_packets_resent) {
            PS_DEBUG("%s: data_packets_resent = %i", src->name.c_str(), transport->data_packets_resent);
        }
        PS_DEBUG("%s: status_packets_sent = %i", src->name.c_str(), transport->status_packets_sent);
        if (transport->timeouts) {
            PS_DEBUG("%s: timeouts = %i", src->name.c_str(), transport->timeouts);
        }
        if (transport->disconnects) {
            PS_DEBUG("%s: disconnects = %i", src->name.c_str(), transport->disconnects);
        }
        //comms stats - receive
        PS_DEBUG("%s: data_packets_received = %i", src->name.c_str(), transport->data_packets_received);
        PS_DEBUG("%s: data_bytes_received = %i", src->name.c_str(), transport->data_bytes_received);
        PS_DEBUG("%s: status_packets_received = %i", src->name.c_str(), transport->status_packets_received);
        if (transport->duplicates) {
            PS_DEBUG("%s: duplicates = %i", src->name.c_str(), transport->duplicates);
        }
        if (transport->sequence_errors) {
            PS_DEBUG("%s: sequence_errors = %i", src->name.c_str(), transport->sequence_errors);
        }
        if (transport->other_rx_errors) {
            PS_DEBUG("%s: other_rx_errors = %i", src->name.c_str(), transport->other_rx_errors);
        }
    }
}

void ps_debug_transport_stats() {
    the_logger().debug_transport_stats();
}

void ps_debug_system_stats() {
    the_logger().debug_system_stats();
}