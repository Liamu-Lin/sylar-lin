function(force_redefine_file_macro_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        # Get the relative path of the source file in project directory
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # Set the updated compile definitions on the source file.
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach()
endfunction()

function(ragelmaker src_rl outputdir)
    get_filename_component(src_file ${src_rl} NAME_WE)
    set(rl_out ${outputdir}/${src_file}.rl.cpp)

    add_custom_command(
        OUTPUT ${rl_out}
        COMMAND ragel ${src_rl} -o ${rl_out} -l -C -G2 --error-format=msvc
        DEPENDS ${src_rl}
        WORKING_DIRECTORY ${outputdir}
    )

    set_source_files_properties(${rl_out} PROPERTIES GENERATED TRUE)
endfunction()
