cmake_minimum_required(VERSION 3.0)
project(sylar)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 \
    -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-unused-variable -Wno-builtin-macro-redefined")

include_directories(./sylar)

set(LIB_SRC
    sylar/log.cpp
    sylar/util.cpp
    sylar/config.cpp
    sylar/thread.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)


set(LIBS
    sylar
    yaml-cpp
    pthread
    )

add_executable(test tests/test.cpp)
force_redefine_file_macro_for_sources(test)
add_dependencies(test sylar)
target_link_libraries(test ${LIBS})

add_executable(test_config tests/test_config.cpp)
force_redefine_file_macro_for_sources(test_config)
add_dependencies(test_config sylar)
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/test_thread.cpp)
force_redefine_file_macro_for_sources(test_thread)
add_dependencies(test_thread sylar)
target_link_libraries(test_thread ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)