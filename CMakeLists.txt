cmake_minimum_required(VERSION 3.10.0)
project(Practice VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

file(GLOB SRCS src/*.cpp)

qt_add_executable(Practice ${SRCS})

target_link_libraries(Practice PRIVATE Qt6::Widgets d3dcompiler)

