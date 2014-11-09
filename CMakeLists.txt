# CMakeLists.txt - microLog_test
#
# Copyright 2012-2014 Pietro Mele
#
# References:
#    http://www.cmake.org/cmake/help/examples.html
#    http://llvm.org/docs/CMake.html

cmake_minimum_required(VERSION 2.8)

set(PRJ microLog_test)

project(${PRJ})

add_definitions(-DVERSION="0.0.1")
add_definitions(-DMICRO_LOG_EXECUTABLE_NAME="${PRJ}")
add_definitions(-DMICRO_LOG_TEST)

# Specify the build type on the command line: Release, Debug, RelWithDebInfo, MinSizeRel
#set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS "-std=c++11 -stdlib=libc++")

message("==============================================")
message("Building project: ${PRJ}")
message("==============================================")

message("Executable output directory: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM})
message(STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR})	#? i386
message(STATUS "UNIX: " ${UNIX})
message(STATUS "CMAKE_C_FLAGS:   " ${CMAKE_C_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE})
message(STATUS "BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS})
message(STATUS "CMAKE_C_COMPILER:   " ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	set(CPP_COMPILER "LLVM/Clang")
	set(CPP_VERSION  ${CMAKE_Clang_COMPILER_VERSION})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CPP_COMPILER "GNU g++")
	set(CPP_VERSION  ${CMAKE_GNU_COMPILER_VERSION})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	set(CPP_COMPILER "Intel C++")
	set(CPP_VERSION  ${CMAKE_Intel_COMPILER_VERSION})
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CPP_COMPILER "Microsoft C++")
	set(CPP_VERSION  ${CMAKE_MSVC_COMPILER_VERSION})
else()
	set(CPP_COMPILER "unknown")
	set(CPP_VERSION  "unknown")
endif()
message(STATUS "C++ compiler: " ${CPP_COMPILER} " vers. " ${CPP_VERSION})

set(SRC_DIR .)

set(SRC
	microLog_config.hpp
	microLog.hpp
	microLog_test.cpp)

add_executable(${PRJ} ${SRC})


#---
#include_directories(${SRCDIR}/${PRJ})
#link_directories(${BIN})
#target_link_libraries(${PRJ} ${LIBS})


