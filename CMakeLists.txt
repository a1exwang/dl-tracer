cmake_minimum_required(VERSION 3.0)
project(dl-tracer C CXX ASM)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Module)
include_directories(libunwind/include)
link_directories(libunwind/src/.libs/)

#find_package(libdwarf)
#include_directories(${libdwarf_INCLUDE_DIRS})
#link_libraries(${libdwarf_LIBRARIES})
add_compile_options(-fPIC)

#add_subdirectory(3rdparty/backward-cpp)

set_source_files_properties(tracer.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
add_library(dl_tracer_plugin SHARED tracer.S target.cpp stack_dump.cpp)
target_compile_options(dl_tracer_plugin PUBLIC -fvisibility=hidden -fvisibility-inlines-hidden)
target_include_directories(dl_tracer_plugin PUBLIC 3rdparty/backward-cpp)
target_link_libraries(dl_tracer_plugin PUBLIC dl dw bfd unwind unwind-x86_64 backtrace)

add_executable(dl_tracer_test dl_tracer_test.cpp)
target_link_libraries(dl_tracer_test PUBLIC dl_tracer_plugin dl)
