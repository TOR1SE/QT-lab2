cmake_minimum_required(VERSION 3.16)

project(SimpleTextEditor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(TextEditor WIN32
    src/main.cpp
    src/mainwindow.h src/mainwindow.cpp src/mainwindow.ui
    src/codeeditor.h src/codeeditor.cpp
    src/finddialog.h src/finddialog.cpp
    src/replacedialog.h src/replacedialog.cpp
    src/aboutdialog.h src/aboutdialog.cpp
    res/icons.qrc
)

target_include_directories(TextEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(TextEditor PRIVATE Qt6::Widgets)
