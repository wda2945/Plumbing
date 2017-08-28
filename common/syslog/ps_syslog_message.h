/* 
 * File:   ps_syslog_message.hpp
 * Author: martin
 *
 * Syslog Message
 *
 * Created on April 19, 2014, 11:09 AM
 */

#ifndef _PS_SYSLOG_MESSAGE_H
#define	_PS_SYSLOG_MESSAGE_H

typedef struct {
    uint8_t severity;
    char source[PS_SOURCE_LENGTH + 1];
    char text[PS_MAX_LOG_TEXT + 1];
} ps_syslog_message_t;

#endif	/* _PS_SYSLOG_MESSAGE_H */

