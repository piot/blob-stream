# generated by cmake-generator
cmake_minimum_required(VERSION 3.16.3)

add_library(blob-stream STATIC 
  blob_stream_in.c
  blob_stream_logic_in.c
  blob_stream_logic_out.c
        debug.c
  blob_stream_out.c)

include(Tornado.cmake)
set_tornado(blob-stream)

target_include_directories(blob-stream PUBLIC ../include)


target_link_libraries(blob-stream PUBLIC 
  bit-array
  monotonic-time
  flood)

