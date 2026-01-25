cmake_minimum_required(VERSION 4.1)
project(cpp_filemanager)

set(CMAKE_CXX_STANDARD 17)

add_library(cpp_filemanager STATIC
        include/filemanager.h
        samples/test.cpp)
