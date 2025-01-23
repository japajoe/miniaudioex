cmake_minimum_required(VERSION 3.5.1)

project(miniaudioex)

set(BUILD_PLATFORM "unknown" CACHE STRING "Target platform for the build")
set(CMAKE_C_FLAGS_RELEASE "-O3")

message("Configuring for platform: ${BUILD_PLATFORM}")

file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

add_definitions(-DNDEBUG)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

if(BUILD_PLATFORM STREQUAL "linux_x86_64")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl)
elseif(BUILD_PLATFORM STREQUAL "linux_arm")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl -latomic)
elseif(BUILD_PLATFORM STREQUAL "linux_arm64")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl -latomic)
endif()