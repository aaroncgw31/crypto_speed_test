cmake_minimum_required(VERSION 2.8.9)

file(GLOB SOURCES *.c)

add_library(Fixengine ${SOURCES})
add_subdirectory(auth)
include_directories(fixengine ..)
target_link_libraries(Fixengine -L../auth -lz -lcryptopp Auth)
target_compile_options(Fixengine PRIVATE -std=gnu99 -ggdb)




