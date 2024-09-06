cmake_minimum_required(VERSION 3.27.2)

option(ZEUS_BUILD_FOUNDATION "" ON)

project(zeus
    DESCRIPTION "Zeus Framework"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(/utf-8) # UTF-8 everywhere!
if(MSVC_VERSION GREATER_EQUAL 1914)
    add_compile_options(/Zc:__cplusplus)
endif()

IF(WIN32)
    add_compile_options(/wd"4275")
    add_compile_options(/wd"4252")
    add_compile_definitions(UNICODE _UNICODE) # Unicode
    add_compile_definitions(NOMINMAX) # Unicode
ENDIF()

if(MSVC)
    add_compile_definitions(_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif()

if(ZEUS_BUILD_FOUNDATION)
    add_subdirectory(foundation)
endif()
