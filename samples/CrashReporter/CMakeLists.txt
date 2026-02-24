if(WIN32)
    raknet_add_sample(${CMAKE_CURRENT_SOURCE_DIR}
        EXTRA_LIBS Dbghelp
    )
endif()
