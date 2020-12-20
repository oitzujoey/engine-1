
#ifndef CFG_H
#define CFG_H

#include <stdbool.h>
#include "str.h"
#include "common.h"

typedef enum cfg_var_type_e {
	none,
	vector,
	integer,
	string
} cfg_var_type_t;

#define CFG_VAR_PERMISSION_NONE     (0b000)
#define CFG_VAR_PERMISSION_READ     (1<<0)
#define CFG_VAR_PERMISSION_WRITE    (1<<1)
#define CFG_VAR_PERMISSION_DELETE   (1<<2)
#define CFG_VAR_PERMISSION_ALL      (0b111)

typedef struct {
	char *name;
	vec_t vector;
	int integer;
	char *string;
	cfg_var_type_t type;
	unsigned int permissions;
} cfg_var_t;

typedef struct {
	bool lock;
	cfg_var_t *vars;
	int vars_length;
} cfg_t;

extern cfg_t cfg;

int cfg_initVars(const cfg_var_t *initCfgList);
cfg_var_t *cfg_addVarNone(const char *name);
cfg_var_t *cfg_addVarVector(const char *name, const vec_t value);
cfg_var_t *cfg_addVarInt(const char *name, const int value);
cfg_var_t *cfg_addVarString(const char *name, const char *value);
int cfg_setVarVector(cfg_var_t *var, const vec_t value);
int cfg_setVarInt(cfg_var_t *var, const int value);
int cfg_setVarString(cfg_var_t *var, const char *value);
int cfg_getVarVector(cfg_var_t *var, vec_t *value);
int cfg_getVarInt(cfg_var_t *var, int *value);
int cfg_getVarString(cfg_var_t *var, char **value);
cfg_var_t *cfg_findVar(const char *name);
int cfg_deleteVar(cfg_var_t *var);
int cfg_printVar(cfg_var_t *var);

int cfg_execString(const string_t *line);
int cfg_execFile(const char *filepath);
void cfg_free(void);

#endif
