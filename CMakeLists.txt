cmake_minimum_required(VERSION 3.5...3.28)

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
    "${PROJECT_SOURCE_DIR}/include/miniaudioex"
    "${PROJECT_SOURCE_DIR}/include/miniaudioex/extras"
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
elseif(MINIAUDIOEX_BUILD_PLATFORM STREQUAL "ios")
	set_source_files_properties("${PROJECT_SOURCE_DIR}/miniaudio.c" PROPERTIES LANGUAGE OBJC)
	set_source_files_properties("${PROJECT_SOURCE_DIR}/miniaudioex.c" PROPERTIES LANGUAGE OBJC)
	set_source_files_properties("${PROJECT_SOURCE_DIR}/extras/miniaudio_libvorbis.c" PROPERTIES LANGUAGE OBJC)
    add_definitions(-DMA_NO_RUNTIME_LINKING)
    target_link_libraries(${PROJECT_NAME}
        "-framework CoreFoundation"
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework AVFoundation"
        "-lpthread"
        "-lm"
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.japajoe.miniaudioex
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING "1.0"
        MACOSX_FRAMEWORK_BUNDLE_VERSION "1"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
    )
endif()
