cmake_minimum_required(VERSION 3.20)
project(ArchaeoPhDDesktop VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_compile_definitions(UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    
    # Win32 subsystem to run as GUI app without console window popping up
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/packages/webview2/build/native/include
)

set(SOURCES
    src/main.cpp
)

add_executable(ArchaeoPhD WIN32 ${SOURCES})

target_link_libraries(ArchaeoPhD PRIVATE
    user32
    shell32
    ole32
    oleaut32
    uuid
    shlwapi
    version
)

# Strip symbols in release mode for ultra-small ~3 MB binary
if(MINGW OR CMAKE_COMPILER_IS_GNUCXX)
    target_link_options(ArchaeoPhD PRIVATE -s -mwindows)
endif()
