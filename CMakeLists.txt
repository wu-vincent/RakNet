cmake_minimum_required(VERSION 3.14)
project(RakNet VERSION 4.082 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)
option(RAKNET_ENABLE_SAMPLES "Generate RakNet sample projects if true." ON)
option(RAKNET_ENABLE_EXTENSIONS "Enable extension projects that might need specific dependencies." ON)
option(RAKNET_GENERATE_INCLUDE_ONLY_DIR "Setup a include/RakNet/ directory in which all the headers are copied." OFF)

# Module path for bundled Find*.cmake modules
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/CmakeIncludes"
    "${CMAKE_CURRENT_SOURCE_DIR}/CmakeIncludes/Ogre3D"
)

# Include the raknet_add_sample() helper
include(${CMAKE_CURRENT_SOURCE_DIR}/CmakeIncludes/CmakeMacros.txt)

# Optional: copy headers to include/RakNet/
if(RAKNET_GENERATE_INCLUDE_ONLY_DIR)
    set(RAKNET_NAMED_INCLUDE_ONLY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/RakNet")
    message(STATUS "Setting up the ${RAKNET_NAMED_INCLUDE_ONLY_DIR} directory...")
    file(MAKE_DIRECTORY "${RAKNET_NAMED_INCLUDE_ONLY_DIR}")
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/Source/" DESTINATION "${RAKNET_NAMED_INCLUDE_ONLY_DIR}" FILES_MATCHING PATTERN "*.h")
    message(STATUS "DONE: Setting up the ${RAKNET_NAMED_INCLUDE_ONLY_DIR} directory.")
endif()

# Build targets
add_subdirectory(Lib)

if(RAKNET_ENABLE_EXTENSIONS)
    add_subdirectory(DependentExtensions)
endif()

if(RAKNET_ENABLE_SAMPLES)
    add_subdirectory(Samples)
endif()

# Install & export infrastructure
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY Source/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/RakNet"
    FILES_MATCHING PATTERN "*.h"
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/RakNetConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/RakNetConfig.cmake"
    INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RakNet"
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/RakNetConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    EXPORT RakNetTargets
    FILE RakNetTargets.cmake
    NAMESPACE RakNet::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RakNet"
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/RakNetConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/RakNetConfigVersion.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/RakNet"
)
