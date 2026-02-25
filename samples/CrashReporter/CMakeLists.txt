if(WIN32)
    add_executable(CrashReporter
        CrashReporter.cpp CrashReporter.h
        SendFileTo.cpp SendFileTo.h
        main.cpp
    )
    target_link_libraries(CrashReporter PRIVATE RakNet Dbghelp)
endif()
