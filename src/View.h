#ifndef YAMC_VIEW_H
#define YAMC_VIEW_H

#include "Renderer.h"

namespace yamc
{
	class Application;

	class View
	{
	public:
		View(Application* application);
		virtual void init() = 0;
		virtual void update(float dt) = 0;
		virtual void render(Renderer* renderer) = 0;
		virtual void scroll(double delta);
		virtual void destroy() = 0;
		virtual ~View();

	protected:
		Application* application;
	};
}

#endif