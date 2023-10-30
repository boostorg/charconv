# Copyright 2023 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# See: https://devblogs.microsoft.com/cppblog/registries-bring-your-own-libraries-to-vcpkg/

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cppalliance/charconv
        REF v1.0.1
        SHA512 b1d0accec1a4251f3107061c80e5c72a76aa4de0d5fdc085c934a3da8b55a5e72bb3e6a3a1b51d4e7debe6917a3c64be731d56eb8e7cacb890434a154f140726
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
