cmake_minimum_required(VERSION 3.10)
project(QuoteTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# macOS 不需要 -lrt，但 Linux 需要
if(APPLE)
    set(EXTRA_LIBS "")
else()
    set(EXTRA_LIBS "rt" "pthread")
endif()

# Orderbook library
add_library(orderbook src/orderbook.cpp)

# Simulator
add_executable(simulator src/simulator.cpp)
target_link_libraries(simulator ${EXTRA_LIBS})

# Recorder
add_executable(recorder src/recorder.cpp)
target_link_libraries(recorder ${EXTRA_LIBS})

# Strategy
add_executable(strategy src/strategy.cpp)
target_link_libraries(strategy orderbook ${EXTRA_LIBS})
