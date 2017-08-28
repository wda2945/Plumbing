//
//  ps_serial_class_linux.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_serial_linux_hpp
#define ps_serial_linux_hpp

#include "serial/ps_serial_class.hpp"

class ps_serial_linux : public ps_serial_class {
    
    int FD;

public:
    
    ps_serial_linux(const char *_name, const char *devicePath, unsigned int baudrate);
    ps_serial_linux(const char *_name, int fd);

    ~ps_serial_linux();
    
    //send bytes
    ps_result_enum write_bytes(const void *data, int length) override;
    
    //receive bytes
    bool data_available() override;
    
    int read_bytes(void *data, int length) override;
    
};

#endif /* ps_serial_linux_hpp */
