#include "Server.h"
#include <stdexcept>
#include <iostream>

int main()
{
	try {
		yamc::Server server;
		server.run(8888);
	}
	catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}