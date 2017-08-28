//
//  ps_serial_rtos.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_rtos_hpp
#define ps_serial_rtos_hpp

#include "serial/ps_serial_class.hpp"
#include "Serial.h"

class ps_serial_rtos : public ps_serial_class {
    UART_MODULE my_uart;
    int         my_baudrate {0};

public:
    ps_serial_rtos(const char *name);
    ~ps_serial_rtos() {}
    
    ps_result_enum init(UART_MODULE uart, int baudrate);
    
        //send bytes
    ps_result_enum write_bytes(const void *data, int length) override;
    
    //receive bytes
    bool data_available() override;
    int read_bytes(void *data, int length) override;
    
};

#endif /* ps_serial_rtos_hpp */
