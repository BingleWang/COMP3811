#pragma once

#include <glm/glm.hpp>

class Particle {
public:
	Particle(const glm::vec3& position, const glm::vec3& velocity = glm::vec3(0.0f, 0.0f, 0.0f), float gravity = -0.98f, float gravityEffect = 1.0f, float lifeTime = 20.0f, float rotation = 0.0f, float scale = 1.0f) {
		this->position = position;
		this->velocity = velocity;
		this->gravity = gravity;
		this->gravityEffect = gravityEffect;

		this->lifeTime = lifeTime;
		this->rotation = rotation;
		this->scale = scale;
	}

	glm::vec3 getPosition() const {
		return position;
	}

	float getRotation() const {
		return rotation;
	}

	float getScale() const {
		return scale;
	}

	bool update(float deltaTime) {
		velocity.y = gravity * gravityEffect * deltaTime;

		position += velocity;
		elapsedTime += deltaTime;
		rotation += deltaTime * 5.0f;

		return ((elapsedTime < lifeTime) || (position.y > -1.6f));
	}

private:
	glm::vec3 position;
	glm::vec3 velocity;

	float gravity = -0.98f;
	float gravityEffect = 1.0f;
	float lifeTime = 20.0f;
	float rotation = 0.0f;
	float scale = 1.0f;
	float elapsedTime = 0.0f;
};