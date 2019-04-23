#pragma once

/*
** This header file is intended to house any miscellaneous resources needed in
** the project.
*/

#define PORTNUM "4000"

// Not needed in client but many server-side objects need access to this def.
#define TICKS_PER_SEC 30  // 33.3 ms per update() loop

// This is absolutely filthy code but it is necessary when multiple machines
// enter the picture.
enum EntityType
{
    ENTITY_EXAMPLE,
	ENTITY_PLAYER
	// TODO: add new types here, e.g. ENTITY_DOGBONE
};
