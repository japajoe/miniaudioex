cmake_minimum_required(VERSION 3.5.1)

project(miniaudioex)

set(BUILD_PLATFORM "unknown" CACHE STRING "Target platform for the build")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_C_FLAGS_RELEASE "-O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic")

if(BUILD_PLATFORM STREQUAL "windows")
    message("Configuring for platform: ${BUILD_PLATFORM}")
elseif(BUILD_PLATFORM STREQUAL "linux")
    message("Configuring for platform: ${BUILD_PLATFORM}")
elseif(BUILD_PLATFORM STREQUAL "mac")
    message("Configuring for platform: ${BUILD_PLATFORM}")
else()
    message(FATAL_ERROR "Unsupported platform: ${BUILD_PLATFORM}")
endif()

file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
#add_executable(${PROJECT_NAME} ${SOURCES})

if(BUILD_PLATFORM STREQUAL "linux")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl)
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/lib)
