cmake_minimum_required (VERSION 3.6)
project (assignment1)

add_executable(assignment1 
	./src/CS380_prog.cpp 
	../common/glad/glad.c)

# Adding glad from common folder
include_directories(../common/
	../common/glad/
	)
	

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT assignment1)

# C++ Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Setting up CUDA compiler (NVCC) flags
set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS})
set(CUDA_HOST_COMPILER clang++)

# Adding cuda libraries
find_package(CUDA)
include_directories(${CUDA_INCLUDE_DIRS})
    SET(ALL_CUDA_LIBS ${CUDA_LIBRARIES} ${CUDA_cusparse_LIBRARY} ${CUDA_cublas_LIBRARY})
    SET(LIBS ${LIBS} ${ALL_CUDA_LIBS})
    message(STATUS "CUDA_LIBRARIES: ${CUDA_INCLUDE_DIRS} ${ALL_CUDA_LIBS}")
    set(CUDA_PROPAGATE_HOST_FLAGS ON)
    set(CUDA_SEPARABLE_COMPILATION OFF)
	
target_link_libraries(assignment1 ${CUDA_LIBRARIES})



# Adding glfw3
#find_package(glfw3 REQUIRED)
add_subdirectory(../common/glfw-3.3 ../common/glfw-3.3/build)
target_link_libraries(assignment1 glfw)
target_compile_features(assignment1 PUBLIC cxx_std_11)
