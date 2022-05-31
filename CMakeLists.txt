cmake_minimum_required(VERSION 3.0.2)

project(pulse-sandbox)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Pulse REQUIRED IMPORTED_TARGET libpulse-simple)

add_executable(main main.cpp)
target_link_libraries(main PRIVATE PkgConfig::Pulse)
