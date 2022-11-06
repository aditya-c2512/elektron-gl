cmake_minimum_required(VERSION 3.10)

project(ElektronGL VERSION 0.1)

add_library(geometry STATIC src/geometry.cpp)
add_library(tgaimage STATIC src/tgaimage.cpp)
add_library(model STATIC src/model.cpp)

target_include_directories(geometry PUBLIC include)
target_include_directories(tgaimage PUBLIC include)
target_include_directories(model PUBLIC include)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE geometry)
target_link_libraries(main PRIVATE tgaimage)
target_link_libraries(main PRIVATE model)