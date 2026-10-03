cmake_minimum_required(VERSION 3.25.0)
project(Lugaru)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Lugaru)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Single source of truth for the language standard. Must stay in sync with
# compiler.cppstd in the Conan profile (see conanfile.py / README).
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
	# Ignore warnings about missing pdb
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()

find_package(OpenGL REQUIRED)
find_package(OpenAL REQUIRED)
find_package(JPEG REQUIRED)
find_package(PNG REQUIRED)
find_package(SDL2 REQUIRED)
find_package(vorbis REQUIRED)
find_package(jsoncpp REQUIRED)
find_package(glext REQUIRED)
find_package(glu REQUIRED)

include(CTest)
find_package(Catch2 REQUIRED)

# Game objects resolve DATA_DIR ("Data") relative to the working directory, so
# tests that load assets run from the source tree's Lugaru folder. That keeps
# them independent of the build layout and of which configuration is being
# tested.
set(LUGARU_TEST_WORKING_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Lugaru)

# Library layers, bottom-up. Each library is followed by its sibling test
# project, which references only that library plus Catch2.
add_subdirectory(Foundation)
add_subdirectory(FoundationTest)
add_subdirectory(Audio)
add_subdirectory(AudioTest)
add_subdirectory(Graphics)
add_subdirectory(GraphicsTest)
add_subdirectory(Game)
add_subdirectory(GameTest)
add_subdirectory(App)
add_subdirectory(AppTest)
add_subdirectory(Lugaru)
