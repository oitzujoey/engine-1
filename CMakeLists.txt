
cmake_minimum_required(VERSION 3.20)

project(engine-1
        VERSION 0.0
        DESCRIPTION "Game engine attempt number 2"
        HOMEPAGE_URL https://github.com/oitzujoey/engine-1
        LANGUAGES C CXX ASM)

message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_C_COMPILER_ID = ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_VERSION = ${CMAKE_C_COMPILER_VERSION}")


# set(DEFAULT_GAME_ZIP_NAME "game.zip" CACHE STRING "Set the name of the ZIP file the engine looks for the game in.")
# add_compile_definitions(DEFAULT_GAME_ZIP_NAME="${DEFAULT_GAME_ZIP_NAME}")


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(TS_FILES runner_en_US.ts)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(OpenGL REQUIRED)

# if(CMAKE_BUILD_TYPE STREQUAL Debug)
# 	add_compile_options(-pedantic-errors -Werror -Wall -Wstrict-prototypes -DDEBUG -g -O0)
# else()
# 	add_compile_options(-O3)
# endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  add_compile_options(-DWINDOWS -DNOTERMINAL)
else()
  add_compile_options(-DLINUX)
endif()

add_executable(sengine-1)
add_executable(cengine-1)
#add_executable(engine-1-runner)
add_executable(sqlite3)
add_executable(lua)
add_executable(luac)

set(models
  game_opensauce26/models/airliner
  game_sandbox/blender/cube
  game_sandbox/blender/frogcube
  game_sandbox/blender/plane
  game_sandbox/blender/sandbox
)
foreach(model ${models})
  add_custom_command(
    OUTPUT ${PROJECT_SOURCE_DIR}/${model}.cmsh ${PROJECT_SOURCE_DIR}/${model}.rmsh
    DEPENDS ${PROJECT_SOURCE_DIR}/${model}.obj
    COMMAND ${PROJECT_SOURCE_DIR}/scripts/obj2cmsh.py ${PROJECT_SOURCE_DIR}/${model}.obj
    COMMAND ${PROJECT_SOURCE_DIR}/scripts/obj2rmsh.py ${PROJECT_SOURCE_DIR}/${model}.obj
  )
endforeach()
set(models_msh
  game_sandbox/blender/cube.rmsh
  game_sandbox/blender/cube.cmsh
  game_opensauce26/models/airliner.rmsh
  game_opensauce26/models/airliner.cmsh
)
add_custom_target(models_target ALL DEPENDS ${models_msh})

add_custom_command(
  OUTPUT ${PROJECT_BINARY_DIR}/game.zip
  DEPENDS ${PROJECT_SOURCE_DIR}/custom_binary.json
  COMMAND ${PROJECT_SOURCE_DIR}/scripts/makepak.py ${PROJECT_SOURCE_DIR}/custom_binary.json ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR}
)
add_custom_target(resources_target ALL DEPENDS ${PROJECT_BINARY_DIR}/game.zip models_target)

target_compile_options(sengine-1 PRIVATE -DSERVER)
target_compile_options(cengine-1 PRIVATE -DCLIENT -DNOTERMINAL)
#target_compile_options(engine-1-runner PRIVATE -DRUNNER)

# target_link_options(sengine-1 PRIVATE -fsanitize=address)
# target_link_options(cengine-1 PRIVATE -fsanitize=address)
# target_link_options(engine-1-runner PRIVATE -fsanitize=address)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_options(cengine-1 PRIVATE "-static")
endif()

add_dependencies(cengine-1 resources_target)
add_dependencies(sengine-1 resources_target)

add_subdirectory(src)

target_link_libraries(sengine-1 m physfs-static enet SDL2-static)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries(cengine-1 m enet opengl32 glew_s physfs-static SDL2-static)
else()
  target_link_libraries(cengine-1 m GL physfs-static enet SDL2-static glew_s)
endif()
#target_link_libraries(engine-1-runner PUBLIC Qt6::Widgets)
