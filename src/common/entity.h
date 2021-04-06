
#ifndef ENTITY_H
#define ENTITY_H

#include "types.h"

extern entityList_t g_entityList;

void entity_initEntityList(void);
void entity_freeEntityList(void);
int entity_createEntity(int *index, entity_childType_t type);
void entity_printEntity(int index);

int l_entity_createEntity(lua_State *luaState);
int l_entity_linkChild(lua_State *luaState);
int l_entity_setPosition(lua_State *luaState);
int l_entity_setOrientation(lua_State *luaState);

#endif
