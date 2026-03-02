cmake_minimum_required(VERSION 3.10)
project(sylar)
enable_language(ASM)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 \
    -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-unused-variable -Wno-builtin-macro-redefined")

include_directories(./sylar)

set(LIB_SRC
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
    sylar/bytearry.cpp
    sylar/http/http.cpp
    sylar/http/http11_parser.rl.cpp
    sylar/http/httpclient_parser.rl.cpp
    sylar/http/http_parser.cpp
    sylar/tcp_server.cpp
    sylar/stream.cpp
    sylar/http/http_session.cpp
    sylar/http/http_server.cpp
    sylar/http/http_servlet.cpp
    sylar/http/http_connection.cpp
    sylar/uri.rl.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(sylar)

ragelmaker(${CMAKE_CURRENT_SOURCE_DIR}/sylar/http/http11_parser.rl ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(${CMAKE_CURRENT_SOURCE_DIR}/sylar/http/httpclient_parser.rl ${CMAKE_CURRENT_SOURCE_DIR}/sylar/http)
ragelmaker(${CMAKE_CURRENT_SOURCE_DIR}/sylar/uri.rl ${CMAKE_CURRENT_SOURCE_DIR}/sylar)

set(LIBS
    sylar
    yaml-cpp
    pthread
    dl
    )

# add_executable(test tests/test.cpp)
# force_redefine_file_macro_for_sources(test)
# add_dependencies(test sylar)
# target_link_libraries(test ${LIBS})

# add_executable(test_config tests/test_config.cpp)
# force_redefine_file_macro_for_sources(test_config)
# add_dependencies(test_config sylar)
# target_link_libraries(test_config ${LIBS})

# add_executable(test_thread tests/test_thread.cpp)
# force_redefine_file_macro_for_sources(test_thread)
# add_dependencies(test_thread sylar)
# target_link_libraries(test_thread ${LIBS})

# add_executable(test_util tests/test_util.cpp)
# force_redefine_file_macro_for_sources(test_util)
# add_dependencies(test_util sylar)
# target_link_libraries(test_util ${LIBS})

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

# add_executable(test_hook tests/test_hook.cpp)
# force_redefine_file_macro_for_sources(test_hook)
# add_dependencies(test_hook sylar)
# target_link_libraries(test_hook ${LIBS})

# add_executable(test_address tests/test_address.cpp)
# force_redefine_file_macro_for_sources(test_address)
# add_dependencies(test_address sylar)
# target_link_libraries(test_address ${LIBS})

# add_executable(test_socket tests/test_socket.cpp)
# force_redefine_file_macro_for_sources(test_socket)
# add_dependencies(test_socket sylar)
# target_link_libraries(test_socket ${LIBS})

# add_executable(test_bytearray tests/test_bytearray.cpp)
# force_redefine_file_macro_for_sources(test_bytearray)
# add_dependencies(test_bytearray sylar)
# target_link_libraries(test_bytearray ${LIBS})

# add_executable(test_http tests/test_http.cpp)
# force_redefine_file_macro_for_sources(test_http)
# add_dependencies(test_http sylar)
# target_link_libraries(test_http ${LIBS})

# add_executable(test_http_parser tests/test_http_parser.cpp)
# force_redefine_file_macro_for_sources(test_http_parser)
# add_dependencies(test_http_parser sylar)
# target_link_libraries(test_http_parser ${LIBS})

# add_executable(test_tcp_server tests/test_tcp_server.cpp)
# force_redefine_file_macro_for_sources(test_tcp_server)
# add_dependencies(test_tcp_server sylar)
# target_link_libraries(test_tcp_server ${LIBS})

# add_executable(echo_server examples/echo_server.cpp)
# force_redefine_file_macro_for_sources(echo_server)
# add_dependencies(echo_server sylar)
# target_link_libraries(echo_server ${LIBS})

# add_executable(test_http_session tests/test_http_session.cpp)
# force_redefine_file_macro_for_sources(test_http_session)
# add_dependencies(test_http_session sylar)
# target_link_libraries(test_http_session ${LIBS})

add_executable(test_http_server tests/test_http_server.cpp)
force_redefine_file_macro_for_sources(test_http_server)
add_dependencies(test_http_server sylar)
target_link_libraries(test_http_server ${LIBS})

add_executable(test_http_connection tests/test_http_connection.cpp)
force_redefine_file_macro_for_sources(test_http_connection)
add_dependencies(test_http_connection sylar)
target_link_libraries(test_http_connection ${LIBS})

add_executable(test_uri tests/test_uri.cpp)
force_redefine_file_macro_for_sources(test_uri)
add_dependencies(test_uri sylar)
target_link_libraries(test_uri ${LIBS})

#add_executable(test_routine tests/test_routine.cpp)
#force_redefine_file_macro_for_sources(test_routine)
#add_dependencies(test_routine sylar)
#target_link_libraries(test_routine ${LIBS})




SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)