/*
 * ps_debug_ios.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */
#include <errno.h>
#include <string.h>
#include <time.h>
#include <thread>

using namespace std;

#include "ps_config.h"
#include "ps_common.h"

//used by PS_DEBUG and PS_ERROR macros

void print_debug_message(const char *text)
{
    print_debug_message_to_file(stdout, text);
}
void print_error_message(const char *text)
{
   print_debug_message_to_file(stderr, text);
}

mutex debugMtx;

void print_debug_message_to_file(FILE *dbgfile, const char *text)
{
    unique_lock<mutex> lck {debugMtx};
    
	char printBuff[30];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(printBuff, 30, "%02i:%02i:%02i ",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);
    
	fprintf(dbgfile, "%s", printBuff);
	fprintf(dbgfile, "%s", text);
	fprintf(dbgfile, "\n");
	fflush(dbgfile);
}

FILE *fopen_logfile(const char *name)
{
    return stdout;
}
