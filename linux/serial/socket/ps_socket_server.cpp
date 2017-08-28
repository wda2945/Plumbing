//
//  ps_socket_server.cpp
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
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include "ps_socket_server.hpp"

#include "packet/serial_packet/ps_packet_serial_linux.hpp"
#include "transport/ps_transport_linux.hpp"
#include "network/ps_network.hpp"

ps_socket_server::ps_socket_server(int _listen_port_number, int _ping_port_number)
	: ps_socket("socket", -1)
{
	listen_port_number  = _listen_port_number;
	ping_port_number	= _ping_port_number;

	//create agent Listen thread
	listenThread = new std::thread([this](){ServerListenThreadMethod();});

	//create agent Ping thread
	if (ping_port_number)
		pingThread = new std::thread([this](){ServerPingThreadMethod();});
}

ps_socket_server::~ps_socket_server()
{
	delete listenThread;
	if (pingThread) delete pingThread;

	close(listen_socket);
}



void ps_socket_server::ServerListenThreadMethod()
{
	struct sockaddr client_address;
	socklen_t client_address_len;

	const struct sigaction sa {{SIG_IGN}, 0, 0};
	sigaction(SIGPIPE, &sa, NULL);

	PS_DEBUG("server: listen thread ready");

	while ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		PS_ERROR("server: listen socket() error: %s", strerror(errno));
		sleep(1);
	}
	int optval = 1;
	setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &optval, 4);

	PS_DEBUG("server: listen socket created");

	//bind socket address
	struct sockaddr_in my_address;
	memset(&my_address, 0, sizeof(my_address));
	my_address.sin_family = AF_INET;
	my_address.sin_port = htons(listen_port_number);
	my_address.sin_addr.s_addr = INADDR_ANY;

	while (bind(listen_socket, (struct sockaddr*) &my_address, sizeof(my_address)) < 0)
	{
		PS_ERROR("server: bind() error: %s", strerror(errno));

		if (errno == EADDRINUSE) sleep(10);

		sleep(1);
	}

	PS_DEBUG("server: listen socket ready");

	while(1)
	{
		//wait for connect
		while(listen(listen_socket, 10) != 0)
		{
			PS_ERROR("server: listen() error %s", strerror(errno));
			sleep(1);
			//ignore errors, just retry
		}

		client_address_len = sizeof(client_address);
		int acceptSocket = accept(listen_socket, &client_address, &client_address_len);

		if (acceptSocket >= 0)
		{
			char address[20];

			//print the address
			struct sockaddr_in *client_address_in = (struct sockaddr_in *) &client_address;

			uint8_t addrBytes[4];
			memcpy(addrBytes, &client_address_in->sin_addr.s_addr, 4);

			snprintf(address, 20, "%i.%i.%i.%i", addrBytes[0], addrBytes[1], addrBytes[2], addrBytes[3]);

			LogInfo("sock: connect from %s", address);
			set_new_socket(acceptSocket);
			set_node_name(address);

			set_serial_status(PS_SERIAL_ONLINE);
		}
	}

}

//thread to broadcast ping every 5 seconds

void ps_socket_server::ServerPingThreadMethod()
{
	int buflen = strlen(SOURCE_NAME) + 6;
	char buf[buflen];

	snprintf(buf, buflen, "%s-%i", SOURCE_NAME, listen_port_number);

	PS_DEBUG("server: ping thread started");

	while (1)
	{
		ServerPing(buf, buflen, ping_port_number);
		sleep(5);
	}
}

//ping
void ServerPing(const char *buf, int buflen, int _ping_port_number)
{
	int broadcastSocket;
	struct sockaddr_in sockAddress;
	socklen_t len = sizeof(sockAddress);

	memset(&sockAddress, 0, sizeof(sockAddress));
	sockAddress.sin_family = AF_INET;
	sockAddress.sin_port = htons(_ping_port_number);
	sockAddress.sin_addr.s_addr = 0xffffffff;

	if ((broadcastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0)
	{
		int bc {1};
		setsockopt(broadcastSocket, SOL_SOCKET, SO_BROADCAST, &bc, 4);

		if (sendto(broadcastSocket, buf, buflen, 0, (struct sockaddr*) &sockAddress, len) > 0)
		{
//				PS_DEBUG("server: broadcast datagram sent");
		}
		else
		{
			PS_ERROR("server: broadcast sendto error : %s, ", strerror(errno));
		}
		close(broadcastSocket);
	}
	else
	{
		PS_ERROR("server: broadcast socket error : %s, ", strerror(errno));
	}

}
