/*
 * ps_debug_linux.cpp
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
#include <mutex>
#include <sys/stat.h>

using namespace std;

#include "ps_syslog_linux.hpp"

mutex debugMtx;

char *log_folder_path {nullptr};

int make_log_folder_path()
{
	if (!log_folder_path)
	{
		char path[200];
		const time_t now = time(NULL);
		struct tm *timestruct = localtime(&now);

		//new folder name
		snprintf(path, 200, "%s/%4i_%02i_%02i_%02i_%02i_%02i", LOGFILE_FOLDER,
				timestruct->tm_year + 1900, timestruct->tm_mon + 1, timestruct->tm_mday,
				timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

		if (mkdir(path, S_IRUSR + S_IWUSR + S_IXUSR + S_IROTH + S_IWOTH + S_IXOTH) < 0)
		{
			PS_ERROR("syslog: mkdir(%s) fail (%s)", path, strerror(errno));
			//use logfiles/
			snprintf(path, 200, "%s", LOGFILE_FOLDER);
			return -1;
		}
		else
		{
			PS_DEBUG("syslog: %s", path);
		}

		log_folder_path = (char*) malloc(strlen(path)+1);
		strcpy(log_folder_path, path);
	}
	return 0;
}

//defaults to plumbing.log
void print_debug_message(const char *text)
{
    if (plumbing_debug_file){
        print_debug_message_to_file(plumbing_debug_file, text);
    }
    print_debug_message_to_file(stdout, text);
}

void print_error_message(const char *text)
{
    if (plumbing_debug_file){
        print_debug_message_to_file(plumbing_debug_file, text);
    }
    print_debug_message_to_file(stderr, text);
}

void print_debug_message_to_file(FILE *dbgfile, const char *text)
{
	char printBuff[30];
	const time_t now = time(NULL);
	struct tm *timestruct = localtime(&now);

	snprintf(printBuff, 30, "%02i:%02i:%02i ",
			timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

    debugMtx.lock();

	fprintf(dbgfile, "%s", printBuff);
	fprintf(dbgfile, "%s", text);
	fprintf(dbgfile, "\n");
	fflush(dbgfile);
    
    debugMtx.unlock();
}

FILE *fopen_logfile(const char *name)
{
	FILE *dbg;
	char path[200];

	if (log_folder_path == nullptr)
	{
		make_log_folder_path();
	}
	snprintf(path, 200, "%s/%s.txt", log_folder_path, name);

	dbg = fopen(path, "w");

    if (dbg == NULL)
    {
        fprintf (stderr, "Couldn’t open %s; %s\n", path, strerror (errno));
        return (FILE*) 0;
    }
    else {
        char printBuff[30];
        const time_t now = time(NULL);
        struct tm *timestruct = localtime(&now);
        
        snprintf(printBuff, 30, "%02i/%02i/%04i %02i:%02i:%02i ",
                 timestruct->tm_mon + 1, timestruct->tm_mday, timestruct->tm_year + 1900,
                 timestruct->tm_hour, timestruct->tm_min, timestruct->tm_sec);

        fprintf (stdout, "%s", printBuff);
    	fprintf (stdout, "Opened logfile %s\n", path);
        fprintf (dbg, "%s", printBuff);
    	fprintf (dbg, "Started %s logfile\n", name);

    	return dbg;
    }
}
