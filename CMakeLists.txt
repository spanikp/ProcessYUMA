cmake_minimum_required(VERSION 3.12.0)
project(ProcessYUMA VERSION 0.1.0)

add_executable(ProcessYUMA ProcessYUMA.cpp)
set_property(TARGET ProcessYUMA PROPERTY CXX_STANDARD 17)

include_directories("/usr/local/include/gpstk")
find_library(GPSTK_LIB gpstk)
target_link_libraries(ProcessYUMA PRIVATE "${GPSTK_LIB}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
