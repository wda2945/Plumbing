/* 
 * File:   ps_syslog_ios.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_IOS_H
#define	_PS_SYSLOG_IOS_H

#include "syslog/ps_syslog.hpp"

typedef void (syslog_message_callback_t)(ps_syslog_message_t *);

class ps_syslog_ios : public ps_syslog_class {

public:
    ps_syslog_ios(syslog_message_callback_t *nsm);
    ps_syslog_ios();
	~ps_syslog_ios();

    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override;
    
    syslog_message_callback_t *syslog_message_callback;
    
    void new_log_message(ps_syslog_message_t *msg) override;
    
    //observer callbacks - not used
    void process_observed_data(ps_root_class *src, const void *msg, int length) override {}
    void process_observed_event(ps_root_class *src, int event) override {}

};

#endif	/* _PS_SYSLOG_IOS_H */

