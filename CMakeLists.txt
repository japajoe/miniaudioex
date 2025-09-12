cmake_minimum_required(VERSION 3.10...3.28)

project(miniaudioex)

set(MINIAUDIOEX_BUILD_PLATFORM "unknown" CACHE STRING "Target platform for the build")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -DNDEBUG")
    if(MSVC)
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /DEBUG:NONE")
    else()
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -g0")
    endif()
endif()

option(MINIAUDIOEX_BUILD_SHARED "Build shared library" ON)

message(STATUS "miniaudioex: Configuring for platform: ${MINIAUDIOEX_BUILD_PLATFORM}")

file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

if(MINIAUDIOEX_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
    message(STATUS "miniaudioex: Building shared library")
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
    message(STATUS "miniaudioex: Building static library")
endif()

if(MINIAUDIOEX_BUILD_PLATFORM STREQUAL "linux_x86_64")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl)
elseif(MINIAUDIOEX_BUILD_PLATFORM STREQUAL "linux_arm")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl -latomic)
elseif(MINIAUDIOEX_BUILD_PLATFORM STREQUAL "linux_arm64")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl -latomic)
endif()
