/*
 * ps_syslog_linux.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include <errno.h>
#include <string.h>
#include <string>
#include <signal.h>

#include "ps_syslog_linux.hpp"
#include "pubsub/ps_pubsub_class.hpp"

FILE *plumbing_debug_file;

ps_syslog_class &the_logger()
{
	static ps_syslog_linux syslog;
	return syslog;
}

ps_syslog_linux::ps_syslog_linux()
{
	plumbing_debug_file = fopen_logfile("plumbing");
	logfile = fopen_logfile("syslog");

	//queue for messages
	log_print_queue = new ps_queue_linux(sizeof(ps_syslog_message_t), SYSLOG_QUEUE_LENGTH);
	log_publish_queue = new ps_queue_linux(sizeof(ps_syslog_message_t), SYSLOG_QUEUE_LENGTH);

	//start log threads
	print_thread = new std::thread([this](){print_thread_method();});
	publish_thread = new std::thread([this](){publish_thread_method();});

	the_broker().register_object(SYSLOG_PACKET, this);
}

ps_syslog_linux::~ps_syslog_linux()
{
    delete print_thread;
    delete publish_thread;
    delete log_print_queue;
    delete log_publish_queue;
}

//add locally-generated new messages to both queues
void ps_syslog_linux::new_log_message(ps_syslog_message_t *msg)
{
    if (log_print_queue->copy_message_to_q(msg, sizeof (ps_syslog_message_t)) < 0) {
        PS_ERROR("log: print copy_message_to_q failed");
    }
    if (log_publish_queue->copy_message_to_q(msg, sizeof (ps_syslog_message_t)) < 0) {
        PS_ERROR("log: publish copy_message_to_q failed");
    }
}

//print to file log messages from the print queue
void ps_syslog_linux::print_thread_method()
{
	{
		ps_syslog_message_t msg;
		msg.severity = SYSLOG_ROUTINE;
		strncpy(msg.source, "sys", PS_SOURCE_LENGTH);
		strncpy(msg.text, "log started", PS_MAX_LOG_TEXT);
		print_log_message(&msg);
	}

	while(1)
	{
		int len;
		ps_syslog_message_t *log_msg = (ps_syslog_message_t *) log_print_queue->get_next_message(-1, &len);
		print_log_message(log_msg);
		log_print_queue->done_with_message(log_msg);
	}
}

//send upstream log messages from the publish queue
void ps_syslog_linux::publish_thread_method()
{
	while(1)
	{
		int len;
		ps_syslog_message_t *log_msg = (ps_syslog_message_t *) log_publish_queue->get_next_message(-1, &len);

		the_broker().transmit_packet(SYSLOG_PACKET, log_msg, len);

		log_publish_queue->done_with_message(log_msg);
	}
}

void ps_syslog_linux::message_handler(ps_packet_source_t packet_source,
        ps_packet_type_t   packet_type,
        const void *msg, int length)
{
	//inbound log messages from elsewhere >> print queue
	if (packet_type == SYSLOG_PACKET)	//sanity check
	{
		log_print_queue->copy_message_to_q(msg, length);
	}
	else
	{
		PS_ERROR("log: bad message_handler call: packet %i, length %i", packet_type, length);
	}
}

void ps_syslog_linux::print_log_message(ps_syslog_message_t *log_msg)
{
	const int MAX_MESSAGE = (PS_SOURCE_LENGTH + PS_MAX_LOG_TEXT + 20);
	std::string severity;

	char printBuff[MAX_MESSAGE];

	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	switch (log_msg->severity) {
	case SYSLOG_ROUTINE:
	default:
		severity = "R";
		break;
	case SYSLOG_INFO:
		severity = "I";
		break;
	case SYSLOG_WARNING:
		severity = "W";
		break;
	case SYSLOG_ERROR:
		severity = "E";
		break;
	case SYSLOG_FAILURE:
		severity = "F";
		break;
	}
	//make sure we don't overflow
	log_msg->text[PS_MAX_LOG_TEXT] = '\0';
	log_msg->source[PS_SOURCE_LENGTH] = '\0';

	//remove newline
	int len = (int) strlen(log_msg->text);
	if (log_msg->text[len-1] == '\n') log_msg->text[len-1] = '\0';


	snprintf(printBuff, MAX_MESSAGE, "%02i:%02i:%02i %s@%s: %s",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec,
			severity.c_str(), log_msg->source, log_msg->text);

	LOCK_MUTEX(printlogMtx);
	fprintf(logfile, "%s\n", printBuff);
	fflush(logfile);
	UNLOCK_MUTEX(printlogMtx);

	PS_DEBUG( "%s", printBuff);
}

