cmake_minimum_required(VERSION 3.16)
project(LowLatencySystem)

set(CMAKE_CXX_STANDARD 20)
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_executable(LowLatencySystem
        src/main.cpp
        src/thread_utils.h
        src/macros.h
        src/mem_pool.h
        src/lock_free_queue.h
        src/time_utils.h
        src/logging.h
        src/socket_utils.h
        src/tcp_socket.h
)

target_link_libraries(LowLatencySystem PRIVATE Threads::Threads)
