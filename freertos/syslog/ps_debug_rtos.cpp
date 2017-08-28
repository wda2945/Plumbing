/*
 * ps_debug_rtos.cpp
 *
 *	Syslog Services
 *
 *  Created on: Jan 19, 2014
 *      Author: martin
 */

#include "xc.h"
#include <string.h>

using namespace std;

#include "ps_common.h"
#include "ps_config.h"
#include "syslog/ps_syslog_rtos.hpp"
#include "serial/Serial.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t debugSerialMutex;
SemaphoreHandle_t debugBufferMutex;

char tick_text[PS_TICK_TEXT] = "";
char debugBuff[PS_MAX_LOG_TEXT];


UART_MODULE log_uart;
bool log_uart_init = false;

int rtos_debug_init(UART_MODULE _log_uart, int _log_uart_baudrate)
{
    
#ifdef LOG_UART

	log_uart = _log_uart;

    if (Serial_begin(_log_uart, _log_uart_baudrate, (UART_LINE_CONTROL_MODE) 0, 0, PS_MAX_LOG_TEXT * 2)) {
        debugSerialMutex = xSemaphoreCreateMutex(); 
        debugBufferMutex = xSemaphoreCreateMutex(); 
        if (debugSerialMutex && debugBufferMutex)
        {
            log_uart_init = true;
            PS_DEBUG("Debug UART %d opened", log_uart);
            return 0;
        }
    } 
#endif
    return -1;
}

char timeCount[100];
TickType_t previousDebugCall {0};
#define MIN_DEBUG_INTERVAL 0
void print_debug_message(const char *text)
{
#ifdef LOG_UART
    
    if (!log_uart_init)
    {
        if (rtos_debug_init(LOG_UART, LOG_UART_BAUDRATE) < 0) return;
    }

    while (previousDebugCall + MIN_DEBUG_INTERVAL > xTaskGetTickCount())
    {
        vTaskDelay(50);
    }
    
    xSemaphoreTake(debugSerialMutex, portMAX_DELAY);

    previousDebugCall = xTaskGetTickCount();

    snprintf(timeCount, 100, " %8li ", xTaskGetTickCount());

    Serial_write_string(log_uart, tick_text);
    Serial_write_string(log_uart, timeCount);
    Serial_write_string(log_uart, text);
    Serial_write(log_uart, (uint8_t)'\n');

//#ifdef DEBUG_FLUSH
    Serial_flush(log_uart);
//#endif
    
    xSemaphoreGive(debugSerialMutex);
#endif
}
void print_error_message(const char *text)
{
	print_debug_message(text);
}
