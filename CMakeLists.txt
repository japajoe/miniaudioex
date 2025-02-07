#cmake_minimum_required(VERSION 3.5.1)
cmake_minimum_required(VERSION 3.10...3.28)

project(miniaudioex)

set(MINIAUDIOEX_BUILD_PLATFORM "unknown" CACHE STRING "Target platform for the build")
set(CMAKE_C_FLAGS_RELEASE "-O3")
option(MINIAUDIOEX_BUILD_SHARED "Build shared library" ON)

message(STATUS "miniaudioex: Configuring for platform: ${MINIAUDIOEX_BUILD_PLATFORM}")

file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

add_definitions(-DNDEBUG)

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
