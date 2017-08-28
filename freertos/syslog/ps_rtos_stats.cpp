/*
 * File:   SystemStats.c
 * Author: martin
 *
 * Created on March 27, 2014
 */
#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define _SUPPRESS_PLIB_WARNING
#define _DISABLE_OPENADC10_CONFIGPORT_WARNING
#include "plib.h"

#include "FreeRTOS.h"
#include "task.h"

#include "ps_config.h"
#include "ps.h"
#include "ps_rtos_stats.hpp"

void CONFIGURE_TIMER_FOR_RUN_TIME_STATS() {
    OpenTimer45(T45_ON | T45_IDLE_STOP | T45_PS_1_256, 0xffffffff);
}

uint32_t GET_RUN_TIME_COUNTER_VALUE() {
    return ReadTimer45();
}

TaskStatus_t *pxTaskStatusArray = nullptr;

void GenerateRunTimeTaskStats() {
    PS_TRACE("GenerateRunTimeTaskStats");
 
    volatile UBaseType_t uxArraySize, x, i;
    uint32_t ulTotalRunTime;
    uint32_t ulStatsAsPercentage;
    uint32_t ulTaskRunTime = 0;

    // Take a snapshot of the number of tasks in case it changes while this
    // function is executing.
    uxArraySize = uxTaskGetNumberOfTasks();

    if (!pxTaskStatusArray) pxTaskStatusArray = (TaskStatus_t *) MEMORY_ALLOC(sizeof(TaskStatus_t) * uxArraySize);
    
    if (!pxTaskStatusArray) {
        PS_ERROR("TaskStats: No memory");
        return;
    } 
    
    // Generate raw status information about each task.
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    TaskHandle_t idleTask = xTaskGetIdleTaskHandle();

    // For percentage calculations.
    ulTotalRunTime /= 100UL;

    // Avoid divide by zero errors.
    if (ulTotalRunTime > 0) {
        for (x = 0; x < uxArraySize; x++) {
            // What percentage of the total run time has the task used?
            // This will always be rounded down to the nearest integer.
            // ulTotalRunTime has already been divided by 100.
            ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalRunTime;

            PS_DEBUG("%s cpu = %d%%, free stack = %d bytes",
                    pxTaskStatusArray[ x ].pcTaskName,
                    ulStatsAsPercentage,
                    pxTaskStatusArray[ x ].usStackHighWaterMark);

            if (pxTaskStatusArray[ x ].xHandle == idleTask) continue;

            ulTaskRunTime += pxTaskStatusArray[ x ].ulRunTimeCounter;
        }

        ulStatsAsPercentage = (ulTaskRunTime / ulTotalRunTime);
        
#ifdef GET_FREE_HEAP_SIZE
        PS_DEBUG("Total cpu = %d%%, free heap = %d bytes",
                ulStatsAsPercentage,
                xPortGetFreeHeapSize());
#else
        PS_DEBUG("Total cpu = %d%%", ulStatsAsPercentage);
#endif

    }
}
