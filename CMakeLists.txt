cmake_minimum_required(VERSION 3.30)
project(GameOfUr_C C)

set(CMAKE_C_STANDARD 11)

include_directories("include")
include_directories("config")

set(SOURCES_Game
        src/common.c
        src/state.c
        src/sim.c
        src/visualize.c)

set(SOURCES_Test
        test/main.c)

add_executable(GameOfUr_C main.c ${SOURCES_Game})
add_executable(GameOfUr_C_Test ${SOURCES_Game} ${SOURCES_Test})
