#include "Game.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include "Shader.h"

namespace yamc
{
	Game::Game(GLFWwindow* window) :
		window(window),
		isCursorLocked(true),
		wasLeftMouseButtonPressed(false),
		wasRightMouseButtonPressed(false),
		mouseSensitivity(0.002f),
		moveSpeed(3.0f),
		fps(0),
		world("test", 5105340),
		player(world.getPlayer())
	{
		currentBlockSelection.isSelected = false;

		glfwGetWindowSize(window, &windowWidth, &windowHeight);

		glfwSetCursorPos(window, windowWidth / 2, windowHeight / 2);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Game::update(float dt)
	{
		updateMouseInput(dt);
		updateMoveKeys(dt);
		if (!player.isGrounded) {
			player.velocity.y -= 7.5f * dt;
		}
		world.update(dt);

		camera.setPosition(player.boundingBox.center + glm::vec3(0, 0.6f, 0));
		currentBlockSelection = findBlockSelection(camera.getPosition(), camera.getLookDirection(), 4.0f);

		inventory.setItem(0, 1);
		inventory.setItem(1, 2);
		inventory.setItem(2, 3);
		inventory.setHotbarItem(0, 0);
		inventory.setHotbarItem(1, 1);
		inventory.setHotbarItem(2, 2);
	}

	int getMinAxis(const glm::vec3& dir)
	{
		auto absDir = glm::abs(dir);
		if (absDir.x < absDir.y && absDir.x < absDir.z) {
			return 0;
		}
		if (absDir.y < absDir.z) {
			return 1;
		}
		return 2;
	}

	float closestBoundDistance(float start, float direction)
	{
		return (direction > 0 ? ceilf(start) - start : start - floorf(start)) / abs(direction);
	}

	BlockSelection Game::findBlockSelection(const glm::vec3& start, const glm::vec3& direction, float maxDistance) const
	{
		BlockSelection selection;
		selection.isSelected = false;

		glm::ivec3 currentBlock = glm::floor(start);

		glm::ivec3 step;
		step[0] = direction[0] < 0 ? -1 : 1;
		step[1] = direction[1] < 0 ? -1 : 1;
		step[2] = direction[2] < 0 ? -1 : 1;

		glm::vec3 delta = (glm::vec3)step / direction;

		glm::vec3 tmax;
		tmax[0] = closestBoundDistance(start[0], direction[0]);
		tmax[1] = closestBoundDistance(start[1], direction[1]);
		tmax[2] = closestBoundDistance(start[2], direction[2]);

		while (true) {
			int axis = getMinAxis(tmax);
			if (tmax[axis] > maxDistance) {
				break;
			}

			tmax[axis] += delta[axis];
			currentBlock[axis] += step[axis];

			uint32_t block = world.getTerrain().getBlock(currentBlock.x, currentBlock.y, currentBlock.z);
			if (block > 0) {
				selection.isSelected = true;
				selection.coordinate = currentBlock;
				selection.normal = glm::ivec3(0, 0, 0);
				selection.normal[axis] = -step[axis];
				return selection;
			}
		}

		return selection;
	}

	void Game::updateMouseInput(float dt)
	{
		double windowCenterX = windowWidth / 2;
		double windowCenterY = windowHeight / 2;
		
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
			if (isCursorLocked) {
				if (currentBlockSelection.isSelected && !wasLeftMouseButtonPressed) {
					world.getTerrain().setBlock(currentBlockSelection.coordinate.x, currentBlockSelection.coordinate.y, currentBlockSelection.coordinate.z, 0);
				}
			}
			else {
				glfwSetCursorPos(window, windowCenterX, windowCenterY);
				isCursorLocked = true;
				glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			}
			wasLeftMouseButtonPressed = true;
		}
		else {
			wasLeftMouseButtonPressed = false;
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
			if (isCursorLocked) {
				if (currentBlockSelection.isSelected && !wasRightMouseButtonPressed) {
					auto block = currentBlockSelection.coordinate + currentBlockSelection.normal;

					AABB blockBoundingBox;
					blockBoundingBox.center = (glm::vec3)block + glm::vec3(0.5f, 0.5f, 0.5f);
					blockBoundingBox.halfSize = glm::vec3(0.5f, 0.5f, 0.5f);

					auto item = inventory.getHotbarItem(inventory.getSelectedHotbarSlot());

					if (!hasIntersection(player.boundingBox, blockBoundingBox) && item) {
						world.getTerrain().setBlock(block.x, block.y, block.z, item->id);
					}
				}
			}
			wasRightMouseButtonPressed = true;
		}
		else {
			wasRightMouseButtonPressed = false;
		}

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			isCursorLocked = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		if (isCursorLocked) {
			double mouseX, mouseY;
			glfwGetCursorPos(window, &mouseX, &mouseY);
			mouseX -= windowCenterX;
			mouseY -= windowCenterY;
			camera.addYaw(-mouseX * mouseSensitivity);
			camera.addPitch(-mouseY * mouseSensitivity);
			glfwSetCursorPos(window, windowCenterX, windowCenterY);
		}
	}

	void Game::updateMoveKeys(float dt)
	{
		float speedForward = 0;
		float speedSide = 0;

		if (glfwGetKey(window, GLFW_KEY_W)) {
			speedForward += 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_S)) {
			speedForward -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_A)) {
			speedSide -= 1.0f;
		}
		if (glfwGetKey(window, GLFW_KEY_D)) {
			speedSide += 1.0f;
		}

		glm::vec3 velocity(0, 0, 0);
		auto direction = camera.getLookDirection();
		direction.y = 0;
		velocity += direction * speedForward;
		velocity += glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0))) * speedSide;

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)) {
			velocity *= 2.0f;
		}

		player.velocity.x = velocity.x * moveSpeed;
		player.velocity.z = velocity.z * moveSpeed;

		if (player.isGrounded && glfwGetKey(window, GLFW_KEY_SPACE)) {
			player.velocity.y += 6.0f;
		}
	}

	void Game::render()
	{
		glClearColor(0.8f, 0.9f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);

		auto perspectiveMatrix = glm::perspective(glm::radians(55.0f), (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f);
		auto viewMatrix = camera.getViewMatrix();

		renderer.renderTerrain(perspectiveMatrix, viewMatrix, world.getTerrain());
		if (currentBlockSelection.isSelected) {
			renderer.renderCubeOutline(perspectiveMatrix, viewMatrix, (glm::vec3)currentBlockSelection.coordinate + glm::vec3(0.5f, 0.5f, 0.5f));
		}

		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glFrontFace(GL_CW);
		auto guiProjectionMatrix = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1000.0f, 1000.0f);
		renderer.renderText(guiProjectionMatrix, "FPS:" + std::to_string(fps), glm::vec3(1, 1, 1),  glm::vec2(10, 10), 2);

		auto pos = player.boundingBox.center;
		renderer.renderText(guiProjectionMatrix, "X:" + std::to_string(pos.x) + ", Y:" + std::to_string(pos.y) + ", Z:" + std::to_string(pos.z), glm::vec3(1, 1, 1), glm::vec2(10, 30), 2);
		int chunksCount = world.getTerrain().getChunks().size();
		renderer.renderText(guiProjectionMatrix, "CHUNKS IN MEMORY:" + std::to_string(chunksCount), glm::vec3(1, 1, 1), glm::vec2(10, 50), 2);
		renderer.renderCross(guiProjectionMatrix, glm::vec2(windowWidth / 2, windowHeight / 2));
		renderer.renderInventoryHotbar(guiProjectionMatrix, glm::vec2(windowWidth / 2, windowHeight - 50), inventory);
	}

	void Game::setFPS(int value)
	{
		fps = value;
	}

	void Game::onWindowResize(int width, int height)
	{
		windowWidth = width;
		windowHeight = height;
		glViewport(0, 0, width, height);
	}

	void Game::onScroll(double delta)
	{
		inventory.scrollHotbar(delta < 0 ? 1 : -1);
	}

	void Game::onExit()
	{
		world.saveWorld();
	}

	Game::~Game()
	{
		
	}
}