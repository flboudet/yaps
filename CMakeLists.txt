cmake_minimum_required (VERSION 2.6)
project (yaps)
add_executable(yaps yaps_rcu.c test_2.cc)
target_link_libraries(yaps pthread)
