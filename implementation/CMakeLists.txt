cmake_minimum_required(VERSION 3.10)

project(2satPlanarDrawing VERSION 1.0)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_custom_target(copy_resources ALL
               COMMAND ${CMAKE_COMMAND} -E copy_directory
               ${CMAKE_SOURCE_DIR}/resources
               ${CMAKE_BINARY_DIR}
           )

include_directories(include)
include_directories(/home/medkha/.local/share/ogdf/include)
link_directories(/home/medkha/.local/share/ogdf)

add_executable(Test src/main.cpp)

target_link_libraries(Test
    OGDF 
    COIN
)


