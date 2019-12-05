#ifndef YAMC_GAME_VIEW_H
#define YAMC_GAME_VIEW_H

#include "Camera.h"
#include "Game.h"
#include "View.h"
#include "Mesh.h"
#include "Entities.h"

namespace yamc
{
	class GameView : public View
	{
	public:
		GameView(Application* application, Game* game);

		void init() override;
		void update(float dt) override;
		void render(Renderer* renderer) override;
		void scroll(double delta) override;
		void onMouseClick(int button, int mods) override;
		void destroy() override;

		~GameView() override;

	private:
		static const float MoveSpeed;
		static const float RunSpeedMultiplier;
		static const float MouseSensitivity;
		static const float Gravity;
		static const float JumpVelocity;

		Game* game;
		Entity player;
		Camera camera;
		BlockSelection currentBlockSelection;
		Inventory inventory;
		bool isCursorLocked;

		BlockSelection findBlockSelection(const glm::vec3& start, const glm::vec3& direction, float maxDistance) const;
		void updateMouseInput(float dt);
		void updateMoveKeys(float dt);
	};
}

#endif