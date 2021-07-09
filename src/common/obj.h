
#ifndef OBJ_H
#define OBJ_H

#include "types.h"

extern modelList_t g_modelList;

int obj_isValidModelIndex(int index);
int modelList_createModel(model_t **model, int *index);
void modelList_init(void);
void modelList_free(void);

void model_init(model_t *model);
void model_free(model_t *model);
#ifdef CLIENT
int l_model_linkDefaultMaterial(lua_State *luaState);
#endif

int l_obj_loadOoliteDAT(lua_State *luaState);

// int l_loadObj(lua_State *Lua);

#endif
