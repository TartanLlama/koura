cmake_minimum_required (VERSION 2.6)

project (Koura)

include_directories("." "ext/Catch/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_executable(koura_test
    tests/catch_main.cpp
    tests/builtin_filters.cpp
    tests/custom_filters.cpp
    tests/multiple_tags.cpp                
    tests/hello.cpp)

add_executable(testy
        tests/test.cpp)

enable_testing()
add_test( koura_test koura_test )


