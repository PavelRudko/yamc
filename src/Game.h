#ifndef YAMC_GAME_H
#define YAMC_GAME_H

#include <gl3w.h>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "World.h"
#include "View.h"
#include "Mesh.h"
#include <GLFW/glfw3.h>

namespace yamc
{
	class Game : public View
	{
	public:
		Game(Application* window);

		void init() override;
		void update(float dt) override;
		void render(Renderer* renderer) override;
		void scroll(double delta) override;
		void destroy() override;

		~Game() override;

	private:	
		World world;
		Entity& player;
		Camera camera;
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
		void rebuildChunkMeshes(Renderer* renderer);
	};
}

#endif