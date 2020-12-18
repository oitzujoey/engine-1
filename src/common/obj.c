
#include "obj.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "str.h"

static const char *getExt(const char *filename) {
    
    const char *dot = &filename[strlen(filename)];
    
    do {
        --dot;
    } while ((*dot != '.') && (dot > filename));
    
    if (dot == filename)
        return NULL;
    
    return dot+1;
}

static char *getFileText(const char *filename) {
    
    FILE *file = fopen(filename, "r");
    char c;
    string_t str;
    string_init(&str, "");
    
    do {
        c = fgetc(file);
        string_append_char(&str, c);
    } while (c != EOF);
    
    fclose(file);
    
    return str.value;
}

int l_loadObj(lua_State *Lua) {
    
    const char *filename = lua_tostring(Lua, 1);
    const char *ext;
    char *filetext;
    
    ext = getExt(filename);
    
    if (ext == NULL) {
        fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to missing file extension. Should be .obj\n", filename);
        return 0;
    }
    
    if (strcmp(ext, "obj")) {
        fprintf(stderr, "Error: (l_loadObj) Refusing to open file \"%s\" as Wavefront OBJ due to incorrect file extension. Should be .obj\n", filename);
        return 0;
    }
    
    filetext = getFileText(filename);
    lua_pushstring(Lua, filetext);
    free(filetext);
    
    return 1;
}
