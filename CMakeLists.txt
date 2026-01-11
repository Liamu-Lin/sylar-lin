cmake_minimum_required(VERSION 3.10)
project(sylar)
enable_language(ASM)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 \
    -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-unused-variable -Wno-builtin-macro-redefined")

include_directories(./sylar)

set(LIB_SRC
    #sylar/coroutine.S
    #sylar/coroutine.cpp

    sylar/log.cpp
    sylar/util.cpp
    sylar/config.cpp
    sylar/thread.cpp
    sylar/mutex.cpp
    sylar/fiber.cpp
    sylar/swap_context.S
    sylar/scheduler.cpp
    sylar/iomanager.cpp
    sylar/timer.cpp
    sylar/hook.cpp
    sylar/fdmanager.cpp
    sylar/address.cpp
    sylar/socket.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)


set(LIBS
    sylar
    yaml-cpp
    pthread
    dl
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

add_executable(test_util tests/test_util.cpp)
force_redefine_file_macro_for_sources(test_util)
add_dependencies(test_util sylar)
target_link_libraries(test_util ${LIBS})

add_executable(test_fiber tests/test_fiber.cpp)
force_redefine_file_macro_for_sources(test_fiber)
add_dependencies(test_fiber sylar)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cpp)
force_redefine_file_macro_for_sources(test_scheduler)
add_dependencies(test_scheduler sylar)
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cpp)
force_redefine_file_macro_for_sources(test_iomanager)
add_dependencies(test_iomanager sylar)
target_link_libraries(test_iomanager ${LIBS})

add_executable(test_hook tests/test_hook.cpp)
force_redefine_file_macro_for_sources(test_hook)
add_dependencies(test_hook sylar)
target_link_libraries(test_hook ${LIBS})

add_executable(test_address tests/test_address.cpp)
force_redefine_file_macro_for_sources(test_address)
add_dependencies(test_address sylar)
target_link_libraries(test_address ${LIBS})

add_executable(test_socket tests/test_socket.cpp)
force_redefine_file_macro_for_sources(test_socket)
add_dependencies(test_socket sylar)
target_link_libraries(test_socket ${LIBS})

#add_executable(test_routine tests/test_routine.cpp)
#force_redefine_file_macro_for_sources(test_routine)
#add_dependencies(test_routine sylar)
#target_link_libraries(test_routine ${LIBS})




SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)