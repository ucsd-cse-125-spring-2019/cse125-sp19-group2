#pragma once

#include <chrono>
#include <string>
#include <vector>

/*
** This header file is intended to house any miscellaneous resources needed in
** the project.
*/

// Not needed in client but many server-side objects need access to this def.
#define TICKS_PER_SEC 90 // 33.3 ms per update() loop

#define PORTNUM "4000"

// Map is a square so this is the same as height
#define MAP_WIDTH 72
#define MAP_BLEND_NUM 10.0f
#define MAP_BLEND_DIST 7

// Width and height of walls and fences
#define FENCE_WIDTH 0.2f
#define FENCE_HEIGHT 3.0f

// Loading screen minimum time
const std::chrono::seconds LOADING_LENGTH(10);

// Pregame countdown
const std::chrono::seconds PREGAME_LENGTH(5);

// Max game length
const std::chrono::seconds MAX_GAME_LENGTH(90);

// Back to lobby countdown
const std::chrono::seconds POSTGAME_LENGTH(15);

// Plunger cooldown
const std::chrono::seconds PLUNGER_COOLDOWN(5);

// Dogbone trap cooldown
const std::chrono::seconds TRAP_COOLDOWN(10);

// Dog-specific stats
#define DOG_SKIN_AMOUNT 3
#define DOG_BASE_VELOCITY 5.5f
#define DOG_RUN_VELOCITY DOG_BASE_VELOCITY * 1.5f
#define DOG_TELEPORT_VELOCITY DOG_BASE_VELOCITY * 5.0f
#define MAX_DOG_STAMINA 3.0f			// Dog can sprint for three seconds max
#define MAX_DOG_URINE 1.0f				// Dog can make one puddle per urine bar
#define MAX_DOG_ESCAPE_PRESSES 12 // Dog must press button 12 times to escape trap

// Human-spedific stats
#define HUMAN_SKIN_AMOUNT 3
#define HUMAN_BASE_VELOCITY 6.0f
#define HUMAN_SWING_VELOCITY 11.0f
#define MAX_HUMAN_CHARGE 2.0f
#define HUMAN_CHARGE_THRESHOLD1 0.5f // lower than that will be swing1
#define HUMAN_CHARGE_THRESHOLD2 1.5f // lower than that will be swing2
#define LAUNCHING_VELOCITY 25.0f		 // Plunger speed
#define HUMAN_FLY_VELOCITY 30.0f

#define HOUSE_SKIN_AMOUNT 4

#define COMPASS_SIZE 200
#define COMPASS_POS_OFFSET 110

#define TOOLTIP_SIZE 250
#define TOOLTIP_OFFSET 125

#define DOGPOINTER_SIZE 75
#define DOGPOINTER_RADIUS_RATIO 0.75f
#define DOGPOINTER_BORDER_OFFSET 100

#define GRASS_SEED 100
#define PEBBLE_SEED 101
#define DIRT_PEBBLE_SEED 102


// This is absolutely filthy code but it is necessary when multiple machines
// enter the picture.
enum EntityType
{
	ENTITY_STATE, // Not really an entity but needed to differentiate GameState
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
	ENTITY_CYLINDER,
	ENTITY_HIT_PLUNGER,
	ENTITY_PLUNGER,
	ENTITY_ROPE,
	ENTITY_TRAP,
	ENTITY_NET,
	ENTITY_TREE,
	ENTITY_GRASS
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
	ACTION_HUMAN_SLIPPING,
	ACTION_HUMAN_LAUNCHING,
	ACTION_HUMAN_SWINGING,
	ACTION_HUMAN_PLACING_TRAP
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
	ACTION_DOG_SCRATCHING,
	ACTION_DOG_TRAPPED,
	ACTION_DOG_JAILED,		 // Teleporting to jail
	ACTION_DOG_TELEPORTING // Teleporting between doghouses
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
	ANIMATION_HUMAN_SHOOT,
	ANIMATION_HUMAN_IDLE_LAUNCHER,
	ANIMATION_HUMAN_FLYING,
	ANIMATION_HUMAN_SLIPPING,
	ANIMATION_HUMAN_SWINGING1,
	ANIMATION_HUMAN_SWINGING3,
	ANIMATION_HUMAN_PLACING
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
	ANIMATION_DOG_DIGGING_OUT,
	ANIMATION_DOG_WALKING,
	ANIMATION_DOG_EATING
};

// Tooltip images to be displayed to players
enum PlayerTooltip
{
	TOOLTIP_NONE,
	TOOLTIP_DRINK,
	TOOLTIP_JAIL,
	TOOLTIP_TRAPPED
};

#ifndef ANIMATIONSTR
#define ANIMATIONSTR
inline std::string humanAnimations[] = {
		"idle",
		"running",
		"swinging2",
		"shoot",
		"idleLauncher",
		"flying",
		"slipping",
		"swinging1",
		"swinging3",
		"placing"};

inline std::string dogAnimations[] = {
		"idle",
		"running",
		"peeing",
		"scratching",
		"drinking",
		"diggingIn",
		"diggingOut",
		"walking",
		"eating"};
#endif

//Logger::getInstance()->debug("x: " + std::to_string(dir.x) + " y: " + std::to_string(dir.y) + " z: " + std::to_string(dir.z));
