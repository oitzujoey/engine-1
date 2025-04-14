
void shaders_init(void);
int shaders_quit(void);
int shader_create(Shader **shader, Str4 *vertexShader_sourceCode, Str4 *fragmentShader_sourceCode, bool instanced);
int l_shader_create(lua_State *l);
int l_shader_setInstanced(lua_State *l);
