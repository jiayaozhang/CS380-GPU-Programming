cmake_minimum_required (VERSION 3.0)

set (CMAKE_CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

project ( 2_assignment_myversion )

if(MSVC)
  add_definitions(/DNOMINMAX)
endif()

find_package( glm CONFIG REQUIRED )
find_package( glfw3 CONFIG REQUIRED )
find_package( OpenGL REQUIRED )

include_directories( ingredients )

add_subdirectory( ingredients )
add_subdirectory( assignment2 )



file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/media DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
