cmake_minimum_required(VERSION 3.16)

project(artnet VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt 核心模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 查找网络模块
if(${QT_VERSION_MAJOR} EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
endif()

# 检查是否成功找到网络模块
if(NOT Qt${QT_VERSION_MAJOR}Network_FOUND)
    message(FATAL_ERROR "Qt Network module not found!")
endif()

set(PROJECT_SOURCES
      main.cpp
      widget.h
      widget.cpp
      widget.ui
      # QledWidget.h
      # QledWidget.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(artnet
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        # QledWidget.h
        # QledWidget.cpp
        # QledWidget.ui
    )
else()
    if(ANDROID)
        add_library(artnet SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(artnet
            ${PROJECT_SOURCES}
        )
    endif()
endif()

# 链接 Qt 核心模块和网络模块
target_link_libraries(artnet PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

# macOS 和 iOS 配置
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.artnet)
endif()
set_target_properties(artnet PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

# 安装配置
include(GNUInstallDirs)
install(TARGETS artnet
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Qt 6 的最终化配置
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(artnet)
endif()
