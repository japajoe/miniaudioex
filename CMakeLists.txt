cmake_minimum_required(VERSION 3.5.1)

project(miniaudioex)

set(BUILD_PLATFORM "unknown" CACHE STRING "Target platform for the build")

if(BUILD_PLATFORM STREQUAL "windows")
    # Windows-specific configurations
    message("Configuring for platform: ${BUILD_PLATFORM}")
    set(CMAKE_C_COMPILER "x86_64-w64-mingw32-gcc")
    set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")
elseif(BUILD_PLATFORM STREQUAL "linux")
    # Linux-specific configurations
    message("Configuring for platform: ${BUILD_PLATFORM}")
elseif(BUILD_PLATFORM STREQUAL "mac")
    # Mac-specific configurations
    message("Configuring for platform: ${BUILD_PLATFORM}")
else()
    message(FATAL_ERROR "Unsupported platform: ${BUILD_PLATFORM}")
endif()

set(CMAKE_C_FLAGS_RELEASE "-O3")
#set(CMAKE_C_COMPILER "x86_64-w64-mingw32-gcc")
#set(CMAKE_CXX_COMPILER "x86_64-w64-mingw32-g++")

# Add sources
file(GLOB SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.c"
)

# Will add __FILENAME__ macros for all source files, which is the filename without full find_path
# Courtesy of SO
function(define_file_basename_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # Add the FILE_BASENAME=filename compile definition to the list.
        get_filename_component(basename "${sourcefile}" NAME)
        list(APPEND defs "__FILENAME__=\"${basename}\"")
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs})
    endforeach()
endfunction()

# Specify include Directory
include_directories(
    "${PROJECT_SOURCE_DIR}/include"
)

#add_library(${PROJECT_NAME} STATIC ${SOURCES})
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if(BUILD_PLATFORM STREQUAL "linux")
    target_link_libraries(${PROJECT_NAME} -lpthread -lm -ldl)
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

define_file_basename_for_sources(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/lib)

