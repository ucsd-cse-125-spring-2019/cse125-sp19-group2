#pragma once

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
	ENTITY_TRIGGER,
	ENTITY_CYLINDER
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

// Actions of human
enum HumanAction
{
	ACTION_HUMAN_IDLE,
	ACTION_HUMAN_MOVING,
	ACTION_HUMAN_SHOOTING,
	ACTION_HUMAN_SWINGING
};

/*
shooting: hold button to play shoot animation, end shoot animation and shoot out plunger, play idle-launcher animation until plunger stop, play fly animation and move until hit wall or release
swing: hold button to charge up (dont affect movement), swing and move forward when release
slipping: stop moving and play slip animation
*/

// Actions of dog
enum DogAction
{
	ACTION_DOG_IDLE,
	ACTION_DOG_MOVING,
	ACTION_DOG_PEEING,
	ACTION_DOG_DRINKING,
	ACTION_DOG_SCRATCHING
};

/*
speed up: hold shift change speed higher until release (do nothing when not running)
scratching: walk near trigger, start scratching animation and start logic until release button
dog house: stop moving and start dig_in animation, end dig_in animation and teleport, stop moving and play dig_out animation
peeing: start peeing animation and create puddle, last 2 seconds or release button
drinking: walk near fountain, start drinking animation and start logic until release button
*/

// Ensure animations are in the same order as their corresponding text files!

// Animations for humans
enum HumanAnimation
{
	ANIMATION_HUMAN_IDLE,
	ANIMATION_HUMAN_RUNNING,
	ANIMATION_HUMAN_SWINGING2,
	ANIMATION_HUMAN_SWINGING2_IDLE, // transition
	ANIMATION_HUMAN_SHOOT,
	ANIMATION_HUMAN_SHOOT_IDLE_LAUNCHER,
	ANIMATION_HUMAN_IDLE_LAUNCHER,
	ANIMATION_HUMAN_FLYING,
	ANIMATION_HUMAN_SLIPPING,
	ANIMATION_HUMAN_SLIPPING_IDLE, // transition
	ANIMATION_HUMAN_SWINGING1,
	ANIMATION_HUMAN_SWINGING1_IDLE, // transition
	ANIMATION_HUMAN_SWINGING3,
	ANIMATION_HUMAN_SWINGING3_IDLE // transition
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
	ANIMATION_DOG_DIGGING_IN_DIGGING_OUT, // transition
	ANIMATION_DOG_DIGGING_OUT,
	ANIMATION_DOG_IDLE_RUNNING // transition
};

//Logger::getInstance()->debug("x: " + std::to_string(dir.x) + " y: " + std::to_string(dir.y) + " z: " + std::to_string(dir.z));