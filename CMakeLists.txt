cmake_minimum_required(VERSION 3.16)

# Project Name and Version
project(SwiftCatch VERSION 1.0 LANGUAGES CXX)

# Require C++17 or higher
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 Package
find_package(Qt6 REQUIRED COMPONENTS Widgets Core Charts)

# Add Executable Target
add_executable(SwiftCatch main.cpp mainwindow.cpp weatherdata.cpp geodatadownloader.cpp hydrodownloader.cpp hydrodownloaderdlg.cpp weatherdownloaderdlg.cpp)

# Add Include Directories
target_include_directories(SwiftCatch PRIVATE ${Qt6Widgets_INCLUDE_DIRS} ${Qt6Core_INCLUDE_DIRS} ${Qt6Charts_INCLUDE_DIRS})

# Link Qt6 Libraries
target_link_libraries(SwiftCatch PRIVATE Qt6::Widgets Qt6::Core Qt6::Charts)

# Add UI Files (if using .ui forms)
qt_add_executable(Qt6Example
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    hydrodownloaderdlg.cpp
    hydrodownloaderdlg.h
    hydrodownloaderdlg.ui
    weatherdownloaderdlg.cpp
    weatherdownloaderdlg.h
    weatherdownloaderdlg.ui
)

