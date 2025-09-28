add_executable(unit_tests
  test_2SAT_computation.cpp
  test_local_transitivity.cpp
  test_contribution1.cpp
  test_random_graphs.cpp
  ${CMAKE_SOURCE_DIR}/src/algorithmSimplev1.cpp
  ${CMAKE_SOURCE_DIR}/src/utils.cpp   
)
target_include_directories(unit_tests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(unit_tests GTest::gtest GTest::gtest_main OGDF Tracy::TracyClient)
# target_compile_options(unit_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -g)
target_link_options(unit_tests PRIVATE -fsanitize=address,undefined)
include(GoogleTest)
gtest_discover_tests(unit_tests)
add_custom_command(TARGET unit_tests POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/tests/unit/graphs
          ${CMAKE_BINARY_DIR}/tests/unit/graphs)
