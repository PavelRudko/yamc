#ifndef YAMC_APPLICATION_H
#define YAMC_APPLICATION_H

#include "Renderer.h"

struct GLFWwindow;

namespace yamc
{
	struct Settings
	{
		int initialScreenWidth;
		int initialScreenHeight;
		int visibleChunkRadius;
	};

	class View;

	class Application
	{
	public:
		Application();
		void run();
		Settings* getSettings();
		void saveSettings();
		uint32_t getCurrentFPS() const;
		const GLFWwindow* getWindow() const;
		GLFWwindow* getWindow();
		int getWindowWidth() const;
		int getWindowHeight() const;
		~Application();

	private:
		int windowWidth;
		int windowHeight;
		Renderer* renderer;
		View* currentView;
		GLFWwindow* window;
		Settings settings;
		uint32_t fps;

		void loadSettings();
		void loadInitialView();
		static void windowSizeCallback(GLFWwindow* window, int width, int height);
		static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
		static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	};

}

#endif