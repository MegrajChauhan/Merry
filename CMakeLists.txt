# This is a novice trying to learn CMAKE
# You are free to write your own make file

# Initial setup
cmake_minimum_required(VERSION 3.28.2)
project(mvm C ASM)

# Enable assembky
# I don't know if this is needed
enable_language(ASM)

# Add the needed options here
set(compile_opts -Wextra -Wall)

# Add compile options(Do we even need this?)
if(CMAKE_BUILD_TYPE STREQUAL "debug")
    add_compile_options(-g)
else()
    add_compile_options(-O3)
endif()

# Merry needs 64-bit machines to run
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
   message(FATAL_ERROR "Merry requires 64-bit machines to run.")    
endif()

# Use GCC. I don't know about CLANG
if(MSVC)
    message(FATAL_ERROR "Please use GCC to compile.")
endif()

# setup the build directory
set(output_dir "${CMAKE_SOURCE_DIR}/build")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${output_dir}/${CMAKE_BUILD_TYPE}/libs/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${output_dir}/${CMAKE_BUILD_TYPE}/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${output_dir}/${CMAKE_BUILD_TYPE}/libs/)

# Include directories

# Platform-specific include directories
if(WIN32)
    set(Platform_Include_Dirs 
    abs/hard/win
    abs/soft/win
    )
elseif(LINUX)
    set(Platform_Include_Dirs 
    abs/hard/linux 
    abs/soft/linux
    )
endif()

# Other include directories
set(Include_Dirs
   utils
   core
   defs
   dev
   os
   tools
)

# The libraries to add
set(LIBS_TO_ADD
    merry_thread.c
    merry_time.c
    merry_process.c
    merry_dynl.c
    merry_fio.c
    merry_errno.c
    merry_stack.c
    merry_dmemory.c
    merry_memory.c
    merry_request_queue.c
    merry_request_hdlr.c
    merry_core.c
    merry_nreader.c
    merry_os.c
    merry.c
    arithmetic.S
)

set(LIBS_NAME
    mthread
    mtime
    mprocess
    mdynl
    mfio
    merrno
    mstack
    mdmem
    mimem
    mreqqueue
    mreqhdlr
    mcore
    mnreader
    mos
    mclo
    marithmetic
)

include_directories(${Include_Dirs} ${Platform_Include_Dirs})


list(LENGTH LIBS_TO_ADD LIBS_TO_ADD_LENGTH)
math(EXPR LIBS_TO_ADD_LENGTH "${LIBS_TO_ADD_LENGTH} - 1")

foreach(i RANGE ${LIBS_TO_ADD_LENGTH})
    list(GET LIBS_TO_ADD ${i} LIB_SRC)
    list(GET LIBS_NAME ${i} LIB_NAME)
    add_library(${LIB_NAME} ${LIB_SRC})
endforeach()

# For assembly language
set_target_properties(marithmetic PROPERTIES LINKER_LANGUAGE ASM)

# Build the project
add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} ${LIBS_NAME})
