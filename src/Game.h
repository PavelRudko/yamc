#ifndef YAMC_GAME_H
#define YAMC_GAME_H

#include "Camera.h"
#include "World.h"
#include "View.h"
#include "Mesh.h"

namespace yamc
{
	class Game : public View
	{
	public:
		Game(Application* window, const std::string& worldName, int worldSeed);

		void init() override;
		void update(float dt) override;
		void render(Renderer* renderer) override;
		void scroll(double delta) override;
		void onMouseClick(int button, int mods) override;
		void destroy() override;

		~Game() override;

	private:
		static const float MoveSpeed;
		static const float RunSpeedMultiplier;
		static const float MouseSensitivity;
		static const float Gravity;
		static const float JumpVelocity;

		World world;
		Entity& player;
		Camera camera;
		BlockSelection currentBlockSelection;
		Inventory inventory;
		bool isCursorLocked;

		BlockSelection findBlockSelection(const glm::vec3& start, const glm::vec3& direction, float maxDistance) const;
		void updateMouseInput(float dt);
		void updateMoveKeys(float dt);
		void rebuildChunkMeshes(Renderer* renderer);
	};
}

#endif