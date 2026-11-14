cmake_minimum_required(VERSION 3.20)
project(archaeophd_engine VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags for performance & small binary size
if(MSVC)
    add_compile_options(/O2 /W4)
else()
    add_compile_options(-O3 -Wall -Wextra -pthread)
endif()

# Find dependencies
find_package(PkgConfig QUIET)
find_package(zstd REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
    src/main.cpp
)

add_executable(archaeophd_engine ${SOURCES})

target_link_libraries(archaeophd_engine PRIVATE
    zstd::libzstd_static
    pthread
)

# Strip binary in release to minimize size (<10MB)
if(NOT MSVC)
    add_custom_command(TARGET archaeophd_engine POST_BUILD
        COMMAND ${CMAKE_STRIP} archaeophd_engine
    )
endif()
