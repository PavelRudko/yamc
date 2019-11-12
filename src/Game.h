#ifndef YAMC_GAME_H
#define YAMC_GAME_H

#include <gl3w.h>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Terrain.h"
#include "Renderer.h"
#include "Mesh.h"
#include <GLFW/glfw3.h>

namespace yamc
{
	struct AABB
	{
		glm::vec3 center;
		glm::vec3 halfSize;
	};

	struct Entity
	{
		AABB boundingBox;
		glm::vec3 velocity;
		bool isGrounded;
	};

	struct BlockSelection
	{
		bool isSelected;
		glm::ivec3 coordinate;
		glm::ivec3 normal;
	};

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
		~Game();

	private:
		GLFWwindow* window;
		int windowWidth;
		int windowHeight;
		int fps;
		
		Camera camera;
		Entity player;
		Terrain terrain;
		Renderer renderer;
		BlockSelection currentBlockSelection;
		
		bool isCursorLocked;
		bool wasLeftMouseButtonPressed;
		bool wasRightMouseButtonPressed;
		float mouseSensitivity;
		float moveSpeed;

		BlockSelection findBlockSelection(const glm::vec3& start, const glm::vec3& direction, float maxDistance) const;
		void updateMouseInput(float dt);
		void updateMoveKeys(float dt);
		void updateEntityPosition(Entity& entity, float dt);
	};
}

#endif