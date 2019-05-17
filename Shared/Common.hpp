#pragma once

/*
** This header file is intended to house any miscellaneous resources needed in
** the project.
*/

// Not needed in client but many server-side objects need access to this def.
#define TICKS_PER_SEC 30  // 33.3 ms per update() loop

#define PORTNUM "4000"

// Map is a square so this is the same as height
#define MAP_WIDTH 48

// Width and height of walls and fences
#define WALL_WIDTH 0.2f
#define WALL_HEIGHT 3.0f

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
	ENTITY_BAR
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
	COLLIDER_CAPSULE
};

// Ensure animations are in the same order as their corresponding text files!

// Animations for humans
enum HumanAnimation
{
	ANIMATION_HUMAN_IDLE, // Index 0
	ANIMATION_HUMAN_RUNNING, // Index 1
	ANIMATION_HUMAN_CATCHING // Index 2
};

// Animations for dogs
enum DogAnimation
{
	ANIMATION_DOG_IDLE, // Index 0
	ANIMATION_DOG_RUNNING // Index 1
};
