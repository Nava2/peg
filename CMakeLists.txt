cmake_minimum_required(VERSION 3.0)
project(pegleg C)

option(PEGLEG_BUILD_EXAMPLES    "Build the examples and test them." ON)
option(USE_CCACHE               "Use `ccache` if available"         ON)

if (USE_CCACHE)
    find_program(CCACHE_FOUND ccache)

    if (CCACHE_FOUND)
        message(STATUS "Using ccache")
    
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache) 
    endif(CCACHE_FOUND)
endif(USE_CCACHE)

set(common_headers src/tree.h src/version.h)

set(common_src src/tree.c
               src/compile.c )

if(MSVC)
  include_directories(SYSTEM win/)
  
  list(APPEND common_src        win/getopt.c)
  list(APPEND common_headers    win/compat.h 
                                win/getopt.h
                                win/libgen.h 
                                win/unistd.h)
endif()

set(GENERATED_SOURCE_DIR ${CMAKE_BINARY_DIR}/src-gen)
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/src-gen)

set(PEGLEG_INCLUDE_DIRS ${CMAKE_CURRENT_LIST_DIR}/src ${GENERATED_SOURCE_DIR})

include_directories(${PEGLEG_INCLUDE_DIRS})

add_executable(peg  src/peg.c 
                    src/peg.peg-c
                    ${common_src}
                    ${common_headers}
                    )
add_executable(leg  src/leg.c
                    ${common_src}
                    ${common_headers}
                    )

if (MSVC)
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} /wd4102 )
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS} -Wno-parentheses-equality 
                                           -Wno-array-bounds)
endif()

if (PEGLEG_BUILD_EXAMPLES)
    include(CTest)
    enable_testing()

    add_subdirectory(examples/)
endif()
