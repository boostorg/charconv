# Copyright 2023 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# See: https://devblogs.microsoft.com/cppblog/registries-bring-your-own-libraries-to-vcpkg/

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cppalliance/charconv
        REF v1.1.0
        SHA512 13d248f22a3c14a8c4950b2a88db133604b2611c6d171f9eac72c93fb97d09f1b2ca89852a7476696c006d0bff79b749e25368955b7b39568ac6a46c2294e068
        HEAD_REF master
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
