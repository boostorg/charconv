# Copyright 2023 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# See: https://devblogs.microsoft.com/cppblog/registries-bring-your-own-libraries-to-vcpkg/

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cppalliance/charconv
        REF e709ea5705e88faefb1dba9f12108b341f3e44c2
        SHA512 3d6f0fdc7447366e2fa135ff31138db2ce6309fb066240a37529f1ea87ad0c1ed79a1c22d38ceeb6f1f7e7353bc928a0e8702d615c20e12f27f6c336f6a83ad6
        HEAD_REF develop
)

vcpkg_replace_string("${SOURCE_PATH}/build/Jamfile"
        "import ../../config/checks/config"
        "import ../config/checks/config"
)

file(COPY "${CURRENT_INSTALLED_DIR}/share/boost-config/checks" DESTINATION "${SOURCE_PATH}/config")
include(${CURRENT_HOST_INSTALLED_DIR}/share/boost-build/boost-modular-build.cmake)
boost_modular_build(
        SOURCE_PATH ${SOURCE_PATH}
        BOOST_CMAKE_FRAGMENT "${CMAKE_CURRENT_LIST_DIR}/b2-options.cmake"
)
include(${CURRENT_INSTALLED_DIR}/share/boost-vcpkg-helpers/boost-modular-headers.cmake)
boost_modular_headers(SOURCE_PATH ${SOURCE_PATH})
