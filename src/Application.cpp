#include "Application.h"
#include <gl3w.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include "View.h"
#include "GameView.h"
#include <chrono>
#include <thread>
#include "SinglePlayerGame.h"
#include "MultiPlayerGame.h"

namespace yamc
{
	Application::Application()
	{
		loadSettings();
		currentView = nullptr;
		fps = 0;

		if (!glfwInit()) {
			throw std::runtime_error("Cannot initialize GLFW.");
		}

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(settings.initialScreenWidth, settings.initialScreenHeight, "YAMC", nullptr, nullptr);

		if (!window) {
			throw std::runtime_error("Cannot initialize GLFW Window.");
		}

		glfwShowWindow(window);
		glfwMakeContextCurrent(window);

		if (gl3wInit() || !gl3wIsSupported(3, 2)) {
			throw std::runtime_error("Cannot initialize GL3W.");
		}

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, windowSizeCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetMouseButtonCallback(window, mouseButtonCallback);
		glfwSwapInterval(0);
		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		renderer = new Renderer();

		loadInitialView();
	}

	void Application::run()
	{
		static long NanosecondsInSecond = 1000000000;
		static long long MaxFPS = 1000;
		static long long MinFrameNanoseconds = NanosecondsInSecond / MaxFPS;

		auto lastTime = std::chrono::high_resolution_clock::now();
		long long nanosecondsSinceFPSUpdate = 0;
		int frameCounter = 0;
		while (!glfwWindowShouldClose(window)) {
			auto currentTime = std::chrono::high_resolution_clock::now();
			auto elapsedNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - lastTime).count();
			double elapsedSeconds = elapsedNanoseconds / (double)NanosecondsInSecond;

			glfwPollEvents();
			currentView->update(elapsedSeconds);
			currentView->render(renderer);
			glfwSwapBuffers(window);

			frameCounter++;
			nanosecondsSinceFPSUpdate += elapsedNanoseconds;
			if (nanosecondsSinceFPSUpdate > NanosecondsInSecond) {
				nanosecondsSinceFPSUpdate -= NanosecondsInSecond;
				fps = frameCounter;
				frameCounter = 0;
			}

			lastTime = currentTime;
			if (elapsedNanoseconds < MinFrameNanoseconds) {
				std::this_thread::sleep_for(std::chrono::nanoseconds(MinFrameNanoseconds - elapsedNanoseconds));
			}
		}
	}

	Settings* Application::getSettings()
	{
		return &settings;
	}

	void Application::saveSettings()
	{
	}

	uint32_t Application::getCurrentFPS() const
	{
		return fps;
	}

	const GLFWwindow* Application::getWindow() const
	{
		return window;
	}

	GLFWwindow* Application::getWindow()
	{
		return window;
	}

	int Application::getWindowWidth() const
	{
		return windowWidth;
	}

	int Application::getWindowHeight() const
	{
		return windowHeight;
	}

	void Application::loadSettings()
	{
		settings.initialScreenHeight = 600;
		settings.initialScreenWidth = 800;
		settings.visibleChunkRadius = 4;
	}

	void Application::loadInitialView()
	{
		//auto game = new SinglePlayerGame(542917, "test", getSettings()->visibleChunkRadius);
		auto game = new MultiPlayerGame(getSettings()->visibleChunkRadius);
		currentView = new GameView(this, game);
		currentView->init();
	}

	Application::~Application()
	{
		currentView->destroy();
		delete currentView;

		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Application::windowSizeCallback(GLFWwindow* window, int width, int height)
	{
		auto application = (Application*)glfwGetWindowUserPointer(window);
		application->windowWidth = width;
		application->windowHeight = height;
		glViewport(0, 0, width, height);
	}

	void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto application = (Application*)glfwGetWindowUserPointer(window);
		application->currentView->scroll(yoffset);
	}

	void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		auto application = (Application*)glfwGetWindowUserPointer(window);

		if (action == GLFW_PRESS) {
			application->currentView->onMouseClick(button, mods);
		}
		else if (action == GLFW_RELEASE) {
			application->currentView->onMouseRelease(button, mods);
		}
	}
}
