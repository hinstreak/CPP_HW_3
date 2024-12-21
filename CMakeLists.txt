cmake_minimum_required(VERSION 3.26)

project(cpp_homework_fluid)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS -O4)
add_definitions("-DTYPES=FLOAT,DOUBLE,FIXED(32,16),FAST_FIXED(48,16)")
add_definitions("-DSIZES=S(24,84),S(50,50)")

include_directories("headers/")


add_executable(main main.cpp
        source/InfoF.cpp
        source/ParsingSettings.cpp
)