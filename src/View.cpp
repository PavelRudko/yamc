#include "View.h"

#include "Application.h"
#include <GLFW/glfw3.h>

namespace yamc
{
	View::View(Application* application) :
		application(application)
	{
	}

	void View::scroll(double delta)
	{
	}

	void View::onMouseClick(int button, int mods)
	{
	}

	void View::onMouseRelease(int button, int mods)
	{
	}

	bool View::isKeyPressed(int key) const
	{
		return glfwGetKey(application->getWindow(), key) == GLFW_PRESS;
	}

	View::~View()
	{
	}
	
}