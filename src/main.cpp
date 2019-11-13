#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <thread>
#include "Game.h"

using namespace yamc;

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	auto game = (Game*)glfwGetWindowUserPointer(window);
	game->onWindowResize(width, height);
}

void run(GLFWwindow* window)
{
	glfwSwapInterval(0);

	Game game(window);
	glfwSetWindowUserPointer(window, &game);
	glfwSetFramebufferSizeCallback(window, windowSizeCallback);

	long long maxFPS = 1000;
	long long minFrameNanoseconds = 1000000000 / maxFPS;

	auto lastTime = std::chrono::high_resolution_clock::now();
	long long nanosecondsSinceFPSUpdate = 0;
	int frameCounter = 0;
	while (!glfwWindowShouldClose(window)) {
		auto currentTime = std::chrono::high_resolution_clock::now();
		auto elapsedNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();
		double elapsedSeconds = elapsedNanoseconds / 1000000000.0;

		glfwPollEvents();
		game.update(elapsedSeconds);
		game.render();
		glfwSwapBuffers(window);

		frameCounter++;
		nanosecondsSinceFPSUpdate += elapsedNanoseconds;
		if (nanosecondsSinceFPSUpdate > 1000000000) {
			nanosecondsSinceFPSUpdate -= 1000000000;
			game.setFPS(frameCounter);
			frameCounter = 0;
		}

		lastTime = currentTime;
		if (elapsedNanoseconds < minFrameNanoseconds) {
			std::this_thread::sleep_for(std::chrono::nanoseconds(minFrameNanoseconds - elapsedNanoseconds));
		}
	}

	game.onExit();
}

int main()
{
	if (!glfwInit()) {
		std::cerr << "Cannot initialize GLFW." << std::endl;
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

	auto window = glfwCreateWindow(800, 600, "YAMC", nullptr, nullptr);
	
	if (!window) {
		std::cerr << "Cannot initialize GLFW Window." << std::endl;
		return -1;
	}

	glfwShowWindow(window);
	glfwMakeContextCurrent(window);

	if (gl3wInit() || !gl3wIsSupported(3, 2)) {
		std::cerr << "Cannot initialize GL3W." << std::endl;
		return -1;
	}

	try {
		run(window);
	}
	catch(std::runtime_error e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}