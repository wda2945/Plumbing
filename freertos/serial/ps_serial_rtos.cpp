//
//  ps_serial_rtos.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include "ps_serial_rtos.hpp"
#include "ps_config.h"

ps_serial_rtos::ps_serial_rtos(const char *name) : ps_serial_class(name) {

}

ps_result_enum ps_serial_rtos::init(UART_MODULE uart, int baudrate) {

    //initialize the uart

    if (!Serial_begin(uart, baudrate,
            static_cast<UART_LINE_CONTROL_MODE> (UART_DATA_SIZE_8_BITS |
            UART_PARITY_NONE | UART_STOP_BITS_1),
            UART_BROKER_BUFFER_SIZE, UART_BROKER_BUFFER_SIZE)) {
        PS_ERROR("Serial_begin(%i) fail", uart);
        return PS_IO_INIT_FAIL;
    }

    my_uart = uart;
    my_baudrate = baudrate;

    return PS_OK;
}

//send bytes

ps_result_enum ps_serial_rtos::write_bytes(const void *data, int length) {
    Serial_write_bytes(my_uart, (const char*) data, length);
    return PS_OK;
}

//receive bytes

bool ps_serial_rtos::data_available() {
    return Serial_available(my_uart);
}

int ps_serial_rtos::read_bytes(void *data, int length) {
    int c = 0;
    int l = length;
    uint8_t *p = (uint8_t*) data;

    while (l) {
        UART_DATA ud = Serial_read(my_uart);
        *p++ = ud.data8bit;
        l--;
        c++;
    }
    return c;
}