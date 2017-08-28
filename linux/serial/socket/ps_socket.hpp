//
//  ps_socket.hpp
//  RobotFramework
//
//  Connected stream socket
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_hpp
#define ps_socket_hpp

#include "serial/ps_serial_class.hpp"

class ps_socket : public ps_serial_class {

public:
	ps_socket(const char *name, int socket);
    
    ~ps_socket();

    //send bytes
    ps_result_enum write_bytes(const void *data, int length) override;

    //receive bytes
    bool data_available() override;

    int read_bytes(void *data, int length) override;
    
    void set_serial_status(ps_serial_status_enum s);

    virtual void disconnect() override;

protected:
    
    void set_new_socket(int socket);

    int rxSocket {-1};
    int txSocket {-1};
    
};

#endif /* ps_socket_server_hpp */
