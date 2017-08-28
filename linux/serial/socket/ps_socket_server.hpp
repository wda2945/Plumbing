//
//  ps_socket_server.hpp
//  RobotFramework
//
//  Created by Martin Lane-Smith on 5/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef ps_socket_server_hpp
#define ps_socket_server_hpp

#include <thread>

#include "ps_socket.hpp"

void ServerPing(const char *buf, int buflen, int _ping_port_number);

class ps_socket_server : public ps_socket{
	int listen_port_number;
	int ping_port_number;

	int listen_socket;

	std::thread *listenThread;
	std::thread *pingThread;

	void ServerListenThreadMethod();
	void ServerPingThreadMethod();

public:

	ps_socket_server(int _listen_port_number, int _ping_port_number);
	~ps_socket_server();

};

#endif /* ps_socket_server_hpp */
