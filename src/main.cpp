#include "Application.h"
#include <stdexcept>
#include <iostream>

int main()
{
	yamc::Application application;

	try {
		application.run();
	}
	catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	return 0;
}