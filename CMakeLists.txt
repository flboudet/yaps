cmake_minimum_required (VERSION 2.6)
project (yaps)

find_package(GTest REQUIRED)

add_executable(yaps yaps_rcu.c test_2.cc test_3.cc)

target_include_directories(yaps PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(yaps ${GTEST_LIBRARIES} pthread)
