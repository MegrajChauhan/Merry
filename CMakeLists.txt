cmake_minimum_required(VERSION 3.28.2)
project(mvm)

# enable assembly language
enable_language(ASM)

set(btype "debug")

if(btype STREQUAL "debug")
   add_compile_options(-g -Wall -Wextra)
else()
   add_compile_options(-O3 -Wall -Wextra)
endif()

# Check for the system's bus length
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "Merry requires a 64-bit operating system to run.")
endif()

if(MSVC)
    message(FATAL_ERROR "Please use gcc.")
endif()

# Setting up build directory
# Ensure all output directories are set correctly
set(OutputDir "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OutputDir}/${btype}/libs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OutputDir}/${btype}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OutputDir}/${btype}/libs)


# Include directories
# Check for platform and set platform-specific include directories
if(WIN32)
    set(PlatFormIncludeDirs
        merry/abs/hard/win
        merry/abs/soft/win
    )
elseif(LINUX)
    set(PlatFormIncludeDirs
        merry/abs/hard/linux
        merry/abs/soft/linux
    )
endif()

# Set common include directories and include platform-specific directories
set(IncludeDirs
    utils
    merry/core
    merry/defs
    merry/dev
    merry/os
    merry/tools
    ${PlatFormIncludeDirs}
)

include_directories(${IncludeDirs})

# The libraries to add
# set(LIBS_TO_ADD
# )

# set(LIBS_NAME
# )

# list(LENGTH LIBS_TO_ADD LIBS_TO_ADD_LENGTH)
# math(EXPR LIBS_TO_ADD_LENGTH "${LIBS_TO_ADD_LENGTH} - 1")

# foreach(i RANGE ${LIBS_TO_ADD_LENGTH})
#     list(GET LIBS_TO_ADD ${i} LIB_SRC)
#     list(GET LIBS_NAME ${i} LIB_NAME)
#     add_library(${LIB_NAME} ${LIB_SRC})
# endforeach()

# Add subdirectories
add_subdirectory(merry)

# Add executable
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${ADDITIONAL_FILESA} ${ADDITIONAL_FILESM})

