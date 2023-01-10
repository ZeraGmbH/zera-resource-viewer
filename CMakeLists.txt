cmake_minimum_required(VERSION 3.1)

# set project name and language
project(zera-resource-viewer LANGUAGES CXX)

# automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# project Version
set(resman_VERSION_MAJOR "0")
set(resman_VERSION_MINOR "10")
set(resman_VERSION_PATCH "0")
set(PROJECT_VERSION "${resman_VERSION_MAJOR}.${resman_VERSION_MINOR}.${resman_VERSION_PATCH}")

# common cmake functions
include(FeatureSummary)
include(GNUInstallDirs)

# find dependecies
find_package(Qt5 COMPONENTS Core Gui Widgets Network CONFIG REQUIRED)

find_package(xiqnet REQUIRED)
find_package(zeraprotobuf REQUIRED)
find_package(SCPI REQUIRED)

file(GLOB FORMS
    ${PROJECT_SOURCE_DIR}/.ui
    )

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

# create executable
add_executable(zera-resourcemanager
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    ${FORMS}
    )

# link dependencies
target_link_libraries(zera-resourcemanager
    PUBLIC
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
    PRIVATE
    VeinMeta::xiqnet
    VeinMeta::zeraprotobuf
    Zera::SCPI
    )
