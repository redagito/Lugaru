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
)

# The App layer tests construct the real objects - Console, KeyCapture,
# GameState, GameAssets, Person, Terrain, Weapons - and assert on their
# behaviour. They deliberately do not read the source tree: a test that
# pattern-matches source text asserts the code looks a certain way, which taxes
# every legitimate refactor and catches nothing that compiling does not.

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