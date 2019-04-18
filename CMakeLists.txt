cmake_minimum_required (VERSION 3.1)
project (elementnummern)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(parser)
generate_zusi_parser(zusi_parser ${CMAKE_CURRENT_BINARY_DIR}/zusi_parser)

add_executable(elementnummern elementnummern.cpp nowide/src/iostream.cpp)

target_link_libraries(elementnummern PRIVATE zusi_parser)

find_package(Boost COMPONENTS filesystem graph program_options REQUIRED)
target_include_directories(elementnummern PRIVATE ${Boost_INCLUDE_DIRS} nowide/include)
target_link_libraries(elementnummern PRIVATE ${Boost_LIBRARIES})

install(TARGETS elementnummern RUNTIME DESTINATION bin)
