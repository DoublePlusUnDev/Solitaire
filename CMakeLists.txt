cmake_minimum_required(VERSION 3.10)

project(Solitare VERSION 1.0)

#Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

#Add executables
add_executable(Solitaire
    src/cards.c
    src/gamestate.c
    src/input.c
    src/leaderboard.c
    src/logging.c
    src/rendering.c
    src/Solitaire.c
    )

#Include headers
target_include_directories(Solitaire PUBLIC include)

find_package(Curses REQUIRED)
target_link_libraries(Solitaire PRIVATE curses)
target_compile_options(Solitaire PRIVATE -Wall -Werror)
target_compile_definitions(Solitaire PRIVATE NCURSES_STATIC)
