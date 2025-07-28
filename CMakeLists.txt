cmake_minimum_required(VERSION 3.10)

project(2satPlanarDrawing VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/inputs")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/inputs/svg")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/inputs/gml")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/outputs")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/outputs/svg")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/outputs/gml")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/graphs/outputs/log")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/profiling")
file(COPY "${CMAKE_SOURCE_DIR}/run" DESTINATION "${CMAKE_BINARY_DIR}")

add_custom_target(copy_resources ALL
               COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/resources
                       ${CMAKE_BINARY_DIR}/graphs/inputs/gml
               COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/src/profiling
                       ${CMAKE_BINARY_DIR}/profiling
               COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/src/profiling
                       ${CMAKE_BINARY_DIR}/profiling
           )

include_directories(include)
include_directories(tracy/public/tracy)
include_directories(/home/medkha/.local/share/ogdf/include)

link_directories(/home/medkha/.local/share/ogdf)

option( TRACY_ENABLE "" ON)
add_subdirectory(tracy)

option( TRACY_ON_DEMAND "" ON)

add_executable(2SATEquivalenceReduction src/main.cpp src/algorithm.cpp src/utils.cpp src/TracyClient.cpp)

target_link_libraries(2SATEquivalenceReduction
    OGDF 
    COIN
    Tracy::TracyClient
)


