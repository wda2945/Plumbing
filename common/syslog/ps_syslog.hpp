/* 
 * File:   ps_syslog.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_H
#define	_PS_SYSLOG_H


#include "common/ps_root_class.hpp"
#include "ps_syslog_message.h"

class ps_syslog_class : public ps_root_class {
public:
    ps_syslog_class()  : ps_root_class(std::string("syslog")){}
    ~ps_syslog_class() {}
    
    //log messages sent from broker !!
    virtual void message_handler(ps_packet_source_t packet_source,
                                 ps_packet_type_t   packet_type,
                                 const void *msg, int length) override = 0;
protected:
    
    //output stats to PS_DEBUG())
    virtual void debug_transport_stats();
    virtual void debug_system_stats() {}
    
    friend void ps_debug_transport_stats();
    friend void ps_debug_system_stats();
    
    virtual void new_log_message(ps_syslog_message_t *msg) = 0;

    friend void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file);
private:
    //observer data callback - not used
    virtual void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
    //observer event callback - used for transport enumeration
    virtual void process_observed_event(ps_root_class *src, int event) override;
};

ps_syslog_class &the_logger();

#endif	/* _PS_SYSLOG_H */

