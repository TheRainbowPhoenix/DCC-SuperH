cmake_minimum_required(VERSION 3.15)
project(DCCNative C)

add_library(dcc STATIC ../src/dcc.c)
target_include_directories(dcc PUBLIC ../include)
set_target_properties(dcc PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)

add_executable(dcc_cli main.c)
target_link_libraries(dcc_cli PRIVATE dcc)
set_target_properties(dcc_cli PROPERTIES OUTPUT_NAME dcc)
