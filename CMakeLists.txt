cmake_minimum_required(VERSION 3.22 FATAL_ERROR)

include(CheckCXXCompilerFlag)

project(unnamed)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 20)

find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED
)

qt_add_executable(
  unnamed
  src/Algorithms.cpp
  src/Graphics.cpp
  src/MainWindow.cpp
  src/MainWindow.ui
  src/Run.cpp
  src/SortItem.cpp
  src/main.cpp
)

target_link_libraries(unnamed PUBLIC Qt::Widgets)

target_include_directories(unnamed PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")

check_cxx_compiler_flag("-Wall" FLAG_WALL_SUPPORTED)
check_cxx_compiler_flag("-Werror" FLAG_WERROR_SUPPORTED)
check_cxx_compiler_flag("-Wextra" FLAG_WEXTRA_SUPPORTED)

if("${FLAG_WALL_SUPPORTED}" EQUAL "1")
  target_compile_options(unnamed PUBLIC "-Wall")
endif()

if("${FLAG_WERROR_SUPPORTED}" EQUAL "1")
  target_compile_options(unnamed PUBLIC "-Werror")
endif()

if("${FLAG_WERROR_SUPPORTED}" EQUAL "1")
  target_compile_options(unnamed PUBLIC "-Wextra")
endif()
