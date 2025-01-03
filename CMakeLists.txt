cmake_minimum_required(VERSION 3.16)

project(deepin-theme-tool VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Dtk6 REQUIRED COMPONENTS Widget)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/dcitoolsview.h
    src/dcitoolsview.cpp
    src/funcitem.cpp
    src/funcitem.h
)

add_executable(deepin-theme-tool ${PROJECT_SOURCES})

target_link_libraries(deepin-theme-tool PRIVATE
    Qt6::Widgets
    Dtk6::Widget
)