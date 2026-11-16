cmake_minimum_required(VERSION 3.15)
project(DCCSuperH C)

include(GenerateHH3)
find_package(Gint 2.9 REQUIRED)

add_library(dcc STATIC src/dcc.c)
target_include_directories(dcc PUBLIC include)
target_compile_options(dcc PRIVATE -Wall -Wextra -Os)
set_target_properties(dcc PROPERTIES C_STANDARD 99 C_STANDARD_REQUIRED YES)

add_executable(dcc_app src/main.c tests/selftest.c)
target_include_directories(dcc_app PRIVATE tests)
target_compile_options(dcc_app PRIVATE -Wall -Wextra -Os)
target_link_libraries(dcc_app PRIVATE dcc Gint::Gint)
generate_hh3(TARGET dcc_app OUTPUT "DCC-SuperH.hh3")
