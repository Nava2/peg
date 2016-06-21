
find_program(DIFF diff)
if(NOT DIFF)
    message(FATAL_ERROR "Can not run examples without `diff`")
endif()

find_program(ECHO echo)
if(NOT ECHO)
    message(FATAL_ERROR "Can not run examples without `echo`")
endif()

find_program(CAT cat)
if(NOT CAT)
    message(FATAL_ERROR "Can not run examples without `cat`")
endif()

find_program(RM rm)
if(NOT RM)
    message(FATAL_ERROR "Can not run examples without `rm`")
endif()

set(EXAMPLE_SCRIPT_DIR ${GENERATED_SOURCE_DIR}/examples/scripts)
set(PARSER_SRC_DIR ${GENERATED_SOURCE_DIR}/examples)

# Generate all of the example parsers
function(generate_parsers tgt out_dir)

    set(pegs )
    set(legs )
    
    set(curr_list)
    foreach(f ${ARGN})
        if("${f}" STREQUAL "PEG")
            set(curr_list pegs)
        elseif("${f}" STREQUAL "LEG")
            set(curr_list legs)
        else()
            list(APPEND ${curr_list} ${f})
        endif()
    endforeach()
    
    add_custom_target("${tgt}" COMMENT "Generate parsers")

    add_custom_target(${tgt}_generate_pegs 
                        DEPENDS peg
                        COMMENT "Generate parsers from PEGs")
    foreach(fpeg ${pegs})
        set(gen_file ${out_dir}/${fpeg}.c)
        add_custom_command(TARGET ${tgt}_generate_pegs
                        COMMAND "$<TARGET_FILE:peg>" -o "${gen_file}" "${fpeg}"
                        BYPRODUCTS ${gen_file}
                        DEPENDS peg
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                        COMMENT "Generate: ${fpeg} -> ${gen_file}"
                        VERBATIM)
    endforeach()
    
    add_custom_target(${tgt}_generate_legs 
                        DEPENDS leg
                        COMMENT "Generate parsers from LEGs")
    foreach(fleg ${legs})
        set(gen_file ${out_dir}/${fleg}.c)
        add_custom_command(TARGET ${tgt}_generate_legs
                        PRE_BUILD
                        COMMAND "$<TARGET_FILE:leg>" -o "${gen_file}" "${fleg}"
                        BYPRODUCTS ${gen_file}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                        COMMENT "Generate: ${fleg} -> ${gen_file}"
                        VERBATIM)
    endforeach()
    
    include_directories(${out_dir})
    add_dependencies(${tgt} ${tgt}_generate_pegs ${tgt}_generate_legs)
endfunction(generate_parsers)

function(add_test_ex generator_type check_target tgt srcs_tgt parse_type parse_arg ) # ... sources 

    if(UNIX)
        find_program(BASH bash)
        if(NOT BASH)
            message(FATAL_ERROR "Can not run examples without `bash` on *nix-like platforms")
        endif()

        set(script_ext "sh")
        set(script_nl UNIX)
    elseif(WIN32)
        set(script_ext "bat")
        set(script_nl WIN32)
    else()
        message(FATAL_ERROR "unknown platform")
    endif()
    
    set(sources )
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/${tgt}.c")
        list(APPEND sources "${CMAKE_CURRENT_LIST_DIR}/${tgt}.c")
    endif()
    
    if("${generator_type}" STREQUAL "LEG")
        if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${tgt}.leg) # Check if the leg exists
            list(APPEND sources ${PARSER_SRC_DIR}/${tgt}.leg.c)
        endif()
    endif()
    
    foreach(src ${ARGN})
        list(APPEND sources ${src})
    endforeach()
    
    # Add executable for the example
    add_executable(${tgt} ${sources})
    add_dependencies(${tgt} ${srcs_tgt})

    # Generate a running script
    set(run_script ${EXAMPLE_SCRIPT_DIR}/run_${tgt}.${script_ext})

    if (parse_type STREQUAL "STRING")
        set(file_command "\"${ECHO}\" '${parse_arg}'")
    elseif(parse_type STREQUAL "FILE")
        set(file_command "\"${CAT}\" '${parse_arg}'")
    else()
        message(FATAL_ERROR "Unknown test input type: ${parse_type} (input='${parse_arg}')")
    endif()

    configure_file("run_example.${script_ext}.in" ${run_script}
                    @ONLY NEWLINE_STYLE ${script_nl})

    add_custom_target(run_${tgt} COMMAND ${run_script} "$<TARGET_FILE:${tgt}>"
                                 DEPENDS ${srcs_tgt} ${tgt}
                                 WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

    add_test(NAME ${tgt}
             COMMAND ${run_script} "$<TARGET_FILE:${tgt}>"
             WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

    if(TARGET ${check_target})
        add_dependencies(${check_target} ${tgt})
    endif()
endfunction(add_test_ex)

generate_parsers(parser_sources ${PARSER_SRC_DIR}
                                PEG accept.peg
                                    dc.peg 
                                    dcv.peg
                                    left.peg
                                    rule.peg
                                    simple.peg
                                LEG basic.leg
                                    calc.leg
                                    erract.leg
                                    localleg.leg
                                    username.leg
                                    wc.leg)

add_custom_target(examples ALL)

add_test_ex(PEG examples simple     parser_sources  STRING  "ab.ac.ad.ae.afg.afh.afg.afh.afi.afj.")
add_test_ex(PEG examples rule       parser_sources  STRING  "abcbcdabcbcdabcbcdabcbcd")
add_test_ex(PEG examples accept     parser_sources  STRING  "abcbcdabcbcdabcbcdabcbcd")
add_test_ex(PEG examples dc         parser_sources  STRING  "  2  *3 *(3+ 4) ")
add_test_ex(PEG examples dcv        parser_sources  STRING  "a = 6;  b = 7;  a * b")
add_test_ex(PEG examples localpeg   parser_sources  STRING  "ab.ac.ad.ae.afg.afh.afg.afh.afi.afj." ${PARSER_SRC_DIR}/simple.peg.c)

add_test_ex(LEG examples wc         parser_sources  FILE    "wc.leg")
add_test_ex(LEG examples calc       parser_sources  STRING  "a = 6;  b = 7;  a * b")
# add_test_ex(LEG examples basic      parser_sources    FILE    "basic.bas") # Seems to be broken?
add_test_ex(LEG examples localleg   parser_sources  FILE    "localleg.leg")
add_test_ex(LEG examples erract     parser_sources  STRING  "6*9")

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} -C "${CMAKE_BUILD_TYPE}" --output-on-failure
                        DEPENDS examples parser_sources
                        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                        VERBATIM)