cmake_minimum_required(VERSION 3.10)
project(control LANGUAGES CXX RC)

set(SOURCES
    src/main.cpp
    src/gui.cpp
    src/blocker.cpp
    src/config.cpp
    src/password.cpp
    resources/resource.rc
)

add_executable(control WIN32 ${SOURCES})

target_include_directories(control PRIVATE src resources)

target_compile_definitions(control PRIVATE UNICODE _UNICODE)

if(MSVC)
    target_compile_options(control PRIVATE /EHsc)
    target_link_libraries(control PRIVATE user32 shell32 comctl32 ole32 shlwapi gdi32)
else()
    target_link_libraries(control PRIVATE user32 shell32 comctl32 ole32 shlwapi gdi32)
endif()
