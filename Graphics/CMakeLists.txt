project(GraphicsLib)

# When on, the engine links against a stubbed OpenGL instead of the driver's
# import library and runs with no GL context at all. It exists so the simulation
# can be loaded and ticked in a test; see Graphics/source/GL/GLStub.cpp for what
# it promises and what it does not. Off by default, because the normal build must
# keep exercising the real GL path.
option(LUGARU_HEADLESS_GL "Link against a stubbed OpenGL so the simulation runs with no context" OFF)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.inc
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.inc
)

# The stub defines the same C symbols opengl32.lib exports. It therefore has to
# stay out of the normal build entirely, where it would collide at link time
# rather than be a choice.
list(FILTER SOURCE_FILES EXCLUDE REGEX "source/GL/")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_library(${PROJECT_NAME} ${SOURCE_FILES})


target_compile_definitions(${PROJECT_NAME}
	PRIVATE $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC glext::glext
	PUBLIC glu::glu

    PUBLIC AudioLib
)

if(LUGARU_HEADLESS_GL)
	add_library(GLStub STATIC source/GL/GLStub.cpp)
	target_include_directories(GLStub PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_compile_options(GLStub PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${LUGARU_MSVC_WARNINGS}>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
	)
	# The stub takes the place of the import library. Nothing else changes: same
	# sources, same warnings, same standard.
	target_link_libraries(${PROJECT_NAME} PUBLIC GLStub)
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
endif()

target_include_directories(${PROJECT_NAME} 
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(${PROJECT_NAME} PRIVATE
  PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX ${LUGARU_MSVC_WARNINGS}>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
  # -Wextra -Wpedantic 
)
