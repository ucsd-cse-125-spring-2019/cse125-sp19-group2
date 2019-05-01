#pragma once

/*
** This header file is intended to house any miscellaneous resources needed in
** the project.
*/

#define PORTNUM "4000"

// Width and height of walls and fences
#define WALL_WIDTH 0.2f
#define WALL_HEIGHT 1.0f

// Map is a square so this is the same as height
#define MAP_WIDTH 10

// Not needed in client but many server-side objects need access to this def.
#define TICKS_PER_SEC 30  // 33.3 ms per update() loop

// This is absolutely filthy code but it is necessary when multiple machines
// enter the picture.
enum EntityType
{
    ENTITY_EXAMPLE,
	ENTITY_DOG,
	ENTITY_HUMAN,
	ENTITY_BOX,
	ENTITY_WALL
	// TODO: add new types here, e.g. ENTITY_DOGBONE
};

// Types of narrow-phase colliders
enum ColliderType
{
	COLLIDER_AABB,
	COLLIDER_CAPSULE
};
