#ifndef YAMC_GAME_H
#define YAMC_GAME_H

#include <gl3w.h>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "World.h"
#include "Renderer.h"
#include "Mesh.h"
#include <GLFW/glfw3.h>

namespace yamc
{
	class Game
	{
	public:
		Game(GLFWwindow* window);
		Game(const Game&) = delete;
		Game(Game&& other) = delete;
		void update(float dt);
		void render();
		void setFPS(int value);
		void onWindowResize(int width, int height);
		void onScroll(double delta);
		void onExit();
		~Game();

	private:
		GLFWwindow* window;
		int windowWidth;
		int windowHeight;
		int fps;
		
		World world;
		Entity& player;
		Camera camera;
		Renderer renderer;
		BlockSelection currentBlockSelection;
		Inventory inventory;
		
		bool isCursorLocked;
		bool wasLeftMouseButtonPressed;
		bool wasRightMouseButtonPressed;
		float mouseSensitivity;
		float moveSpeed;

		BlockSelection findBlockSelection(const glm::vec3& start, const glm::vec3& direction, float maxDistance) const;
		void updateMouseInput(float dt);
		void updateMoveKeys(float dt);
	};
}

#endif