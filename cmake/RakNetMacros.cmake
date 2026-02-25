# Modern helper function for RakNet sample projects

function(raknet_add_sample DIR)
    cmake_parse_arguments(SAMPLE "" "NAME;FOLDER" "EXTRA_SOURCES;EXTRA_INCLUDES;EXTRA_LIBS" ${ARGN})

    # Derive executable name from directory name if NAME not given
    if(NOT SAMPLE_NAME)
        get_filename_component(SAMPLE_NAME "${DIR}" NAME)
        # Replace spaces with underscores -- CMake target names cannot contain spaces
        string(REPLACE " " "_" SAMPLE_NAME "${SAMPLE_NAME}")
    endif()

    if(NOT SAMPLE_FOLDER)
        set(SAMPLE_FOLDER "Samples")
    endif()

    file(GLOB _sample_cpp CONFIGURE_DEPENDS "${DIR}/*.cpp")
    file(GLOB _sample_c   CONFIGURE_DEPENDS "${DIR}/*.c")
    file(GLOB _sample_h   CONFIGURE_DEPENDS "${DIR}/*.h")

    add_executable(${SAMPLE_NAME}
        ${_sample_cpp}
        ${_sample_c}
        ${_sample_h}
        ${SAMPLE_EXTRA_SOURCES}
    )

    target_link_libraries(${SAMPLE_NAME} PRIVATE RakNet ${SAMPLE_EXTRA_LIBS})

    if(SAMPLE_EXTRA_INCLUDES)
        target_include_directories(${SAMPLE_NAME} PRIVATE ${SAMPLE_EXTRA_INCLUDES})
    endif()

    # Per-target warning suppression
    if(MSVC)
        target_compile_definitions(${SAMPLE_NAME} PRIVATE
            _CRT_SECURE_NO_DEPRECATE
            _CRT_NONSTDC_NO_DEPRECATE
        )
        target_compile_options(${SAMPLE_NAME} PRIVATE /W0)
    else()
        target_compile_options(${SAMPLE_NAME} PRIVATE -w)
    endif()

    set_target_properties(${SAMPLE_NAME} PROPERTIES FOLDER "${SAMPLE_FOLDER}")
endfunction()
