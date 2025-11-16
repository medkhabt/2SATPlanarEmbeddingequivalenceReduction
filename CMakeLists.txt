cmake_minimum_required(VERSION 3.10)

project(2satPlanarDrawing VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(OGDF_BUILD_DIR "/home/medkha/.local/share/ogdf" CACHE STRING "the path to the build of OGDF project")


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
file(GLOB ALG_SRC_FILES "${CMAKE_SOURCE_DIR}/src/algorithms/*.cpp")

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
include_directories(include/algorithms)
include_directories(${OGDF_BUILD_DIR}/include)

link_directories(${OGDF_BUILD_DIR})

option(BUILD_TESTING "Build tests" OFF)
option(BUILD_PROFILING "Build Profiling" OFF)

if(BUILD_TESTING)
    include(CTest) 
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip 
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests/unit)
endif()
#add_executable(2SATEquivalenceReduction src/main.cpp src/algorithm.cpp src/utils.cpp src/TracyClient.cpp)
add_executable(2SATEquivalenceReduction src/main.cpp src/utils.cpp  src/GraphBuilder.cpp ${ALG_SRC_FILES})

#[===[
target_compile_options(2SATEquivalenceReduction PRIVATE
    -Wall
    -Wextra
    -pedantic
)
]===]
#
target_link_libraries(2SATEquivalenceReduction
    OGDF 
    COIN
)


if(BUILD_PROFILING)
    option(TRACY_ENABLE "" ON)
    add_subdirectory(tracy)

    option( TRACY_ON_DEMAND "" ON)

target_link_libraries(2SATEquivalenceReduction Tracy::TracyClient)
endif()
