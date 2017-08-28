//
//  ps_socket_client.cpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
//#include <stropts.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <netdb.h>

#include "serial/socket/ps_socket_client.hpp"

ps_socket_client::ps_socket_client(const char *_ip_address, int _port_number): ps_socket("sock_client", 0)
{
	if (_ip_address) ip_address = strdup(_ip_address);
    else ip_address = nullptr;
    
	port_number = _port_number;
    
	//create connect thread
    connect_thread = new std::thread([this](){client_connect_thread_method();});
    
    //create ping listen thread
    ping_thread = new std::thread([this](){client_ping_thread_method();});
}

void ps_socket_client::set_ip_address(const char *_ip_address, int _port_number)
{
    switch(connect_status)
    {
        case PS_CLIENT_CONNECTED:
            PS_TRACE("sock: new ip - closing current connection");
            close(rxSocket);
            close(txSocket);
            txSocket = rxSocket = 0;
            set_client_status(PS_CLIENT_LOST_CONNECTION);
            break;
        default:
            break;
    }
    if (_ip_address) {
        ip_address = strdup(_ip_address);
        PS_TRACE("sock: ip address = %s", _ip_address);
    }
    else ip_address = nullptr;
    
    port_number = _port_number;
}

ps_socket_client::~ps_socket_client()
{
    delete connect_thread;
    delete ping_thread;
}

void ps_socket_client::client_connect_thread_method()
{
    PS_DEBUG("sock: connect thread started");
    
    while (1)
    {
        sleep(2);
        
        switch(connect_status)
        {
            case PS_CLIENT_CONNECTED:
                sleep(1);
                break;
            case PS_CLIENT_CONNECT_ERROR:
            case PS_CLIENT_LOST_CONNECTION:
                sleep(5);
            default:

                if (ip_address)
                {
                    PS_DEBUG("sock: connecting to %s:%d", ip_address, port_number);
                    //parse ip addres
                    int b0, b1, b2, b3;
                    sscanf(ip_address, "%i.%i.%i.%i", &b0, &b1, &b2, &b3);
                    ipAddress.bytes[0] = b0;
                    ipAddress.bytes[1] = b1;
                    ipAddress.bytes[2] = b2;
                    ipAddress.bytes[3] = b3;
                    set_client_status(connect_to_server());
                }
                break;
        }
    }
}

ps_client_status_enum ps_socket_client::connect_to_server()
{
    struct sockaddr_in serverSockAddress;

    sprintf(ipAddressStr, "%i.%i.%i.%i", ipAddress.bytes[0], ipAddress.bytes[1], ipAddress.bytes[2], ipAddress.bytes[3]);
    
    set_client_status(PS_CLIENT_CONNECTING);

    memset(&serverSockAddress, 0, sizeof(serverSockAddress));
//    serverSockAddress.sin_len = sizeof(serverSockAddress);
    serverSockAddress.sin_family = AF_INET;
    serverSockAddress.sin_port = htons(port_number);
    serverSockAddress.sin_addr.s_addr = ipAddress.address;

    //create socket & connect
    for (int i=0; i < 5; i++)
    {
        if ((rxSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            PS_ERROR("sock: create error: %s", strerror(errno));
            return(PS_CLIENT_CONNECT_ERROR);
        }
        else if (connect(rxSocket, (const struct sockaddr*) &serverSockAddress, sizeof(serverSockAddress)) == 0)
        {
            //accepted
            PS_DEBUG("sock: client connected to %s:%d", ip_address, port_number);
            //dup a socket for the send
            txSocket = dup(rxSocket);
            return(PS_CLIENT_CONNECTED);
        }
        else
        {
            close(rxSocket);
        }
        sleep(1);
    }
    PS_ERROR("sock: connect error");
    return(PS_CLIENT_CONNECT_ERROR);
}

void ps_socket_client::notify_new_event(ps_root_class *rcl, int _res)
{
    ps_serial_status_enum res = static_cast<ps_serial_status_enum>(_res);
    switch(res)
    {
        case PS_SERIAL_WRITE_ERROR:
        case PS_SERIAL_READ_ERROR:
        case PS_SERIAL_OFFLINE:
            PS_TRACE("sock: new event serial offline");
            close(rxSocket);
            close(txSocket);
            txSocket = rxSocket = 0;
            set_client_status(PS_CLIENT_LOST_CONNECTION);
            break;
        default:
            PS_TRACE("sock: new event %d", _res);
            ps_serial_class::notify_new_event(rcl, res);
            break;
    }
}

void ps_socket_client::set_client_status(ps_client_status_enum stat)
{
	connect_status = stat;

	switch(stat)
	{
	case PS_CLIENT_SEARCHING:
	case PS_CLIENT_CONNECTING:
            PS_TRACE("sock: connecting");
	case PS_CLIENT_CONNECT_ERROR:
//        ps_serial_class::notify_new_event(this, PS_SERIAL_OFFLINE);
	default:
        PS_TRACE("sock: not connected");
		break;
	case PS_CLIENT_CONNECTED:
        PS_TRACE("sock: connected");
		ps_serial_class::notify_new_event(this, PS_SERIAL_ONLINE);
		break;
	case PS_CLIENT_LOST_CONNECTION:
        PS_TRACE("sock: lost connection");
		ps_serial_class::notify_new_event(this, PS_SERIAL_OFFLINE);
		break;
	}
}

void ps_socket_client::get_client_status_caption(char *buff, int len)
{
	switch(connect_status)
	{
	case PS_CLIENT_SEARCHING:
		snprintf(buff, len, "Searching...");
		break;
	case PS_CLIENT_CONNECTING:
		snprintf(buff, len, "Connecting to %s", ipAddressStr);
		break;
	case PS_CLIENT_CONNECT_ERROR:
		snprintf(buff, len, "Connect Error");
		break;
	case PS_CLIENT_CONNECTED:
		snprintf(buff, len, "Connected to %s", ipAddressStr);
		break;
    case PS_CLIENT_LOST_CONNECTION:
		snprintf(buff, len, "No Connection");
		break;
	default:
		snprintf(buff, len, "Unknown");
		break;
	}
}

//thread to listen for pings
void callback(char *robot, char *ip_address, void *args);

void ps_socket_client::client_ping_thread_method()
{
    pingListener(callback, this);
}

//ping callback
void callback(char *robot, char *ip_address, void *args)
{
    ps_socket_client *psc = (ps_socket_client*) args;
    psc->ping_callback_method(robot, ip_address);
}
void ps_socket_client::ping_callback_method(char *robot, char *ip_address)
{
    for (auto obs : pingObservers)
    {
        (obs.callback)(robot, ip_address, obs.args);
    }
}
void ps_socket_client::add_ping_observer(PingCallback_t callback, void *args)
{
    PingObserver_t obs {callback, args};
    pingObservers.push_back(obs);
}

