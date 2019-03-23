cmake_minimum_required(VERSION 2.8.9)

file(GLOB SOURCES *.cpp)

add_executable(speedtest ${SOURCES})
add_subdirectory(fixengine)
target_link_libraries(speedtest -L../fixengine Fixengine -lz -pthread -Wl,-Bstatic websockets ssl crypto -Wl,-Bdynamic dl)
target_compile_options(speedtest PRIVATE -std=c++11 -ggdb)


