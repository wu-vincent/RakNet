cmake_minimum_required(VERSION 3.14)
project(RakNet VERSION 4.081 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)
option(RAKNET_ENABLE_SAMPLES "Generate RakNet sample projects if true." OFF)
option(RAKNET_ENABLE_EXTENSIONS "Enable extension projects that might need specific dependencies." OFF)
option(RAKNET_ENABLE_TESTS "Build RakNet test suite" OFF)

# Build targets
add_subdirectory(src)

if(RAKNET_ENABLE_EXTENSIONS)
    add_subdirectory(extensions)
endif()

if(RAKNET_ENABLE_SAMPLES)
    add_subdirectory(samples)
endif()

if(RAKNET_ENABLE_TESTS)
    add_subdirectory(tests)
endif()

# Install & export infrastructure
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(DIRECTORY include/RakNet/
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
