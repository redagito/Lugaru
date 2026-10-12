# Unit tests for AppLib. References AppLib and Catch2 only.

project(AppTest)

# The tests under source/repro/ are deliberately not part of this binary. They
# reproduce a live STATUS_STACK_BUFFER_OVERRUN, and a crash takes the whole test
# binary with it - which would stop every other test in here from completing, not
# just the reproducer. They live in their own binary, AppTestRepro, so this one
# stays runnable while the bug is outstanding. Move them here once it is fixed.
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

list(FILTER SOURCE_FILES EXCLUDE REGEX "source/repro/")

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

# Tests that reproduce a live crash. These fail the process rather than an
# assertion, so they get their own binary - see the note above.

project(AppTestRepro)

# The suppression lives in source/ alongside the main suite, but this binary is
# the one that actually crashes - so it needs it even more. A reproducer that
# stops everything with a dialog is worse than one that exits.
set(REPRO_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/source/repro/SimulationTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/repro/AIRepeatTest.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/QuietFailures.cpp
)

file(GLOB_RECURSE EXTRA_REPRO_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/repro/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/repro/*.hpp
)
list(APPEND REPRO_FILES ${EXTRA_REPRO_FILES})
list(REMOVE_DUPLICATES REPRO_FILES)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${REPRO_FILES})

add_executable(${PROJECT_NAME} ${REPRO_FILES})

target_link_libraries(${PROJECT_NAME}
	PRIVATE AppLib
	PRIVATE Catch2::Catch2WithMain
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/
)

target_compile_options(${PROJECT_NAME} PRIVATE
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${LUGARU_MSVC_WARNINGS}>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
)

# Discovered so a crash shows up as a real failing test in ctest rather than a
# test that was never registered at all. The expectation is that these fail until
# the underlying bug is fixed.
include(Catch)
catch_discover_tests(${PROJECT_NAME}
	WORKING_DIRECTORY ${LUGARU_TEST_WORKING_DIR}
)