
set(common_sources cfg2.c common.c entity.c file.c log.c lua_common.c lua_sandbox.c obj.c vfs.c network.c vector.c terminal.c str2.c str3.c parse_mesh.c memory.c str4.c allocator.c arena.c sqlite3.c)
target_sources(sengine-1 PRIVATE ${common_sources})
target_sources(cengine-1 PRIVATE ${common_sources})
