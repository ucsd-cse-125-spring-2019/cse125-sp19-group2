#pragma once

#include "SBoxEntity.hpp"
#include "SBarEntity.hpp"
#include "SGateEntity.hpp"
#include "STriggerEntity.hpp"
#include "SDogEntity.hpp"
#include "EmptyCollider.hpp"
#include "SBoxPlungerEntity.hpp"
#include <algorithm>

#define JAIL_WALL_WIDTH 0.15f
#define TRIGGER_WIDTH 0.4f
#define TRIGGER_HEIGHT 0.6f

#define GATE_OPEN_THRESHOLD 1.0f
#define GATE_MAX_HEIGHT 1.6f

#define GATE_LIFT_RATE 1.6f // 2 units per second
#define GATE_LOWER_RATE 0.8f // 1 units per second

class SJailEntity : public SBaseEntity
{
public:
	SJailEntity(
		glm::vec3 pos,
		glm::vec3 scale)
	{
		_state = std::make_shared<BaseState>();

		// Base defaults
		SBaseEntity::initState();
		_state->type = ENTITY_JAIL;

		_state->pos = pos;
		_state->scale = scale;

		// No collider on the jail object, only on its children
		_state->colliderType = COLLIDER_NONE;
		_collider = std::make_unique<EmptyCollider>();

		// Jails are not rendered at all
		_state->isSolid = false;

		float xScale = scale.x / 3;
		float zScale = scale.z / 3;

		// North left wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - xScale, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// North right wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + xScale, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// South left wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - xScale, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// South right wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + xScale, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale, scale.y, JAIL_WALL_WIDTH)
		));

		// West up wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z - zScale + JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
		));

		// West down wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z + zScale - JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
			));

		// East up wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z - zScale + JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
			));

		// East down wall
		_children.push_back(std::make_shared<SBarEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z + zScale - JAIL_WALL_WIDTH / 2),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - JAIL_WALL_WIDTH)
		));

		// North gate
		auto northGate = std::make_shared<SGateEntity>(
			glm::vec3(pos.x, pos.y, pos.z + scale.z / 2 - JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale - 0.05, scale.y, JAIL_WALL_WIDTH)
		);
		// South gate
		auto southGate = std::make_shared<SGateEntity>(
			glm::vec3(pos.x, pos.y, pos.z - scale.z / 2 + JAIL_WALL_WIDTH / 2),
			glm::vec3(xScale - 0.05, scale.y, JAIL_WALL_WIDTH)
		);
		// West gate
		auto westGate = std::make_shared<SGateEntity>(
			glm::vec3(pos.x - scale.x / 2 + JAIL_WALL_WIDTH / 2, pos.y, pos.z),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - 0.05)
		);
		// East gate
		auto eastGate = std::make_shared<SGateEntity>(
			glm::vec3(pos.x + scale.x / 2 - JAIL_WALL_WIDTH / 2, pos.y, pos.z),
			glm::vec3(JAIL_WALL_WIDTH, scale.y, zScale - 0.05)
		);

		// solidity function for gates
		auto solidFunc = [&](BaseState* entity, BaseState* collidingEntity)
		{
			if (collidingEntity->type == ENTITY_DOG &&
				entity->pos.y >= GATE_OPEN_THRESHOLD) {
				return false;
			}
			return true;
		};

		_gates.push_back(northGate);
		_children.push_back(northGate);
		_gates.push_back(southGate);
		_children.push_back(southGate);
		_gates.push_back(westGate);
		_children.push_back(westGate);
		_gates.push_back(eastGate);
		_children.push_back(eastGate);

		// set solidity function for all gates
		for (auto gate : _gates) {
			gate->getState()->setSolidity(solidFunc);
		}

		// Collision handling function for trigger
		auto collisionFunc = [&](SBaseEntity* entity, SBaseEntity* collidingEntity)
		{
			if (collidingEntity->getState()->type == ENTITY_DOG)
			{
				SDogEntity* collidingDog = static_cast<SDogEntity*>(collidingEntity);
				collidingDog->setNearTrigger(true);
				// sending position of trigger to dogEntity for interpolate
				collidingDog->targetPos = entity->getState()->pos;
				
				// start lifting the gate in Stage 1
				if (collidingDog->isLifting() && collidingDog->actionStage == 1) {
					_lifted = true;
				}
			}
		};

		// Sensor for trigger
		auto northSensorBox = std::make_shared<STriggerEntity>(
			glm::vec3(pos.x + xScale / 2 + TRIGGER_WIDTH / 2, TRIGGER_HEIGHT, pos.z + scale.z / 2 + TRIGGER_WIDTH / 2),
			glm::vec3(TRIGGER_WIDTH, 0.5f, TRIGGER_WIDTH),
			glm::vec3(0, 0, 1)
		);
		northSensorBox->getState()->isSolid = false;
		northSensorBox->onCollision(collisionFunc);
		_children.push_back(northSensorBox);
		_triggers.push_back(northSensorBox);

		auto southSensorBox = std::make_shared<STriggerEntity>(
			glm::vec3(pos.x - xScale / 2 - TRIGGER_WIDTH / 2, TRIGGER_HEIGHT, pos.z - scale.z / 2 - TRIGGER_WIDTH / 2),
			glm::vec3(TRIGGER_WIDTH, 0.5f, TRIGGER_WIDTH),
			glm::vec3(0, 0, -1)
		);
		southSensorBox->getState()->isSolid = false;
		southSensorBox->onCollision(collisionFunc);
		_children.push_back(southSensorBox);
		_triggers.push_back(southSensorBox);

		auto eastSensorBox = std::make_shared<STriggerEntity>(
			glm::vec3(pos.x + scale.x / 2 + TRIGGER_WIDTH / 2, TRIGGER_HEIGHT, pos.z - zScale /2 - TRIGGER_WIDTH / 2),
			glm::vec3(TRIGGER_WIDTH, 0.5f, TRIGGER_WIDTH),
			glm::vec3(-1, 0, 0)
		);
		eastSensorBox->getState()->isSolid = false;
		eastSensorBox->onCollision(collisionFunc);
		_children.push_back(eastSensorBox);
		_triggers.push_back(eastSensorBox);

		auto westSensorBox = std::make_shared<STriggerEntity>(
			glm::vec3(pos.x - scale.x / 2 - TRIGGER_WIDTH / 2, TRIGGER_HEIGHT, pos.z + zScale / 2 + TRIGGER_WIDTH / 2),
			glm::vec3(TRIGGER_WIDTH, 0.5f, TRIGGER_WIDTH),
			glm::vec3(1, 0, 0)
		);
		westSensorBox->getState()->isSolid = false;
		westSensorBox->onCollision(collisionFunc);
		_children.push_back(westSensorBox);
		_triggers.push_back(westSensorBox);


		// sensor for checking if dog is in jail
		auto jailSensorBox = std::make_shared<SBoxEntity>(
			glm::vec3(pos.x, pos.y, pos.z),
			glm::vec3(scale.x - 0.9f, 0.5f, scale.z - 0.9f)
		);
		jailSensorBox->getState()->transparency = 0.0f;
		jailSensorBox->getState()->isSolid = false;
		jailSensorBox->onCollision([&](SBaseEntity* entity, SBaseEntity* collidingEntity)
		{
			if (collidingEntity->getState()->type == ENTITY_DOG)
			{
				SDogEntity* collidingDog = static_cast<SDogEntity*>(collidingEntity);
				collidingDog->isCaught = true;
			}
		});
		_children.push_back(jailSensorBox);

		_children.push_back(std::make_shared<SBoxPlungerEntity>(pos, scale));
	};
	~SJailEntity() {};

	// If walls have children in the future, change this
	std::vector<std::shared_ptr<SBaseEntity>> getChildren() override
	{
		return _children;
	}

	virtual void update(std::vector<std::shared_ptr<GameEvent>> events) override
	{
		if (_lifted) {
			if (gateHeight < GATE_MAX_HEIGHT) {
				gateHeight += GATE_LIFT_RATE / TICKS_PER_SEC;
				for (int i = 0; i < _gates.size(); i++) {
					_gates[i]->getState()->pos.y = gateHeight;
					_gates[i]->hasChanged = true;
				}
			}
			for (int i = 0; i < _triggers.size(); i++) {
				_triggers[i]->updateForward(4);
			}
		}
		else {
			if (gateHeight > 0) {
				gateHeight -= GATE_LOWER_RATE / TICKS_PER_SEC;
				for (int i = 0; i < _gates.size(); i++) {
					_gates[i]->getState()->pos.y = gateHeight;
					_gates[i]->hasChanged = true;
				}
				for (int i = 0; i < _triggers.size(); i++) {
					_triggers[i]->updateForward(-2);
				}
			}
			else {
				for (int i = 0; i < _gates.size(); i++) {
					_gates[i]->hasChanged = false;
				}
				for (int i = 0; i < _triggers.size(); i++) {
					_triggers[i]->hasChanged = false;
				}
			}
		}

		_lifted = false;
	}

private:
	std::vector<std::shared_ptr<SBaseEntity>> _children;
	std::vector<std::shared_ptr<SGateEntity>> _gates;
	std::vector<std::shared_ptr<STriggerEntity>> _triggers;
	bool _lifted = false;
	float gateHeight = 0;
};

