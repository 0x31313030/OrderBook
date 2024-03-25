include(GoogleTest)
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

#add_definitions(-D_GLIBCXX_DEBUG )
add_definitions(-DNDEBUG )

set(CMAKE_CXX_STANDARD 20)

if(NOT MSVC)
  add_compile_options(
      -std=c++2a
      -W
      -Wall 
  )
endif()

enable_testing()

add_executable(
  OrderBookTests
  OrderBookTests.cpp
)

target_link_libraries(
  OrderBookTests
  orderbook
  GTest::gtest_main
)

target_include_directories(OrderBookTests PUBLIC ${PROJECT_SOURCE_DIR}/src)
#target_compile_options(problem1Tests PRIVATE -Wsign-conversion )

gtest_discover_tests(OrderBookTests)
