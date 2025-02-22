
#ifndef TEXTURE_H
#define TEXTURE_H

#include "../common/types.h"

extern material_list_t g_materialList;

int material_initList(material_list_t *materialList);
void material_freeList(material_list_t *materialList);
bool material_indexExists(const material_list_t materialList, const ptrdiff_t materialIndex);

int l_material_create(lua_State *luaState);
int l_material_setDepthSort(lua_State *luaState);
int l_material_setCull(lua_State *l);

#endif
