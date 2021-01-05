
cmake_minimum_required(VERSION 3.19)

project(engine-1
        VERSION 0
        DESCRIPTION "Game engine attempt number 2"
        HOMEPAGE_URL https://github.com/oitzujoey/engine-1
        LANGUAGES C)

set(CMAKE_BUILD_TYPE Debug)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_compile_options(-std=c99 -pedantic-errors -Werror -Wall -Wstrict-prototypes -DDEBUG -g -O0)
else()
	add_compile_options(-std=c99 -O3)
endif()

add_executable(sengine-1)
add_executable(cengine-1)

add_subdirectory(src)

target_link_libraries(sengine-1 SDL2 SDL2_net lua m)
target_link_libraries(cengine-1 SDL2 SDL2_net lua m)
