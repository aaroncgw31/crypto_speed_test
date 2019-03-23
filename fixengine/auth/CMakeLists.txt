cmake_minimum_required(VERSION 2.8.9)

file(GLOB SOURCES *.cpp)

add_library(Auth ${SOURCES})
target_link_libraries(Auth -lcryptopp)
target_compile_options(Auth PRIVATE -std=c++11 -ggdb)






