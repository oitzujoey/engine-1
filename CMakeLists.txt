
cmake_minimum_required(VERSION 3.20)

project(engine-1
        VERSION 0.0
        DESCRIPTION "Game engine attempt number 2"
        HOMEPAGE_URL https://github.com/oitzujoey/engine-1
        LANGUAGES C CXX)

message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_C_COMPILER_ID = ${CMAKE_C_COMPILER_ID}")
message(STATUS "CMAKE_C_COMPILER_VERSION = ${CMAKE_C_COMPILER_VERSION}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${engine-1_SOURCE_DIR}/cmake")

# set(CMAKE_BUILD_TYPE Debug)

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
find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(PhysFS REQUIRED)
find_package(ENet REQUIRED)

include_directories(${ENET_INCLUDE_DIRS})

# if(CMAKE_BUILD_TYPE STREQUAL Debug)
# 	add_compile_options(-pedantic-errors -Werror -Wall -Wstrict-prototypes -DDEBUG -g -O0)
# else()
# 	add_compile_options(-O3)
# endif()

# I should learn CMake better.
add_compile_options(-DLINUX)
# add_compile_options(-DWINDOWS)

add_executable(sengine-1)
add_executable(cengine-1)
add_executable(engine-1-runner)
add_executable(sqlite3)
add_executable(lua)
add_executable(luac)

target_compile_options(sengine-1 PRIVATE -DSERVER)
target_compile_options(cengine-1 PRIVATE -DCLIENT)
target_compile_options(engine-1-runner PRIVATE -DRUNNER)

# target_link_options(sengine-1 PRIVATE -fsanitize=address)
# target_link_options(cengine-1 PRIVATE -fsanitize=address)
# target_link_options(engine-1-runner PRIVATE -fsanitize=address)

add_subdirectory(src)

target_link_libraries(sengine-1 SDL2 m enet physfs-static)
target_link_libraries(cengine-1 SDL2 m enet GL GLEW physfs-static)
# target_link_libraries(cengine-1 SDL2 m enet opengl32 glew32 physfs-static)
target_link_libraries(engine-1-runner PUBLIC Qt6::Widgets)
