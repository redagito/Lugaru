project(Lugaru)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.def
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.rc
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_executable (${PROJECT_NAME} ${SOURCE_FILES})


target_link_libraries (${PROJECT_NAME}
	PRIVATE AppLib
)

target_compile_definitions(${PROJECT_NAME} 
	PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_include_directories(${PROJECT_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source/
)

target_compile_options(${PROJECT_NAME} 
	PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${LUGARU_MSVC_WARNINGS}>
		PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
	# -Wextra -Wpedantic 
)

# The game resolves DATA_DIR ("Data") relative to the current working directory
# and only chdirs to the executable's own folder on non-Windows platforms, so
# the data has to sit next to the binary. Copy it per configuration after the
# build rather than at configure time: a multi-config generator puts Debug and
# Release in separate folders, and copy_directory_if_different avoids recopying
# ~38 MB of assets on every incremental build.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/Data"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/Data"
	COMMENT "Copying game data next to Lugaru.exe"
)

# Linux packaging bits (metainfo, man page, desktop file). Not used by the
# Windows build, but kept alongside for parity with the previous layout.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		"${CMAKE_CURRENT_SOURCE_DIR}/Dist"
		"$<TARGET_FILE_DIR:${PROJECT_NAME}>/Dist"
)
