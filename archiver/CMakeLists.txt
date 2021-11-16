cmake_minimum_required(VERSION 3.00)

project(Archiver C)

set(CMAKE_C_STANDARD 90)

add_executable(rxar
        rxarmain.c
        rxaradd.c rxaradd.h
        rxarlist.c rxarlist.h
        rxardel.c rxardel.h)

target_link_libraries(rxar common)

target_include_directories(rxar PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../platform)
