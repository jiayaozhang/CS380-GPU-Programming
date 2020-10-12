cmake_minimum_required (VERSION 3.6)
project (assignment)

# Adding compile units to executable
add_executable(assignment
	../common/glad/glad.c 
	./src/CS380_prog.cpp
	./src/glslprogram.cpp 
	./src/vbocube.cpp 
	./src/vbosphere.cpp 
	./src/vbomesh.cpp 
	./src/glutils.cpp 
	./src/texture.cpp ./src/texture.h
	./src/stb_image.h
	)
	
	
# Adding glad from common folder
include_directories(../common/ ../common/glad)

# Set project assignment as startup project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT assignment)

# C++ Compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Setting up CUDA compiler (NVCC) flags
# set(CUDA_NVCC_FLAGS ${CUDA_NVCC_FLAGS})
# set(CUDA_HOST_COMPILER clang++)

# Adding cuda libraries
# find_package(CUDA)
# include_directories(${CUDA_INCLUDE_DIRS})
#    SET(ALL_CUDA_LIBS ${CUDA_LIBRARIES} ${CUDA_cusparse_LIBRARY} ${CUDA_cublas_LIBRARY})
#    SET(LIBS ${LIBS} ${ALL_CUDA_LIBS})
#    message(STATUS "CUDA_LIBRARIES: ${CUDA_INCLUDE_DIRS} ${ALL_CUDA_LIBS}")
#    set(CUDA_PROPAGATE_HOST_FLAGS ON)
#    set(CUDA_SEPARABLE_COMPILATION OFF)
	
#target_link_libraries(assignment ${CUDA_LIBRARIES})



# Adding glfw3
#find_package(glfw3 REQUIRED)
add_subdirectory(../common/glfw-3.3 ../common/glfw-3.3/build)
target_link_libraries(assignment glfw)
# target_include_directories(assignment PUBLIC stb)
target_compile_features(assignment PUBLIC cxx_std_11)
