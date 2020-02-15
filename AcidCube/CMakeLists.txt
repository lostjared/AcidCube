cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libacidcam LANGUAGES CXX)
set(ACIDCUBE_MAJOR_VERSION 1)
set(ACIDCUBE_MINOR_VERSION 0)
set(ACIDCUBE_VERSION ${ACIDCAM_MINOR_VERSION}.${ACIDCAM_MAJOR_VERSION})
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build configuration is: ${CMAKE_BUILD_TYPE}")
message(STATUS "is the C++ compiler loaded? ${CMAKE_CXX_COMPILER_LOADED}")

if(CMAKE_CXX_COMPILER_LOADED)
	message(STATUS "The C++ Compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
	message(STATUS "The C++ Compiler Version: ${CMAKE_CXX_COMPILER_VERSION}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQURIED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#set(include_dest "include/acidcam")
#set(main_lib_dest "lib/acidcam")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
list(APPEND COMPILER_FLAGS "-Wall" "-pedantic" "-ansi" "-Ofast")
else()
message(STATUS "Debug Mode")
list(APPEND COMPILER_FLAGS "-Wall" "-pedantic" "-ansi")
endif()


find_package(OpenCV REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL2 REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

if(${OpenCV_VERSION} LESS 3.0)
	message(FATAL_ERROR "Requires OpenCV 3 or greater")
endif()

include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR} ${OpenCV_INCLUDE_DIRS} ${OpenGL_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS})
link_directories(/usr/local/lib)



add_executable(AcidCube ${CMAKE_SOURCE_DIR}/acidcube.cpp ${CMAKE_SOURCE_DIR}/MX_interface.cpp)
target_link_libraries(AcidCube ${OpenCV_LIBS} ${OPENGL_gl_LIBRARY} acidcam ${OPENGL_glu_LIBRARY} ${GLEW_SHARED_LIBRARIES} Threads::Threads SDL2 ${GLEW_LIBARIES})
target_compile_options(AcidCube PRIVATE ${COMPILER_FLAGS})

