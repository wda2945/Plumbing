/* 
 * File:   ps_syslog_linux.hpp
 * Author: martin
 *
 * Message-related data initialized in ps_messages.cpp
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_LINUX_H
#define	_PS_SYSLOG_LINUX_H

#include <thread>

#include "syslog/ps_syslog.hpp"
#include "queue/ps_queue_linux.hpp"

extern char *log_folder_path;
//also used by PS_TRACE, PS_DEBUG and PS_ERROR macros
extern FILE *plumbing_debug_file;

class ps_syslog_linux : public ps_syslog_class {
	std::thread *print_thread;
	std::thread *publish_thread;
	ps_queue_linux *log_print_queue;
	ps_queue_linux *log_publish_queue;
	FILE *logfile;

public:
	ps_syslog_linux();
	virtual ~ps_syslog_linux();

	void message_handler(ps_packet_source_t packet_source,
            ps_packet_type_t   packet_type,
            const void *msg, int length) override;

	void print_log_message(ps_syslog_message_t *log_msg);

protected:

	void new_log_message(ps_syslog_message_t *msg) override;

	void print_thread_method();
	void publish_thread_method();


	DECLARE_MUTEX(printlogMtx);
};

#endif	/* _PS_SYSLOG_LINUX_H */

