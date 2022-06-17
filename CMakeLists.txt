cmake_minimum_required(VERSION 3.0.2)

project(pulse-sandbox)

find_package(PkgConfig REQUIRED)
pkg_check_modules(Pulse REQUIRED IMPORTED_TARGET libpulse)

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

set(SOURCES
  AsioPulse.cpp
  PulseContext.cpp
)

add_executable(main main.cpp ${SOURCES})
target_link_libraries(main PRIVATE PkgConfig::Pulse Boost::boost Threads::Threads)
