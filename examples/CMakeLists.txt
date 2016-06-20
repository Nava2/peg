
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

function(add_example all_target generator_target target_name parse_type parse_arg )

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

    set(gen_file_base "${target_name}.${generator_target}")
    set(gen_file ${GENERATED_SOURCE_DIR}/examples/${gen_file_base}.c)

    set(gen_target  "example.${target_name}.generate")
    set(exec_target "example.${target_name}.exec")

    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${gen_file_base})
        add_custom_command(OUTPUT ${gen_file}
                        VERBATIM
                        COMMAND "$<TARGET_FILE:${generator_target}>" -o "${gen_file}" "${gen_file_base}"
                        DEPENDS ${generator_target}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
                        COMMENT "Generating test sources: ${gen_file}")
        add_custom_target(${gen_target} DEPENDS ${gen_file})

        set_source_files_properties(${gen_file} PROPERTIES GENERATED true)
    endif()

    set(srcs)
    if("${generator_target}" STREQUAL "peg")
        list(APPEND srcs ${CMAKE_CURRENT_LIST_DIR}/${target_name}.c)
    elseif("${generator_target}" STREQUAL "leg")
        list(APPEND srcs ${gen_file})
    endif()

    include_directories(${GENERATED_SOURCE_DIR}/examples)
    add_executable(${exec_target} ${srcs})
    if(TARGET ${gen_target})
        add_dependencies(${exec_target} ${gen_target})
    endif(TARGET ${gen_target})

    # Generate a running script
    set(run_script ${GENERATED_SOURCE_DIR}/examples/run_${target_name}.${script_ext})
    set(gen_exec_path ${exec_target})

    if (parse_type STREQUAL "STRING")
        set(file_command "\"${ECHO}\" -e '${parse_arg}'")
    elseif(parse_type STREQUAL "FILE")
        set(file_command "\"${CAT}\" '${parse_arg}'")
    else()
        message(FATAL_ERROR "Unknown test input type: ${parse_type} (input='${parse_arg}')")
    endif()

    configure_file("run_example.${script_ext}.in" ${run_script}
                    @ONLY NEWLINE_STYLE ${script_nl})

    add_custom_target(example.${target_name} 
                        DEPENDS ${exec_target})

    add_test(NAME ${target_name}
             COMMAND "${run_script}"
             WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR})

    if(TARGET ${all_target})
        add_dependencies(${all_target} example.${target_name})
    endif()
endfunction(add_example)

add_custom_target(examples)

add_example(examples    peg simple      STRING  "ab.ac.ad.ae.afg.afh.afg.afh.afi.afj.")
add_example(examples    peg rule        STRING  "abcbcdabcbcdabcbcdabcbcd")
add_example(examples    peg accept      STRING  "abcbcdabcbcdabcbcdabcbcd")
add_example(examples    peg dc          STRING  "  2  *3 *(3+ 4) ")
add_example(examples    peg dcv         STRING  "a = 6;  b = 7;  a * b")
add_example(examples    peg localpeg    STRING  "ab.ac.ad.ae.afg.afh.afg.afh.afi.afj.")
add_dependencies(example.localpeg example.simple)

add_example(examples    leg wc          FILE    "wc.leg")
add_example(examples    leg calc        STRING  "a = 6;  b = 7;  a * b")
# add_example(examples    leg basic       FILE    "basic.bas") # Seems to be broken?
add_example(examples    leg localleg    FILE    "localleg.leg")
add_example(examples    leg erract      STRING  "6*9")

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} DEPENDS examples)