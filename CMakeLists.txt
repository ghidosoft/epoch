cmake_minimum_required(VERSION 3.22)
project(epoch)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CheckIPOSupported)
check_ipo_supported(RESULT is_ipo_supported OUTPUT ipo_output)
if(is_ipo_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG OFF)
else()
    message(WARNING "IPO is not supported: ${ipo_output}")
endif()

add_subdirectory(external)
add_subdirectory(src)

enable_testing()

add_subdirectory(test)
