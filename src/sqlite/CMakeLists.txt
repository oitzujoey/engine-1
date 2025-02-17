set(common_sources sqlite3.c)
target_sources(sengine-1 PRIVATE ${common_sources})
target_sources(cengine-1 PRIVATE ${common_sources})
target_sources(sqlite3 PRIVATE shell.c ${common_sources})
