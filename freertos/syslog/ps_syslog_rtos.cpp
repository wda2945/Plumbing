/*
 * ps_syslog_rtos.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */
#include <string.h>
#include <string>

#include "ps_common.h"
#include "ps_config.h"
#include "ps_syslog_rtos.hpp"
#include "pubsub/ps_pubsub_class.hpp"
#include "ps_rtos_stats.hpp"

ps_syslog_class &the_logger()
{
	static ps_syslog_rtos syslog;
	return syslog;
}

//task wrapper
//void print_thread_wrapper(void *pvParameters)
//{
//    ps_syslog_rtos *pssr = static_cast<ps_syslog_rtos*>(pvParameters);
//    pssr->print_thread_method();
//    //does not return
//}
//task wrapper
void publish_thread_wrapper(void *pvParameters)
{
    ps_syslog_rtos *pssr = static_cast<ps_syslog_rtos*>(pvParameters);
    pssr->publish_thread_method();
    //does not return
}

ps_syslog_rtos::ps_syslog_rtos()
{
        log_publish_queue = new ps_queue_rtos(sizeof (ps_syslog_message_t), SYSLOG_PUBLISH_QUEUE_LENGTH);

        if (log_publish_queue) {
            //start log publish thread
            if (xTaskCreate(publish_thread_wrapper, /* The function that implements the task. */
                    "Syslog", /* The text name assigned to the task*/
                    SYSLOG_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
                    (void *) this, /* The parameter passed to the task. */
                    SYSLOG_TASK_PRIORITY, /* The priority assigned to the task. */
                    NULL) /*The task handle */
                    != pdPASS) {
                PS_ERROR("Syslog: Publish Task FAIL!");
            }
        } else {
            PS_ERROR("Syslog: Publish Q FAIL!");
        }

#ifdef LOG_UART
	if (rtos_debug_init(LOG_UART, LOG_UART_BAUDRATE) == 0)
    {
//	//queue for messages
//        log_print_queue = new ps_queue_rtos(sizeof (ps_syslog_message_t), SYSLOG_PRINT_QUEUE_LENGTH);
//
//        if (log_print_queue) {
//            //start log print thread
//            if (xTaskCreate(print_thread_wrapper, /* The function that implements the task. */
//                    "Syslog", /* The text name assigned to the task*/
//                    SYSLOG_TASK_STACK_SIZE, /* The size of the stack to allocate to the task. */
//                    (void *) this, /* The parameter passed to the task. */
//                    SYSLOG_TASK_PRIORITY, /* The priority assigned to the task. */
//                    NULL) /*The task handle */
//                    == pdPASS) {
//            } else {
//                PS_ERROR("Syslog: Print Task FAIL!");
//            }
//        } else {
//            PS_ERROR("Syslog: Print Q FAIL!");
//        }
    }
    else
    {
        PS_ERROR("Syslog: UART Init FAIL!");      //!!
    }
#endif
}

ps_syslog_rtos::~ps_syslog_rtos()
{
//    if (task) vTaskDelete(task);
//    if (log_print_queue) delete log_print_queue;
}

//add locally-generated new messages to both queues
void ps_syslog_rtos::new_log_message(ps_syslog_message_t *msg) {
    if (log_publish_queue->copy_message_to_q(msg, sizeof (ps_syslog_message_t)) != PS_OK) {
        PS_ERROR("log: publish copy_message_to_q failed");
    }
#ifdef LOG_UART
    print_log_message(msg);
//    if (log_print_queue->copy_message_to_q(msg, sizeof (ps_syslog_message_t)) != PS_OK) {
//        PS_ERROR("log: print copy_message_to_q failed");
//    }
#endif
}

//print log messages from the print queue
//void ps_syslog_rtos::print_thread_method()
//{
//#ifdef LOG_UART
//	the_broker().register_object(SYSLOG_PACKET, this);
//
//	{
//		ps_syslog_message_t msg;
//		msg.severity = SYSLOG_ROUTINE;
//		strncpy(msg.source, "sys", PS_SOURCE_LENGTH);
//		strncpy(msg.text, "log started", PS_MAX_LOG_TEXT);
//		print_log_message(&msg);
//	}
//
//	while(1)
//	{
//		int len;
//		ps_syslog_message_t *log_msg = (ps_syslog_message_t *) log_print_queue->get_next_message(-1, &len);
//		
//		print_log_message(log_msg);
//        
//		log_print_queue->done_with_message(log_msg);
//	}
//#endif
//}
//send upstream log messages from the publish queue
void ps_syslog_rtos::publish_thread_method()
{
	while(1)
	{
		int len;
		ps_syslog_message_t *log_msg = (ps_syslog_message_t *) log_publish_queue->get_next_message(-1, &len);

		the_broker().transmit_packet(SYSLOG_PACKET, log_msg, len);

		log_publish_queue->done_with_message(log_msg);
	}
}

void ps_syslog_rtos::message_handler(ps_packet_source_t packet_source,
        ps_packet_type_t   packet_type,
        const void *msg, int length)
{
    //don't print log messages from elsewhere
}

void ps_syslog_rtos::print_log_message(ps_syslog_message_t *log_msg)
{
#ifdef LOG_UART
	const int MAX_MESSAGE = (PS_SOURCE_LENGTH + PS_MAX_LOG_TEXT + 20);
	char severity;
    
	char printBuff[MAX_MESSAGE];

	switch (log_msg->severity) {
	case SYSLOG_ROUTINE:
	default:
		severity = 'R';
		break;
	case SYSLOG_INFO:
		severity = 'I';
		break;
	case SYSLOG_WARNING:
		severity = 'W';
		break;
	case SYSLOG_ERROR:
		severity = 'E';
		break;
	case SYSLOG_FAILURE:
		severity = 'F';
		break;
	}
	//make sure we don't overflow
	log_msg->text[PS_MAX_LOG_TEXT] = '\0';
	log_msg->source[PS_SOURCE_LENGTH] = '\0';

	//remove newline
	int len = (int) strlen(log_msg->text);
	if (log_msg->text[len-1] == '\n') log_msg->text[len-1] = '\0';

	snprintf(printBuff, MAX_MESSAGE, "%c@%s: %s",
			severity, log_msg->source, log_msg->text);

	PS_DEBUG("%s", printBuff);
#endif
}

void ps_syslog_rtos::debug_system_stats()
{
    GenerateRunTimeTaskStats();
}