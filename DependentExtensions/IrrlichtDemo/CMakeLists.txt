find_package(Irrlicht QUIET)
if(NOT IRRLICHT_FOUND)
    message(STATUS "Irrlicht not found -- IrrlichtDemo will not be built")
    return()
endif()

if(NOT WIN32)
    find_package(IrrKlang QUIET)
    if(NOT IRRKLANG_FOUND)
        message(STATUS "IrrKlang not found -- IrrlichtDemo will not be built")
        return()
    endif()
endif()

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_SOURCE_DIR}/samples/PHPDirectoryServer2/PHPDirectoryServer2.cpp"
)
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_SOURCE_DIR}/samples/PHPDirectoryServer2/PHPDirectoryServer2.h"
)

add_executable(IrrlichtDemo WIN32 ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(IrrlichtDemo PRIVATE ${IRRLICHT_INCLUDE_DIR})
target_link_libraries(IrrlichtDemo PRIVATE RakNet)
set_target_properties(IrrlichtDemo PROPERTIES FOLDER "Samples/3D Demos")

if(MSVC)
    target_compile_definitions(IrrlichtDemo PRIVATE _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    target_compile_options(IrrlichtDemo PRIVATE /W0)
else()
    target_compile_options(IrrlichtDemo PRIVATE -w)
    if(IRRKLANG_FOUND)
        target_include_directories(IrrlichtDemo PRIVATE ${IRRKLANG_INCLUDE_DIR})
        target_link_libraries(IrrlichtDemo PRIVATE ${IRRLICHT_LIBRARIES} ${IRRKLANG_LIBRARIES})
    endif()
endif()
