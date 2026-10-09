# Unit tests for AppLib. References AppLib and Catch2 only.

project(AppTest)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.inc
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.rc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
	PRIVATE AppLib
	PRIVATE Catch2::Catch2WithMain
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
	# The migration guard reads Globals.h, GameGlobals.h and GameState.hpp at
	# runtime. Hand it their real location rather than letting it guess one from
	# whatever working directory ctest was configured with.
	PRIVATE LUGARU_APP_INCLUDE_DIR="${CMAKE_SOURCE_DIR}/App/include"
	# The same guard reads App/source/GameDraw.cpp to check where the renderer's
	# multiplier save slot lives.
	PRIVATE LUGARU_APP_SOURCE_DIR="${CMAKE_SOURCE_DIR}/App/source"
	# The asset ownership guard scans Lugaru/source as well: main.cpp is the other
	# place that owns a GameAssets, and it is not under App.
	PRIVATE LUGARU_LUGARU_SOURCE_DIR="${CMAKE_SOURCE_DIR}/Lugaru/source"
	# The console command documentation guard reads Docs/DEVTOOLS.txt, which lives
	# outside both trees.
	PRIVATE LUGARU_REPO_ROOT="${CMAKE_SOURCE_DIR}"
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/
)

target_compile_options(${PROJECT_NAME} PRIVATE
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${LUGARU_MSVC_WARNINGS}>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
  # -Wextra -Wpedantic
)

include(Catch)
catch_discover_tests(${PROJECT_NAME}
	WORKING_DIRECTORY ${LUGARU_TEST_WORKING_DIR}
)