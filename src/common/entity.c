
#include "entity.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"
#include "log.h"
#include "insane.h"
#include "obj.h"
#include "vector.h"
#ifdef CLIENT
#include "../client/material.h"
#endif

entityList_t g_entityList;

static void entity_initEntity(entity_t *entity) {
	memset(entity, 0, sizeof(entity_t));
	
	entity->children = NULL;
	entity->children_length = 0;
	entity->childType = entity_childType_none;
	for (int i = 0; i < sizeof(vec3_t)/sizeof(vec_t); i++) {
		entity->position[i] = 0;
		entity->orientation.v[i] = 0;
	}
	entity->orientation.s = 1;
	entity->inUse = true;
#ifdef CLIENT
	entity->shown = true;
	entity->materials = NULL;
	entity->materials_length = 0;
#endif
}

static void entity_freeEntity(entity_t *entity) {
	insane_free(entity->children);
	entity->children_length = 0;
#	ifdef CLIENT
	insane_free(entity->materials);
	entity->materials_length = 0;
#	endif
}

void entity_initEntityList(void) {
	int index;

	memset(&g_entityList, 0, sizeof(entityList_t));
	g_entityList.entities = NULL;
	g_entityList.entities_length = 0;
	g_entityList.deletedEntities = NULL;
	g_entityList.deletedEntities_length = 0;
	g_entityList.deletedEntities_length_allocated = 0;
	
	entity_createEntity(&index, entity_childType_entity);
}

void entity_freeEntityList(void) {
	for (int i = 0; i < g_entityList.entities_length; i++) {
		entity_freeEntity(&g_entityList.entities[i]);
	}
	insane_free(g_entityList.entities);
	g_entityList.entities_length = 0;
	insane_free(g_entityList.deletedEntities);
	g_entityList.deletedEntities_length = 0;
	g_entityList.deletedEntities_length_allocated = 0;
}

/* entity_isValidEntityIndex
index:i     The index of an entity.
Returns:    1 if index is valid, otherwise 0.
*/
static int entity_isValidEntityIndex(ptrdiff_t index) {
	
	// Out of bounds.
	if (index < 0) {
		return 0;
	}
	if (index >= g_entityList.entities_length) {
		return 0;
	}

	// Freed.
	for (ptrdiff_t i = 0; i < g_entityList.deletedEntities_length; i++) {
		if (index == g_entityList.deletedEntities[i]) {
			return 0;
		}
	}
	
	// Use this instead? Would be faster.
	// if (g_entityList.entities[index].inUse) return 1; else return 0;
	
	return 1;
}

int entity_createEntity(int *index, entity_childType_t type) {
	int error = ERR_CRITICAL;
	
	// Check if we can use a deleted entity.
	if (g_entityList.deletedEntities_length > 0) {
		// New entity is the entity that was most recently deleted.
		*index = g_entityList.deletedEntities[--g_entityList.deletedEntities_length];
	}
	// Create an entity from scratch.
	else {
		/* If the entity list is empty, that means that all slots in the entity
		   list have been used. Therefore, we have to allocate memory for the
		   new entity. */
		*index = g_entityList.entities_length;
		g_entityList.entities_length++;
		g_entityList.entities = realloc(g_entityList.entities, g_entityList.entities_length * sizeof(entity_t));
		if (g_entityList.entities == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
	}
	
	entity_initEntity(&g_entityList.entities[*index]);
	g_entityList.entities[*index].childType = type;
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int entity_deleteEntity(int index) {
	int error = ERR_CRITICAL;
	
	// Don't actually free it, just add it to the deleted list. Don't even clear it.
	
	// Use an entry that has already been allocated if possible.
	if (g_entityList.deletedEntities_length < g_entityList.deletedEntities_length_allocated) {
		g_entityList.deletedEntities_length++;
	}
	// Allocate a new spot.
	else {
		g_entityList.deletedEntities_length_allocated++;
		g_entityList.deletedEntities_length++;
		g_entityList.deletedEntities = realloc(g_entityList.deletedEntities, g_entityList.deletedEntities_length_allocated * sizeof(ptrdiff_t));
		if (g_entityList.deletedEntities == NULL) {
			outOfMemory();
			error = ERR_OUTOFMEMORY;
			goto cleanup_l;
		}
		// Yep. All that for a measly int.
	}
	
	// Unlink children and free list.
	insane_free(g_entityList.entities[index].children)
	g_entityList.entities[index].children_length = 0;
	// Easy!
	
	// And finally, save the index and mark as deleted.
	g_entityList.deletedEntities[g_entityList.deletedEntities_length - 1] = index;
	g_entityList.entities[index].inUse = false;
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

int entity_linkChild(ptrdiff_t parentIndex, ptrdiff_t childIndex) {
	int error = ERR_CRITICAL;
	
	entity_t *parent;
	
	parent = &g_entityList.entities[parentIndex];
	
	// Allocate index.
	parent->children_length++;
	parent->children = realloc(parent->children, parent->children_length * sizeof(ptrdiff_t));
	if (parent->children == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	// Link.
	parent->children[parent->children_length - 1] = childIndex;
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

/*
Unlinking is faster when the parent has few children.
*/
int entity_unlinkChild(ptrdiff_t parentIndex, ptrdiff_t childIndex) {
	int error = ERR_CRITICAL;
	
	entity_t *parent;
	ptrdiff_t childIndex_index;
	
	parent = &g_entityList.entities[parentIndex];
	
	if (parent->children_length > 0) {
		// Find child index.
		for (ptrdiff_t i = 0; i < parent->children_length; i++) {
			if (parent->children[i] == childIndex) {
				childIndex_index = i;
				break;
			}
		}
		
		// Unlink by shifting the end of the array over top of the link.
		for (ptrdiff_t i = childIndex_index + 1; i < parent->children_length; i++) {
			parent->children[i - 1] = parent->children[i];
		}
		
		// Deallocate index.
		--parent->children_length;
		if (parent->children_length > 0) {
			parent->children = realloc(parent->children, parent->children_length * sizeof(ptrdiff_t));
			if (parent->children == NULL) {
				outOfMemory();
				error = ERR_OUTOFMEMORY;
				goto cleanup_l;
			}
		}
		else {
			insane_free(parent->children);
		}
	}
	else {
		error("Cannot unlink child. Parent has no children.", "");
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}

/* entity_isValidEntityIndex
index:i         The index of an entity.
Globals:
	g_entityList:i
Description:    Print the contents of an entity.
*/
void entity_printEntity(int index) {
	entity_t *entity;
	
	if (entity_isValidEntityIndex(index)) {
		entity = &g_entityList.entities[index];
		
		printf("ENTITY #%i\n", index);
		printf("\tchildren {");
		for (int i = 0; i < entity->children_length; i++) {
			if (i != 0) {
				printf(", ");
			}
			printf("%td", entity->children[i]);
		}
		printf("}\n");
		printf("\tchildren_length %td\n", entity->children_length);
		printf("\tchildType %i\n", entity->childType);
		printf("\tposition {");
		for (int i = 0; i < 3; i++) {
			if (i != 0) {
				printf(", ");
			}
			printf("%f", entity->position[i]);
		}
		printf("}\n");
		printf("\torientation {");
		for (int i = 0; i < 3; i++) {
			printf("%f", entity->orientation.v[i]);
			printf(", ");
		}
		printf("%f", entity->orientation.s);
		printf("}\n");
	}
	else {
		warning("%i is not a valid entity index.", index);
	}
}


#ifdef CLIENT
static inline int entity_linkMaterial(ptrdiff_t parentIndex, ptrdiff_t materialIndex) {
	int error = ERR_CRITICAL;

	g_entityList.entities[parentIndex].materials_length++;
	g_entityList.entities[parentIndex].materials = realloc(g_entityList.entities[parentIndex].materials, g_entityList.entities[parentIndex].materials_length * sizeof(ptrdiff_t));
	if (g_entityList.entities[parentIndex].materials == NULL) {
		outOfMemory();
		error = ERR_OUTOFMEMORY;
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	return error;
}
#endif

/* Lua calls */
/* ========= */

int l_entity_createEntity(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int index = -1;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	
	error = entity_createEntity(&index, lua_tointeger(luaState, 1));
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	lua_pushinteger(luaState, index);
	lua_pushinteger(luaState, error);
	
	return 2;
}

int l_entity_deleteEntity(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	
	error = entity_deleteEntity(lua_tointeger(luaState, 1));
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_entity_linkChild(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int parentIndex;
	int childIndex;
	entity_childType_t parentType;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	if (!lua_isinteger(luaState, 2)) {
		error("Argument 2 must be an integer.", "");
		lua_error(luaState);
	}
	
	parentIndex = lua_tointeger(luaState, 1);
	childIndex = lua_tointeger(luaState, 2);
	
	if (!entity_isValidEntityIndex(parentIndex)) {
		error("Parent index (%i) references an invalid entity.", parentIndex);
		lua_error(luaState);
	}
	
	// Might not be an entity.
	parentType = g_entityList.entities[parentIndex].childType;
	switch (parentType) {
	case entity_childType_entity:
		if (!entity_isValidEntityIndex(childIndex)) {
			error("Child index (%i) references an invalid entity.", childIndex);
			lua_error(luaState);
		}
		break;
	case entity_childType_model:
		if (!obj_isValidModelIndex(childIndex)) {
			error("Child index (%i) references an invalid entity.", childIndex);
			lua_error(luaState);
		}
		break;
	case entity_childType_none:
		error("Parent cannot have children because it is set to type \"none\"", childIndex);
		lua_error(luaState);
	default:
		error("Invalid entity type %i", parentType);
		lua_error(luaState);
	}
	
	error = entity_linkChild(parentIndex, childIndex);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_entity_unlinkChild(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int parentIndex;
	int childIndex;
	// entity_childType_t parentType;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	if (!lua_isinteger(luaState, 2)) {
		error("Argument 2 must be an integer.", "");
		lua_error(luaState);
	}
	
	parentIndex = lua_tointeger(luaState, 1);
	childIndex = lua_tointeger(luaState, 2);
	
	if (!entity_isValidEntityIndex(parentIndex)) {
		error("Parent index (%i) references an invalid entity.", parentIndex);
		lua_error(luaState);
	}
	
	// // Might not be an entity.
	// parentType = g_entityList.entities[parentIndex].childType;
	// switch (parentType) {
	// case entity_childType_entity:
	// 	if (!entity_isValidEntityIndex(childIndex)) {
	// 		error("Child index (%i) references an invalid entity.", childIndex);
	// 		lua_error(luaState);
	// 	}
	// 	break;
	// case entity_childType_model:
	// 	if (!obj_isValidModelIndex(childIndex)) {
	// 		error("Child index (%i) references an invalid entity.", childIndex);
	// 		lua_error(luaState);
	// 	}
	// 	break;
	// case entity_childType_none:
	// 	error("Parent cannot have children because it is set to type \"none\"", childIndex);
	// 	lua_error(luaState);
	// default:
	// 	error("Invalid entity type %i", parentType);
	// 	lua_error(luaState);
	// }
	
	error = entity_unlinkChild(parentIndex, childIndex);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	if (error) {
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_entity_setPosition(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int index;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	
	index = lua_tointeger(luaState, 1);
	if (!entity_isValidEntityIndex(index)) {
		error("Index (%i) references an invalid entity.", index);
		lua_error(luaState);
	}
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].position[0] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].position[1] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].position[2] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	error = ERR_OK;

	lua_pushinteger(luaState, error);
	
	return 1;
}

int l_entity_setOrientation(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	int index;
	
	if (!lua_isinteger(luaState, 1)) {
		error("Argument 1 must be an integer.", "");
		lua_error(luaState);
	}
	
	index = lua_tointeger(luaState, 1);
	if (!entity_isValidEntityIndex(index)) {
		error("Index (%i) references an invalid entity.", index);
		lua_error(luaState);
	}
	
	lua_pushstring(luaState, "w");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key w must be a number. w has type \"%s\"", lua_typename(luaState, lua_type(luaState, -1)));
		lua_error(luaState);
	}
	g_entityList.entities[index].orientation.s = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "x");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key x must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].orientation.v[0] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "y");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key y must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].orientation.v[1] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	lua_pushstring(luaState, "z");
	if (lua_gettable(luaState, 2) != LUA_TNUMBER) {
		error("Key z must be a number", "");
		lua_error(luaState);
	}
	g_entityList.entities[index].orientation.v[2] = lua_tonumber(luaState, 3);
	lua_pop(luaState, 1);
	
	error = quat_normalize(&g_entityList.entities[index].orientation);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:

	lua_pushinteger(luaState, error);

	return 1;
}


#ifdef CLIENT

int l_entity_linkMaterial(lua_State *luaState) {
	int error = ERR_CRITICAL;
	
	ptrdiff_t materialIndex = -1;
	ptrdiff_t entityIndex = -1;
	
	if (lua_gettop(luaState) != 2) {
		critical_error("Function requires 2 arguments.", "");
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 1)) {
		critical_error("Argument 1 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, 1)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	if (!lua_isinteger(luaState, 2)) {
		critical_error("Argument 2 should be an integer, not a %s.", lua_typename(luaState, lua_type(luaState, 2)));
		error = ERR_CRITICAL;
		goto cleanup_l;
	}
	
	materialIndex = lua_tointeger(luaState, 1);
	if (!material_indexExists(g_materialList, materialIndex)) {
		error("Bad material index %i.", materialIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	entityIndex = lua_tointeger(luaState, 2);
	if (!entity_isValidEntityIndex(entityIndex)) {
		error("Bad entity index %i.", entityIndex);
		error = ERR_GENERIC;
		goto cleanup_l;
	}
	
	error = entity_linkMaterial(entityIndex, materialIndex);
	if (error) {
		goto cleanup_l;
	}
	
	error = ERR_OK;
	cleanup_l:
	
	if (error >= ERR_CRITICAL) {
		lua_error(luaState);
	}
	
	lua_pushinteger(luaState, error);
	
	return 1;
}

#endif
