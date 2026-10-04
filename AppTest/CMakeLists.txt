# Unit tests for AppLib. References AppLib and Catch2 only.

project(AppTest)

file(GLOB TEST_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_FILES})

add_executable(${PROJECT_NAME} ${TEST_FILES})

target_link_libraries(${PROJECT_NAME}
	PRIVATE AppLib
	PRIVATE Catch2::Catch2WithMain
)

target_compile_definitions(${PROJECT_NAME}
	PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
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