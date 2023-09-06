# Copyright 2023 Matt Borland
# Distributed under the Boost Software License, Version 1.0.
# https://www.boost.org/LICENSE_1_0.txt
#
# See: https://devblogs.microsoft.com/cppblog/registries-bring-your-own-libraries-to-vcpkg/

vcpkg_from_github(
        OUT_SOURCE_PATH SOURCE_PATH
        REPO cppalliance/charconv
        REF v1.0.0
        SHA512 ced9e5ca45df285709e19ac0a142b58447bcff91d5a14ffcc3b7d6686120dca631497aa145508fd1f2c4ffea4d7b6bf50397fb86f3eafec7ab281014aee8f6b5
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
