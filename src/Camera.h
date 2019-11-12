#ifndef YAMC_CAMERA_H
#define YAMC_CAMERA_H

#include <glm/glm.hpp>

namespace yamc
{
	class Camera
	{
	public:
		Camera();

		void setPosition(const glm::vec3& value);
		void addPitch(float delta);
		void addYaw(float delta);
		void moveForward(float delta);
		void moveSide(float delta);
		glm::vec3 getPosition() const;
		glm::vec3 getLookDirection();
		glm::mat4 getViewMatrix();

	private:
		float pitch;
		float yaw;
		glm::vec3 position;
	};
}

#endif