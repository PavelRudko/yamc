#ifndef YAMC_SOCKETS_H
#define YAMC_SOCKETS_H

#ifdef _WIN32
	#ifndef _WIN32_WINNT
	    #define _WIN32_WINNT 0x0501
	#endif
	#include <winsock2.h>
	#include <Ws2tcpip.h>

    #define IS_INVALID_SOCKET(sock) (sock == INVALID_SOCKET)

#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#include <netdb.h> 
	#include <unistd.h>

    typedef int SOCKET;
    #define IS_INVALID_SOCKET(sock) (sock < 0)
    #define INVALID_SOCKET -1
#endif

int initSockets(void);
int deinitSockets(void);
int safelyCloseSocket(SOCKET sock);
bool setToNonBlockingMode(SOCKET sock);

#endif