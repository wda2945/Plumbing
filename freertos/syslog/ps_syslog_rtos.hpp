/* 
 * File:   ps_syslog_rtos.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_RTOS_H
#define	_PS_SYSLOG_RTOS_H

#include "xc.h"
#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING

#include <plib.h>

using namespace std;

#include "syslog/ps_syslog.hpp"
#include "queue/ps_queue_rtos.hpp"

extern char tick_text[PS_TICK_TEXT];

int rtos_debug_init(UART_MODULE _log_uart, int _log_uart_baudrate);

class ps_syslog_rtos : public ps_syslog_class {

public:
	ps_syslog_rtos(UART_MODULE debug_uart, int debug_baudrate);
	ps_syslog_rtos();
	~ps_syslog_rtos();

	void message_handler(ps_packet_source_t packet_source,
            ps_packet_type_t   packet_type,
            const void *msg, int length) override;

    void print_log_message(ps_syslog_message_t *log_msg);
        
protected:
    void new_log_message(ps_syslog_message_t *msg);

    void debug_system_stats() override;
    
//	ps_queue_rtos *log_print_queue;
	ps_queue_rtos *log_publish_queue;
//	void print_thread_method();
	void publish_thread_method();
//    friend void print_thread_wrapper(void *pvParameters);
    friend void publish_thread_wrapper(void *pvParameters);
};

#endif	/* _PS_SYSLOG_RTOS_H */

