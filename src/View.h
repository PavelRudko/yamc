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
		virtual void onMouseClick(int button, int mods);
		virtual void onMouseRelease(int button, int mods);
		virtual void destroy() = 0;
		virtual ~View();

	protected:
		Application* application;

		bool isKeyPressed(int key) const;
	};
}

#endif