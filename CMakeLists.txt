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

set(peg_peg src/peg.peg)
set(leg_leg src/leg.leg)

add_executable(peg  src/peg.c 
                    ${peg_peg}-c
                    ${common_src}
                    ${common_headers}
                    )
add_executable(leg  src/leg.c
                    ${common_src}
                    ${common_headers}
                    )

if (MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4102")
elseif("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-parentheses-equality -Wno-array-bounds -Wno-error=unused-command-line-argument")
endif()

add_custom_target(newpeg 
                  COMMAND ${CMAKE_COMMAND} -E rename ${peg_peg}-c ${peg_peg}-c-old
                  COMMAND peg -o ${peg_peg}-c ${peg_peg}
                  DEPENDS peg
                  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                  COMMENT "Regenerating ${peg_peg}")

add_custom_target(newleg 
                  COMMAND ${CMAKE_COMMAND} -E rename src/leg.c src/leg.c-old
                  COMMAND leg -o src/leg.c ${leg_leg}
                  DEPENDS leg
                  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                  COMMENT "Regenerating ${leg_leg}")

add_custom_target(new 
                  DEPENDS newpeg newleg
                  COMMENT "Regenerating peg and leg")

add_custom_target(revert 
                  COMMAND ${CMAKE_COMMAND} -E remove ${peg_peg}-c src/leg.c
                  COMMAND ${CMAKE_COMMAND} -E rename ${peg_peg}-c-old ${peg_peg}-c
                  COMMAND ${CMAKE_COMMAND} -E rename src/leg.c-old src/leg.c
                  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                  COMMENT "Reverting ${peg_peg}-c -> ${peg_peg}-c-old and leg.c -> leg.c-old")

install(EXPORT pegleg-targets DESTINATION lib/cmake)

install(TARGETS peg leg
        DESTINATION bin/
        EXPORT pegleg-targets)

if(UNIX)
    install(FILES src/peg.1 
            DESTINATION man/man1)
endif(UNIX)

if (PEGLEG_BUILD_EXAMPLES)
    include(CTest)
    enable_testing()

    add_subdirectory(examples/)
endif()
