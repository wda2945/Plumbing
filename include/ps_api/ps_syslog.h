/* 
 * File:   ps_syslog.h
 * Author: martin
 *
 * Created on August 7, 2013, 8:19 PM
 */

#ifndef PS_SYSLOG_H
#define	PS_SYSLOG_H

#include "ps_config.h"
#include <stdio.h>

//sys logging severity
typedef enum {
   SYSLOG_ROUTINE, SYSLOG_INFO, SYSLOG_WARNING, SYSLOG_ERROR, SYSLOG_FAILURE
} SysLogSeverity_enum;

enum {
	LOG_ALL, LOG_INFO_PLUS, LOG_WARN_PLUS, LOG_NONE
};

#ifndef SYSLOG_LEVEL
#define SYSLOG_LEVEL LOG_ALL
#endif

#ifdef __cplusplus
extern "C" {
#endif

void _LogMessage(SysLogSeverity_enum _severity, const char *_message, const char *_file);

#define SysLog( s, ...) {char tmp[PS_MAX_LOG_TEXT];\
    snprintf(tmp,PS_MAX_LOG_TEXT,__VA_ARGS__);\
    tmp[PS_MAX_LOG_TEXT-1] = 0;\
    _LogMessage(s, tmp, __BASE_FILE__);}

#if (SYSLOG_LEVEL == LOG_ALL)
#define LogRoutine( ...) {SysLog( SYSLOG_ROUTINE, __VA_ARGS__);}
#else
    #define LogRoutine( m, ...)
#endif

    #if (SYSLOG_LEVEL <= LOG_INFO_PLUS)
#define LogInfo( ...) {SysLog( SYSLOG_INFO, __VA_ARGS__);}
#else
    #define LogInfo( m, ...)
#endif

#if (SYSLOG_LEVEL <= LOG_WARN_PLUS)
#define LogWarning( ...) {SysLog( SYSLOG_WARNING, __VA_ARGS__);}
#else
    #define LogWarning( m, ...)
#endif

#define LogError(...) {SysLog(SYSLOG_ERROR, __VA_ARGS__);}

//output stats to PS_DEBUG())
void ps_debug_transport_stats();
void ps_debug_system_stats();

#ifdef __cplusplus
}
#endif

#endif	/* PS_SYSLOG_H */

