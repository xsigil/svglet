cmake_minimum_required(VERSION 3.16)
project(svglet VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(2GEOM REQUIRED 2geom)
pkg_check_modules(LIBXML2 REQUIRED libxml-2.0)

find_package(Boost REQUIRED)
find_package(CLI11 REQUIRED)

include_directories(
    src/include
    ${2GEOM_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
)

add_executable(svglet
    src/main.cpp
    src/translate.cpp
    src/rotate.cpp
    src/path_length.cpp
    src/util.cpp
)

target_link_libraries(svglet PRIVATE
    ${2GEOM_LIBRARIES}
    ${LIBXML2_LIBRARIES}
    CLI11::CLI11
)

install(TARGETS svglet DESTINATION bin)
