
#ifndef ENTITY_H
#define ENTITY_H

#include <lua.h>
#include "common.h"

typedef enum entity_childType_e {
	entity_childType_none,
	entity_childType_entity,
	entity_childType_model
} entity_childType_t;

typedef struct {
	// Children are specified by index and type.
	int *children;
	int children_length;
	entity_childType_t childType;
	vec3_t position;
	quat_t orientation;
} entity_t;

typedef struct {
	entity_t *entities;
	int entities_length;
	/* entities_length_allocated always equals entities_length +
	   deletedEntities_length, so it is not really needed. */
	// int entities_length_allocated;
	int *deletedEntities;
	int deletedEntities_length;
	int deletedEntities_length_allocated;
} entityList_t;

extern entityList_t g_entityList;

void entity_initEntityList(void);
void entity_freeEntityList(void);
int entity_createEntity(int *index, entity_childType_t type);

int l_entity_createEntity(lua_State *luaState);
int l_entity_linkChild(lua_State *luaState);
int l_entity_setPosition(lua_State *luaState);
int l_entity_setOrientation(lua_State *luaState);

#endif
