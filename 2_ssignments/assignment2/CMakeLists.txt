set(target assignment02)
set( chapter04_SOURCES
	scenemultilight.cpp scenemultilight.h
	sceneperfragment.cpp sceneperfragment.h
	main.cpp
	scenefog.cpp scenefog.h
	scenenormalmap.cpp scenenormalmap.h
	scenetoon.cpp scenetoon.h
	scenespot.cpp scenespot.h
	scenepbr.cpp scenepbr.h 
	scenepervertex.cpp scenepervertex.h
	scenestripe.cpp scenestripe.h
	scenelatti.cpp scenelatti.h
	)

add_executable( ${target} ${chapter04_SOURCES} )

target_compile_definitions(${target}
		PRIVATE
		GLFW_INCLUDE_NONE
		)

target_link_libraries( ${target}
		PRIVATE
		ingredients
		glfw
		${OPENGL_gl_LIBRARY}
		)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shader DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
