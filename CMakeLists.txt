cmake_minimum_required (VERSION 3.1)
project (elementnummern)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(parser)
generate_zusi_parser(zusi_parser ${CMAKE_CURRENT_BINARY_DIR}/zusi_parser
  WHITELIST
    Zusi::Strecke::StrElement
    StrElement::InfoNormRichtung
    StrElement::InfoGegenRichtung
    StreckenelementRichtungsInfo::Reg
    Zusi::Fahrplan::StrModul::Datei
    Dateiverknuepfung::Dateiname
  IGNORE_UNKNOWN)

add_executable(elementnummern elementnummern.cpp nowide/src/iostream.cpp)

target_link_libraries(elementnummern PRIVATE zusi_parser)

find_package(Boost COMPONENTS program_options REQUIRED)
target_include_directories(elementnummern PRIVATE nowide/include)
target_link_libraries(elementnummern PRIVATE Boost::program_options)

install(TARGETS elementnummern RUNTIME DESTINATION bin)
