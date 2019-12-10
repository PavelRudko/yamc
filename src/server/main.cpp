#include "Server.h"
#include <stdexcept>
#include <iostream>

int main()
{
	try {
		int port = 8888;
		yamc::Server server(port);
		server.start();
		std::cout << "Running on port " << port << "..." << std::endl;
		std::cout << "Press ENTER to stop." << std::endl;
		std::cin.ignore();
		std::cout << "Shutting down..." << std::endl;
		server.stop();
		std::cout << "Server is stopped." << std::endl;
	}

	catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
	}
	return 0;
}