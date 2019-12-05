#ifndef YAMC_SERVER_SERVER_H
#define YAMC_SERVER_SERVER_H

#include "../Sockets.h"

namespace yamc
{
	class Server
	{
	public:
		Server();
		void run(int port);
		~Server();
	};
}

#endif