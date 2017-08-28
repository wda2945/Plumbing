//
//  ps_socket_client.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_client_hpp
#define ps_socket_client_hpp

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>

#include "ps_socket.hpp"
#include "ps_pingListener.h"

typedef enum {
	PS_CLIENT_UNDEFINED,
	PS_CLIENT_SEARCHING,
	PS_CLIENT_CONNECTING,
	PS_CLIENT_CONNECT_ERROR,
	PS_CLIENT_CONNECTED,
	PS_CLIENT_LOST_CONNECTION
} ps_client_status_enum;

typedef union {
    uint8_t bytes[4];
    in_addr_t address;
} IPaddress_t;

typedef struct {
    PingCallback_t *callback;
    void *args;
} PingObserver_t;

class ps_socket_client  : public ps_socket {
public:

    ps_socket_client(const char *ip_address, int port_number);
    ~ps_socket_client();

    void set_ip_address(const char *_ip_address, int _port_number);
    
    bool isConnected(){return (connect_status == PS_CLIENT_CONNECTED);}
    
    //get a caption for the app
	void get_client_status_caption(char *buff, int len);
    
    void add_ping_observer(PingCallback_t callback, void *args);
    
    void message_handler(ps_packet_source_t packet_source,
                         ps_packet_type_t   packet_type,
                         const void *msg, int length) override {}
protected:
    
	char *server_name;
	char *ip_address;
	int port_number;
    
    //thread to connect to server
    std::thread *connect_thread;

    //thread to connect to server
    std::thread *ping_thread;

    //status - see above
	ps_client_status_enum connect_status;

	IPaddress_t ipAddress;
	char		ipAddressStr[30];

    //connect thread method
	void client_connect_thread_method();

    //ping thread method
    void client_ping_thread_method();
    void ping_callback_method(char *robot, char *ip_address);
    
    //connection attempt method
	ps_client_status_enum connect_to_server();

    int connect_retry(int domain, int type, int protocol, const struct sockaddr *addr, socklen_t alen);
    
	void set_client_status(ps_client_status_enum stat);
    
    //intercepts socket errors
    void notify_new_event(ps_root_class *rcl, int res) override;

    std::vector<PingObserver_t> pingObservers;
    
    friend void callback(char *robot, char *ip_address, void *args);
};

#endif /* ps_socket_client_hpp */
