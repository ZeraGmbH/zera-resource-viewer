cmake_minimum_required(VERSION 3.1)

project(zera-resource-viewer LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(resource_viewer_VERSION_MAJOR "0")
set(resource_viewer_VERSION_MINOR "10")
set(resource_viewer_VERSION_PATCH "0")
set(PROJECT_VERSION "${resource_viewer_VERSION_MAJOR}.${resource_viewer_VERSION_MINOR}.${resource_viewer_VERSION_PATCH}")

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MAJOR_VER "5" CACHE STRING "Qt major version")
set(QTX "Qt${QT_MAJOR_VER}")

find_package(${QTX} COMPONENTS Core Gui Widgets Network CONFIG REQUIRED)
if("${QT_MAJOR_VER}" STREQUAL "6")
    find_package(${QTX} COMPONENTS StateMachine REQUIRED)
endif()
find_package(VeinFramework REQUIRED)
find_package(SCPI REQUIRED)

file(GLOB FORMS
    ${PROJECT_SOURCE_DIR}/*.ui
    )

file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

add_executable(zera-resource-viewer
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    ${FORMS}
    )

target_link_libraries(zera-resource-viewer
    PUBLIC
    ${QTX}::Core
    ${QTX}::Gui
    ${QTX}::Widgets
    ${QTX}::Network
    PRIVATE
    Zera::xiqnet
    Zera::zeraprotobuf
    Zera::SCPI
    )

if("${QT_MAJOR_VER}" STREQUAL "6")
    target_link_libraries(zera-resource-viewer
        PUBLIC
        ${QTX}::StateMachine
    )
endif()

install(TARGETS zera-resource-viewer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
