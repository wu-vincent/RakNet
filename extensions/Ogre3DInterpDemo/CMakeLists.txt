if(NOT UNIX)
    return()
endif()

find_package(OGRE QUIET)
if(NOT OGRE_FOUND)
    message(STATUS "OGRE not found -- Ogre3DInterpDemo will not be built")
    return()
endif()

find_package(OIS QUIET)
if(NOT OIS_FOUND)
    message(STATUS "OIS not found -- Ogre3DInterpDemo will not be built")
    return()
endif()

file(GLOB ALL_CPP_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB ALL_HEADER_SRCS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(Ogre3DInterpDemo ${ALL_CPP_SRCS} ${ALL_HEADER_SRCS})
target_include_directories(Ogre3DInterpDemo PRIVATE ${OGRE_INCLUDE_DIR} ${OIS_INCLUDE_DIR})
target_link_libraries(Ogre3DInterpDemo PRIVATE RakNet ${OGRE_LIBRARIES} ${OIS_LIBRARIES})
set_target_properties(Ogre3DInterpDemo PROPERTIES FOLDER "Samples/3D Demos")
target_compile_options(Ogre3DInterpDemo PRIVATE -w)
