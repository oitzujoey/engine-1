
cmake_minimum_required(VERSION 3.19)

project(engine-1
        VERSION 0
        DESCRIPTION "Game engine attempt number 2"
        HOMEPAGE_URL https://github.com/oitzujoey/engine-1
        LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_compile_options(-std=c17 -pedantic-errors -Werror -Wall -Wstrict-prototypes -DDEBUG -g -O0)
else()
	add_compile_options(-std=c17 -O3)
endif()

# I should learn CMake better.
add_compile_options(-DLINUX)
# add_compile_options(-DWINDOWS)

add_executable(sengine-1)
add_executable(cengine-1)

target_compile_options(sengine-1 PRIVATE -DSERVER)
target_compile_options(cengine-1 PRIVATE -DCLIENT)

# target_link_options(sengine-1 PRIVATE -fsanitize=address)
# target_link_options(cengine-1 PRIVATE -fsanitize=address)

add_subdirectory(src)

target_link_libraries(sengine-1 SDL2 lua m enet physfs)
target_link_libraries(cengine-1 SDL2 lua m enet physfs GL GLEW)
# target_link_libraries(cengine-1 SDL2 lua m enet physfs opengl32 glew32)
