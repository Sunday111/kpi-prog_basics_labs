cmake_minimum_required(VERSION 3.5.1)
include(generate_vs_filters)
include(glob_cxx_sources)
include(cxx_version)

set(target_name "graph_lib")
glob_cxx_sources(${CMAKE_CURRENT_SOURCE_DIR} target_sources)
add_library(${target_name} INTERFACE)
target_include_directories(${target_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
generate_vs_filters(${target_sources})

add_custom_target("${target_name}_" SOURCES ${target_sources})
set_target_properties("${target_name}_" PROPERTIES FOLDER ${local_filter})
