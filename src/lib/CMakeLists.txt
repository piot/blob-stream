cmake_minimum_required(VERSION 3.17)
project(blob_stream_lib C)

set(CMAKE_C_STANDARD 99)

set(deps ../../deps/)

if (WIN32) 
    add_compile_definitions(TORNADO_OS_WINDOWS 1)
endif(WIN32)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(isDebug TRUE)
else()
    set(isDebug FALSE)
endif()

if (isDebug)
    message("Debug build detected")
    add_compile_definitions(CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)
    set(CONFIGURATION_DEBUG 1)
endif()

file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "*.c"
        )

add_library(blob_stream_lib STATIC
    ${deps_src}
    ${lib_src})

target_include_directories(blob_stream_lib PUBLIC ${deps}piot/tiny-libc/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/tyran/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/clog/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/imprint/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/bit-array/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/monotonic-time-c/src/include)
target_include_directories(blob_stream_lib PUBLIC ${deps}piot/flood-c/src/include)

target_include_directories(blob_stream_lib PUBLIC ../include)


if (WIN32)
target_link_libraries(blob_stream_lib)
else()
target_link_libraries(blob_stream_lib m)
endif()
