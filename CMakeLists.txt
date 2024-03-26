cmake_minimum_required(VERSION 3.16.3)
project(blob-stream C)
enable_testing()

add_subdirectory(deps/piot/bit-array/src/lib)
add_subdirectory(deps/piot/clog/src/lib)
add_subdirectory(deps/piot/flood-c/src/lib)
add_subdirectory(deps/piot/imprint/src/lib)
add_subdirectory(deps/piot/monotonic-time-c/src/lib)
add_subdirectory(deps/piot/tiny-libc/src/lib)


add_subdirectory(src)
