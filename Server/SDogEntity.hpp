#pragma once

#include "SPlayerEntity.hpp"

class SDogEntity : public SPlayerEntity
{
public:
	SDogEntity(uint32_t playerId, std::vector<glm::vec2>* jails) : SPlayerEntity(playerId)
	{
		_state->type = ENTITY_DOG;

		_state->scale = glm::vec3(1);

		// Collider stuff
		_state->width = 0.8f;
		_state->height = 0.8f;
		_state->depth = 0.8f;

		_velocity = 5.0f;

		_jails = jails;
	};

	~SDogEntity() {};

	bool isCaught = false;

	// Dog getting caught is handled by the dog, not the human
	void handleCollision(std::shared_ptr<SBaseEntity> entity) override
	{
		if (entity->getState()->type == ENTITY_HUMAN)
		{
			Logger::getInstance()->debug("Sending dog to jail");
			isCaught = true;
			
			// Choose a random jail
			unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count();
			std::shuffle(_jails->begin(), _jails->end(), std::default_random_engine(seed));
			glm::vec2 jailPos = (*_jails)[0];
			getState()->pos = glm::vec3(jailPos.x, 0, jailPos.y);
		}
		else
		{
			// Otherwise, handle collisions as usual
			SBaseEntity::handleCollision(entity);
		}
	}

private:
	// List of jails the dog could potentially be sent to
	std::vector<glm::vec2>* _jails;
};

