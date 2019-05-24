#pragma once

#include <chrono>

/*
** This header file is intended to house any miscellaneous resources needed in
** the project.
*/

// Not needed in client but many server-side objects need access to this def.
#define TICKS_PER_SEC 90  // 33.3 ms per update() loop

#define PORTNUM "4000"

// Map is a square so this is the same as height
#define MAP_WIDTH 48

// Width and height of walls and fences
#define FENCE_WIDTH 0.2f
#define FENCE_HEIGHT 3.0f

// Pregame countdown
const std::chrono::seconds PREGAME_LENGTH(6);

// Max game length
const std::chrono::seconds MAX_GAME_LENGTH(30);

// Back to lobby countdown
const std::chrono::seconds POSTGAME_LENGTH(15);

// Dog-specific stats
#define MAX_DOG_STAMINA 3.0f;	// Dog can sprint for three seconds max

// This is absolutely filthy code but it is necessary when multiple machines
// enter the picture.
enum EntityType
{
	ENTITY_STATE,	// Not really an entity but needed to differentiate GameState
	ENTITY_FLOOR,
    ENTITY_EXAMPLE,
	ENTITY_DOG,
	ENTITY_HUMAN,
	ENTITY_BOX,
	ENTITY_WALL,
	ENTITY_JAIL,
	ENTITY_HOUSE_6X6_A,
	ENTITY_BONE,
	ENTITY_DOGHOUSE,
	ENTITY_HYDRANT,
	ENTITY_FOUNTAIN,
	ENTITY_PUDDLE,
	ENTITY_FENCE,
	ENTITY_BAR,
	ENTITY_GATE,
	ENTITY_TRIGGER
	// TODO: add new types here, e.g. ENTITY_DOGBONE
};

enum FloorType
{
	FLOOR_GRASS,
	FLOOR_ROAD,
	FLOOR_DIRT
};

// Types of narrow-phase colliders
enum ColliderType
{
	COLLIDER_NONE,
	COLLIDER_AABB,
	COLLIDER_CAPSULE,
	COLLIDER_GATE
};

// Ensure animations are in the same order as their corresponding text files!

// Animations for humans
enum HumanAnimation
{
	ANIMATION_HUMAN_IDLE,
	ANIMATION_HUMAN_RUNNING,
	ANIMATION_HUMAN_CATCHING,
	ANIMATION_HUMAN_SWINGING2,
	ANIMATION_HUMAN_SWINGING2_IDLE,
	ANIMATION_HUMAN_SHOOT,
	ANIMATION_HUMAN_SHOOT_IDLE_LAUNCHER,
	ANIMATION_HUMAN_FLYING,
	ANIMATION_HUMAN_SLIPPING,
	ANIMATION_HUMAN_SLIPPING_IDLE,
	ANIMATION_HUMAN_SWINGING1,
	ANIMATION_HUMAN_SWINGING1_IDLE,
	ANIMATION_HUMAN_SWINGING3,
	ANIMATION_HUMAN_SWINGING3_IDLE
};

// Animations for dogs
enum DogAnimation
{
	ANIMATION_DOG_IDLE,
	ANIMATION_DOG_RUNNING,
	ANIMATION_DOG_PEEING,
	ANIMATION_DOG_SCRATCHING,
	ANIMATION_DOG_DRINKING,
	ANIMATION_DOG_DIGGING_IN,
	ANIMATION_DOG_DIGGING_IN_DIGGING_OUTG,
	ANIMATION_DOG_DIGGING_OUT,
	ANIMATION_DOG_IDLE_RUNNING
};
