cmake_minimum_required(VERSION 3.16)
project(OrderBook VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CTest)
include(CPack)

add_subdirectory(src)
add_subdirectory(unit_tests)