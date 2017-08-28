/*
 * ps_pingListener.c
 *
 *  Created on: Mar 31, 2016
 *      Author: martin
 */

#include "stdint.h"

#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ps_pingListener.h"
#include "ps_config.h"
#include "ps_common.h"

#define MAXADDRLEN 256
#define BUFLEN 256

void pingListener(PingCallback_t *callback, void *args)
{
    int pingSocket;
    char buf[BUFLEN];
    char abuf[MAXADDRLEN];
    ssize_t n;
    
    struct sockaddr_in *addr = (struct sockaddr_in*) abuf;
    socklen_t alen;
    
    while (1)
    {
        struct sockaddr_in sockAddress;
        
        memset(&sockAddress, 0, sizeof(sockAddress));
        sockAddress.sin_family = AF_INET;
        sockAddress.sin_port = htons(5000);
        sockAddress.sin_addr.s_addr = INADDR_ANY;
        
        if ((pingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) >= 0)
        {
            while (bind(pingSocket, (struct sockaddr*) &sockAddress, sizeof(sockAddress)) == -1)
            {
                PS_ERROR("pingListener: bind() error: %s", strerror(errno));
                sleep(10);
            }
            PS_DEBUG("pingListener: socket ready");
            
            while (1)
            {
                
                alen = MAXADDRLEN;
                
                if ((n = recvfrom(pingSocket, buf, BUFLEN, 0, (struct sockaddr*) addr, &alen)) < 0)
                {
                    PS_ERROR("pingListener: recvfrom - %s", strerror(errno));
                    break;
                }
                buf[n] = 0;
                char ip_addr[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &addr->sin_addr.s_addr, ip_addr, INET_ADDRSTRLEN);
                //                PS_DEBUG("pingListener: %s @ %s", buf, ip_addr);
                
                if (callback) (callback)(buf, ip_addr, args);
            }
            close(pingSocket);
        }
    }
}
