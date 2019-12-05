#include "Server.h"
#include <stdexcept>
#include <iostream>

namespace yamc
{
	Server::Server()
	{
		if (initSockets() != 0) {
			throw std::runtime_error("Cannot initialize sockets.");
		}
	}

	void Server::run(int port)
	{
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (IS_INVALID_SOCKET(sock)) {
			throw std::runtime_error("Cannot create socket.");
		}

		struct sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = INADDR_ANY;
		serverAddress.sin_port = htons(port);

		if (bind(sock, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
			throw std::runtime_error("Cannot bind socket.");
		}

		std::cout << "Running on port " << port << "..." << std::endl;

		listen(sock, 3);

		struct sockaddr_in clientAddress;
		int clientAddressLength = sizeof(clientAddress);
		SOCKET clientSock = accept(sock, (struct sockaddr*)&clientAddress, &clientAddressLength);
		if (IS_INVALID_SOCKET(clientSock)) {
			throw std::runtime_error("Cannot accept client socket.");
		}

		const char* message = "Hello from server.";
		send(clientSock, message, strlen(message) + 1, 0);

		safelyCloseSocket(sock);
	}

	Server::~Server()
	{
		deinitSockets();
	}
}