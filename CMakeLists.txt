cmake_minimum_required (VERSION 2.6)

project (Koura)

include_directories("." "ext/Catch/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z")

add_executable(koura_test
    tests/koura_test.cpp)

add_executable(testy
        tests/test.cpp)

enable_testing()
add_test(koura_test koura_test)

set(STANDARDESE_TOOL ext/standardese/build/tool/standardese)
include(ext/standardese/standardese-config.cmake)

# generates a custom target that will run standardese to generate the documentation
standardese_generate(my_target
                     INCLUDE_DIRECTORY .
                     CONFIG standardese-config
                     INPUT koura.hpp)
