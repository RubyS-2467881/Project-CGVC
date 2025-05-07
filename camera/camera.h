#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cameraDirection.h>

class Camera {
public: 
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	float yaw;
	float pitch;

	float movementSpeed;
	float mouseSensitivity;

	Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch) : front(glm::vec3(0.0f, 0.0f, -1.0f)), movementSpeed(2.5f), mouseSensitivity(0.1f) {
		position = startPosition;
		worldUp = startUp;
		yaw = startYaw;
		pitch = startPitch;
		updateCameraVectors();
	}

	void updateCameraVectors() {
		glm::vec3 startFront;

		startFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		startFront.y = sin(glm::radians(pitch));
		startFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		front = glm::normalize(startFront);
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

	void processKeyboardInput(CameraDirection direction, float deltaTime) {
		float velocity = movementSpeed * deltaTime;

		if (direction == FORWARD)
			position += front * velocity;
		if (direction == BACKWARD)
			position -= front * velocity;
		if (direction == LEFT)
			position -= right * velocity;
		if (direction == RIGHT)
			position += right * velocity;
		if (direction == UP)
			position += up * velocity;
		if (direction == DOWN)
			position -= up * velocity;
	}

	glm::mat4 viewMatrix() {
		return glm::lookAt(position, position + front, up);
	}

	void processMouseInput(float xOffset, float yOffset) {
		xOffset *= mouseSensitivity;
		yOffset *= mouseSensitivity;

		yaw += xOffset;
		pitch += yOffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		updateCameraVectors();
	}
};



#endif